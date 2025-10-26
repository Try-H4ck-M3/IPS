#include "parser.h"

static void check_unknown_fields(const json& item, Logger& logger)
{
    vector<string> known_fields = {"rule_id", "description", "src_ip", "dst_ip", "src_port", "dst_port", "protocol", "action", "string"};
    
    for (const auto& [key, value] : item.items())
    {
        if (find(known_fields.begin(), known_fields.end(), key) == known_fields.end())
        {
            logger.warn("Unknown field \"" + key + "\" in rule " + item.value("description", "unknown") + " (ID: " + item.value("rule_id", "unknown") + ")");
        }
    }
}

static int validate_rule(const json& item, Logger& logger, set<int>& seen_rule_ids)
{
    if (!item.contains("rule_id") || item.at("rule_id").is_null())
    {
        logger.error("Rule ID is required");
        cleanup_and_exit(1);
        return 1;
    }
    int rule_id = item.at("rule_id");
    if (seen_rule_ids.find(rule_id) != seen_rule_ids.end())
    {
        logger.error("Duplicate rule ID: " + to_string(rule_id));
        cleanup_and_exit(1);
        return 1;
    }
    seen_rule_ids.insert(rule_id);



    if (!item.contains("action") || item.at("action").is_null())
    {
        logger.error("Action is required");
        return 1;
    }
    if (item.at("action").is_string() && item.at("action").get<string>() != "drop" && item.at("action").get<string>() != "alert" && item.at("action").get<string>() != "accept")
    {
        logger.error("Action must be \"drop\", \"alert\" or \"accept\"");
        return 1;
    }



    if ((!item.contains("src_ip") || item.at("src_ip").is_null()) && (!item.contains("dst_ip") || item.at("dst_ip").is_null()))
    {
        logger.error("Either src_ip or dst_ip is required");
        return 1;
    }
    // Port validation is now optional since we support expressions

    return 0;
}

vector<Rule> parse_all_rules(string rules_file_path, Logger logger)
{
    logger.info("Proceessing rules from \"" + rules_file_path + "\" next:");
    // Open file
    ifstream file(rules_file_path);
    if (!file.is_open())
    {
        logger.error("Cannot open file \"" + rules_file_path + "\". Creating empty one.");
        return {};
    }

    // Parse JSON
    json j;
    try
    {
        file >> j;
    }
    catch (const exception& e)
    {
        logger.error("Invalid rules JSON - " + string(e.what()));
        return {};
    }

    // Expect array of rules
    if (!j.is_array())
    {
        logger.error("Rules JSON should be an array");
        return {};
    }


    // Parse each rule
    vector<Rule> rules;
    set<int> seen_rule_ids; // Local set for this parsing session
    for (const auto& item : j)
    {
        try
        {
            logger.verbose_log("Processing rule \"" + item.value("description", "unknown") + "\" (ID: " + to_string(item.value("rule_id", -1)) + ")");
            if (validate_rule(item, logger, seen_rule_ids) != 0)
            {
                logger.error("Invalid rule. Cannot continue");
                return {};
            }
            
            // Check for unknown fields
            check_unknown_fields(item, logger);
            
            Rule rule;
            rule.rule_id      = item.value("rule_id", 0);
            rule.description  = item.value("description", "unknown");
            rule.src_ip       = item.value("src_ip", string("any"));
            rule.dst_ip       = item.value("dst_ip", string("any"));
            rule.src_port     = item.value("src_port", string("any"));
            rule.dst_port     = item.value("dst_port", string("any"));
            rule.protocol     = item.value("protocol", string("any"));
            rule.action       = item.value("action", string("accept"));
            rule.string_content = item.value("string", string(""));

            rule.print(logger);
            rules.push_back(rule);
        }
        catch (const exception& e)
        {
            logger.error("This should never happen. Please report, if you see this message. Malformed rule - " + string(e.what()));
            logger.error("Invalid rule. Cannot continue");
            return {};
        }
    }
    logger.info("All rules OK. Loaded " + to_string(rules.size()) + " rules");

    return rules;
}