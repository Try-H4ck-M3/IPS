#pragma once
#include "../includes.h"

enum TokenType {
    TOKEN_LITERAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_EOF
};

struct Token {
    TokenType type;
    string value;
};

class ExpressionParser {
public:
    ExpressionParser(const string& expression);
    
    // Parse and evaluate the expression
    bool evaluate(const string& test_value, const string& field_type = "literal");
    
    // For debugging
    void print_tokens();
    
private:
    string expression;
    vector<Token> tokens;
    size_t current_token;
    string test_value;
    string field_type;
    
    void tokenize();
    bool parse_expression();
    bool parse_and_expression();
    bool parse_or_expression();
    bool parse_primary();
    
    bool match_literal(const string& test_value, const string& field_type);
    bool match_ip(const string& rule_value, const string& test_value);
    bool match_port(const string& rule_value, const string& test_value);
    bool match_string(const string& rule_value, const string& test_value);
    
    Token get_current_token();
    void advance_token();
    bool is_at_end();
};
