#pragma once
#include "../includes.h"
#include "../logger/logger.h"

class ConfigParser {
private:
    map<string, string> config;
    string filename;

    void flattenYaml(const YAML::Node& node, const std::string& prefix = "");
    
public:
    ConfigParser(const string& configFile);
    bool loadConfig(Logger *logger);
    string get(const string& key) const;
    string get(const string& key, const string& defaultValue) const;
    void printAll() const;
    bool hasKey(const string& key) const;
};