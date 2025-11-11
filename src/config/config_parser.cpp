#include "config_parser.h"

ConfigParser::ConfigParser(const std::string& configFile) : filename(configFile) {}

bool ConfigParser::loadConfig(Logger *logger)
{
    try
    {
        config.clear();
        YAML::Node root = YAML::LoadFile(filename);
        flattenYaml(root);
        return true;
    }
    catch (const YAML::Exception& e)
    {
        logger->error("Error loading config file: " + string(e.what()));
        return false;
    }
}

void ConfigParser::flattenYaml(const YAML::Node& node, const string& prefix)
{
    if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            string key = it->first.as<string>();
            string newPrefix = prefix.empty() ? key : prefix + "." + key;
            flattenYaml(it->second, newPrefix);
        }
    }
    else if (node.IsScalar())
    {
        config[prefix] = node.as<string>();
    }
}

string ConfigParser::get(const string& key) const
{
    auto it = config.find(key);
    if (it != config.end())
    {
        return it->second;
    }
    return ""; // or throw an exception
}

string ConfigParser::get(const string& key, const string& defaultValue) const
{
    auto it = config.find(key);
    if (it != config.end())
    {
        return it->second;
    }
    return defaultValue;
}

bool ConfigParser::hasKey(const string& key) const
{
    return config.find(key) != config.end();
}

void ConfigParser::printAll() const
{
    cout << "====  Configuration  ====" << endl;
    for (const auto& pair : config)
    {
        cout << pair.first << " = " << pair.second << endl;
    }
    cout << "=========================" << endl;
}