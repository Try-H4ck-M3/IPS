#include "packet_processor.h"
#include "../rules/expression_parser.h"
#include "../rate_limit/rate_limiter.h"

// External reference to global rate limiter (defined in IPS.cpp)
extern RateLimiter* g_rate_limiter;

static Logger *g_logger = nullptr;
string g_rules_path = "./configs/rules.json"; // Default path, can be overridden from config

void set_packet_processor_logger(Logger *logger)
{
    g_logger = logger;
}

// Load rules once on first use
static vector<Rule> &get_rules()
{
    static bool initialized = false;
    static vector<Rule> rules;
    if (!initialized)
    {
        if (g_logger == nullptr)
        {
            static Logger fallback_logger(false);
            g_logger = &fallback_logger;
        }
        // Parse rules (will print if logger is verbose)
        rules = parse_all_rules(g_rules_path, *g_logger);
        initialized = true;
    }
    return rules;
}

// Force load rules at startup to show them in verbose mode
void load_rules_at_startup()
{
    if (g_logger) {
        g_logger->verbose_log("Starting to load rules from config");
    }
    get_rules(); // This will trigger the one-time rule loading with proper logger
}

static string protocol_to_string(uint8_t protocol)
{
    if (protocol == 6) return "TCP";
    if (protocol == 17) return "UDP";
    return "ANY";
}

static string to_upper_copy(string s)
{
    for (char &c : s) c = toupper(static_cast<unsigned char>(c));
    return s;
}

static bool ip_matches(const string &rule_ip, const string &value_ip)
{
    ExpressionParser parser(rule_ip);
    return parser.evaluate(value_ip, "ip");
}

static bool port_matches(const string &rule_port, uint16_t value_port)
{
    ExpressionParser parser(rule_port);
    return parser.evaluate(to_string(value_port), "port");
}

static bool string_matches(const string &rule_string, unsigned char *data, int len)
{
    if (rule_string.empty())
    {
        return true; // No string to match, so it matches
    }
    
    if (len <= 0 || data == nullptr)
    {
        return false; // No data to search
    }
    
    // Convert packet data to string for expression evaluation
    string packet_data_str(data, data + len);
    
    ExpressionParser parser(rule_string);
    return parser.evaluate(packet_data_str, "string");
}

static bool rule_matches(const Rule &rule,
                         const string &src_ip,
                         const string &dst_ip,
                         uint16_t src_port,
                         uint16_t dst_port,
                         uint8_t protocol,
                         unsigned char *data,
                         int len)
{
    if (!ip_matches(rule.src_ip, src_ip)) return false;
    if (!ip_matches(rule.dst_ip, dst_ip)) return false;
    if (!port_matches(rule.src_port, src_port)) return false;
    if (!port_matches(rule.dst_port, dst_port)) return false;

    string rule_proto = to_upper_copy(rule.protocol);
    if (!(rule_proto == "ANY" || rule_proto == to_upper_copy(protocol_to_string(protocol))))
        return false;

    // Check string content if specified
    if (!string_matches(rule.string_content, data, len))
        return false;

    return true;
}

static void handle_alert(unsigned char *data, int len, string src_ip, string dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t protocol, Rule last_rule)
{
    g_logger->alert(src_ip + ":" + to_string(src_port) + " -> " + dst_ip + ":" + to_string(dst_port) + " (" + protocol_to_string(protocol) + ")");
    cout << "| Matches rule: \"" << last_rule.description << "\" (ID: " << last_rule.rule_id << ")" << endl;
    if (g_logger->is_verbose())
    {
        last_rule.print(*g_logger);
    }
}

static bool decide_verdict_from_action(const string &action, unsigned char *data, int len, string src_ip, string dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t protocol, Rule last_rule)
{
    // Accept by default if unknown
    string act = to_upper_copy(action);
    if (act == "DROP") return false;
    if (act == "ACCEPT") return true;
    return true;
}

bool packet_processor(unsigned char *data, int len, string src_ip, string dst_ip, uint16_t src_port, uint16_t dst_port, uint8_t protocol) // 6 = TCP, 17 = UDP
{
    (void)data;
    (void)len;

    vector<Rule> &rules = get_rules();

    // First, check rate limiting rules
    for (const Rule &rule : rules)
    {
        if (rule.is_rate_limit_rule && g_rate_limiter != nullptr)
        {
            if (rule_matches(rule, src_ip, dst_ip, src_port, dst_port, protocol, data, len))
            {
                // Check if IP should be rate limited
                if (g_rate_limiter->check_rate_limit(src_ip, rule.max_requests, rule.time_window_seconds))
                {
                    // Rate limit exceeded, ban the IP
                    g_rate_limiter->ban_ip(src_ip, rule.ban_duration_seconds);
                    
                    g_logger->alert("Rate limit exceeded by " + src_ip + 
                                    " (exceeded " + to_string(rule.max_requests) + 
                                    " packets per " + to_string(rule.time_window_seconds) + 
                                    " seconds) - Banned for " + to_string(rule.ban_duration_seconds) + " seconds");
                    
                    // Apply the action (usually drop)
                    return decide_verdict_from_action(rule.action, data, len, src_ip, dst_ip, src_port, dst_port, protocol, rule);
                }
            }
        }
    }
    
    // Check if IP is currently banned (check all rate limit rules for consistency)
    if (g_rate_limiter != nullptr)
    {
        for (const Rule &rule : rules)
        {
            if (rule.is_rate_limit_rule)
            {
                if (g_rate_limiter->is_banned(src_ip))
                {
                    g_logger->alert("Banned IP " + src_ip + " attempted connection");
                    return false; // Drop banned IPs
                }
            }
        }
    }

    string last_action = ""; // track last matching action
    Rule last_rule;          // track last matching rule

    for (const Rule &rule : rules)
    {
        if (rule_matches(rule, src_ip, dst_ip, src_port, dst_port, protocol, data, len))
        {
            string act = to_upper_copy(rule.action);
            if (act == "ALERT")
            {
                handle_alert(data, len, src_ip, dst_ip, src_port, dst_port, protocol, rule);
                continue;
            }
            last_action = rule.action;
            last_rule = rule;
        }
    }

    if (last_action.empty())
    {
        // No rules matched → accept for now
        return true;
    }

    return decide_verdict_from_action(last_action, data, len, src_ip, dst_ip, src_port, dst_port, protocol, last_rule);
}