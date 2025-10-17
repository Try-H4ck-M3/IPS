#include "packet_processor.h"


static Logger *g_logger = nullptr;

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
        rules = parse_all_rules("./rules.json", *g_logger);
        initialized = true;
    }
    return rules;
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
    return (rule_ip == "any" || rule_ip == "ANY" || rule_ip == value_ip);
}

static bool port_matches(int rule_port, uint16_t value_port)
{
    return (rule_port == -1 || static_cast<uint16_t>(rule_port) == value_port);
}

static bool rule_matches(const Rule &rule,
                         const string &src_ip,
                         const string &dst_ip,
                         uint16_t src_port,
                         uint16_t dst_port,
                         uint8_t protocol)
{
    if (!ip_matches(rule.src_ip, src_ip)) return false;
    if (!ip_matches(rule.dst_ip, dst_ip)) return false;
    if (!port_matches(rule.src_port, src_port)) return false;
    if (!port_matches(rule.dst_port, dst_port)) return false;

    string rule_proto = to_upper_copy(rule.protocol);
    if (!(rule_proto == "ANY" || rule_proto == to_upper_copy(protocol_to_string(protocol))))
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

    string last_action = ""; // track last matching action
    Rule last_rule;          // track last matching rule

    for (const Rule &rule : rules)
    {
        if (rule_matches(rule, src_ip, dst_ip, src_port, dst_port, protocol))
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