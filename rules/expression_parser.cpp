#include "expression_parser.h"
#include "../logger/logger.h"

ExpressionParser::ExpressionParser(const string& expression) : expression(expression), current_token(0) {
    tokenize();
}

void ExpressionParser::tokenize() {
    tokens.clear();
    current_token = 0;
    
    size_t i = 0;
    while (i < expression.length()) {
        // Skip whitespace
        while (i < expression.length() && isspace(expression[i])) {
            i++;
        }
        
        if (i >= expression.length()) break;
        
        char c = expression[i];
        
        if (c == '(') {
            tokens.push_back({TOKEN_LPAREN, "("});
            i++;
        } else if (c == ')') {
            tokens.push_back({TOKEN_RPAREN, ")"});
            i++;
        } else if (c == 'A' || c == 'a') {
            // Check for AND
            if (i + 2 < expression.length() && 
                (expression[i+1] == 'N' || expression[i+1] == 'n') &&
                (expression[i+2] == 'D' || expression[i+2] == 'd') &&
                (i + 3 >= expression.length() || isspace(expression[i+3]) || expression[i+3] == '(' || expression[i+3] == ')')) {
                tokens.push_back({TOKEN_AND, "AND"});
                i += 3;
            } else {
                // Part of a literal
                string literal = "";
                while (i < expression.length() && !isspace(expression[i]) && 
                       expression[i] != '(' && expression[i] != ')' &&
                       !(i + 2 < expression.length() && 
                         (expression[i] == 'A' || expression[i] == 'a') &&
                         (expression[i+1] == 'N' || expression[i+1] == 'n') &&
                         (expression[i+2] == 'D' || expression[i+2] == 'd')) &&
                       !(i + 1 < expression.length() && 
                         (expression[i] == 'O' || expression[i] == 'o') &&
                         (expression[i+1] == 'R' || expression[i+1] == 'r'))) {
                    literal += expression[i];
                    i++;
                }
                if (!literal.empty()) {
                    tokens.push_back({TOKEN_LITERAL, literal});
                }
            }
        } else if (c == 'O' || c == 'o') {
            // Check for OR
            if (i + 1 < expression.length() && 
                (expression[i+1] == 'R' || expression[i+1] == 'r') &&
                (i + 2 >= expression.length() || isspace(expression[i+2]) || expression[i+2] == '(' || expression[i+2] == ')')) {
                tokens.push_back({TOKEN_OR, "OR"});
                i += 2;
            } else {
                // Part of a literal
                string literal = "";
                while (i < expression.length() && !isspace(expression[i]) && 
                       expression[i] != '(' && expression[i] != ')' &&
                       !(i + 2 < expression.length() && 
                         (expression[i] == 'A' || expression[i] == 'a') &&
                         (expression[i+1] == 'N' || expression[i+1] == 'n') &&
                         (expression[i+2] == 'D' || expression[i+2] == 'd')) &&
                       !(i + 1 < expression.length() && 
                         (expression[i] == 'O' || expression[i] == 'o') &&
                         (expression[i+1] == 'R' || expression[i+1] == 'r'))) {
                    literal += expression[i];
                    i++;
                }
                if (!literal.empty()) {
                    tokens.push_back({TOKEN_LITERAL, literal});
                }
            }
        } else if (c == 'N' || c == 'n') {
            // Check for NOT
            if (i + 2 < expression.length() && 
                (expression[i+1] == 'O' || expression[i+1] == 'o') &&
                (expression[i+2] == 'T' || expression[i+2] == 't') &&
                (i + 3 >= expression.length() || isspace(expression[i+3]) || expression[i+3] == '(' || expression[i+3] == ')')) {
                tokens.push_back({TOKEN_NOT, "NOT"});
                i += 3;
            } else {
                // Part of a literal
                string literal = "";
                while (i < expression.length() && !isspace(expression[i]) && 
                       expression[i] != '(' && expression[i] != ')' &&
                       !(i + 2 < expression.length() && 
                         (expression[i] == 'A' || expression[i] == 'a') &&
                         (expression[i+1] == 'N' || expression[i+1] == 'n') &&
                         (expression[i+2] == 'D' || expression[i+2] == 'd')) &&
                       !(i + 1 < expression.length() && 
                         (expression[i] == 'O' || expression[i] == 'o') &&
                         (expression[i+1] == 'R' || expression[i+1] == 'r')) &&
                       !(i + 2 < expression.length() && 
                         (expression[i] == 'N' || expression[i] == 'n') &&
                         (expression[i+1] == 'O' || expression[i+1] == 'o') &&
                         (expression[i+2] == 'T' || expression[i+2] == 't'))) {
                    literal += expression[i];
                    i++;
                }
                if (!literal.empty()) {
                    tokens.push_back({TOKEN_LITERAL, literal});
                }
            }
        } else {
            // Regular literal
            string literal = "";
            while (i < expression.length() && !isspace(expression[i]) && 
                   expression[i] != '(' && expression[i] != ')' &&
                   !(i + 2 < expression.length() && 
                     (expression[i] == 'A' || expression[i] == 'a') &&
                     (expression[i+1] == 'N' || expression[i+1] == 'n') &&
                     (expression[i+2] == 'D' || expression[i+2] == 'd')) &&
                   !(i + 1 < expression.length() && 
                     (expression[i] == 'O' || expression[i] == 'o') &&
                     (expression[i+1] == 'R' || expression[i+1] == 'r'))) {
                literal += expression[i];
                i++;
            }
            if (!literal.empty()) {
                tokens.push_back({TOKEN_LITERAL, literal});
            }
        }
    }
    
    tokens.push_back({TOKEN_EOF, ""});
}

bool ExpressionParser::evaluate(const string& test_value, const string& field_type) {
    current_token = 0;
    this->test_value = test_value;
    this->field_type = field_type;
    return parse_expression();
}

bool ExpressionParser::parse_expression() {
    return parse_or_expression();
}

bool ExpressionParser::parse_or_expression() {
    bool left = parse_and_expression();
    
    while (get_current_token().type == TOKEN_OR) {
        advance_token();
        bool right = parse_and_expression();
        left = left || right;
    }
    
    return left;
}

bool ExpressionParser::parse_and_expression() {
    bool left = parse_primary();
    
    while (get_current_token().type == TOKEN_AND) {
        advance_token();
        bool right = parse_primary();
        left = left && right;
    }
    
    return left;
}

bool ExpressionParser::parse_primary() {
    Token token = get_current_token();
    
    if (token.type == TOKEN_NOT) {
        advance_token();
        bool result = parse_primary();
        return !result;
    } else if (token.type == TOKEN_LITERAL) {
        advance_token();
        return match_literal(token.value, "literal");
    } else if (token.type == TOKEN_LPAREN) {
        advance_token();
        bool result = parse_expression();
        if (get_current_token().type != TOKEN_RPAREN) {
            // Error: unmatched parenthesis
            return false;
        }
        advance_token();
        return result;
    }
    
    return false;
}

bool ExpressionParser::match_literal(const string& rule_value, const string& field_type) {
    // Use the stored test value and field type
    if (this->field_type == "ip") {
        return match_ip(rule_value, this->test_value);
    } else if (this->field_type == "port") {
        return match_port(rule_value, this->test_value);
    } else if (this->field_type == "string") {
        return match_string(rule_value, this->test_value);
    }
    
    // Default literal matching
    return rule_value == "any" || rule_value == "ANY";
}

bool ExpressionParser::match_ip(const string& rule_value, const string& test_value) {
    return rule_value == "any" || rule_value == "ANY" || rule_value == test_value;
}

bool ExpressionParser::match_port(const string& rule_value, const string& test_value) {
    if (rule_value == "any" || rule_value == "ANY") return true;
    
    try {
        int rule_port = stoi(rule_value);
        int test_port = stoi(test_value);
        return rule_port == test_port;
    } catch (...) {
        return false;
    }
}

bool ExpressionParser::match_string(const string& rule_value, const string& test_value) {
    if (rule_value.empty()) return true;
    
    // Case-insensitive substring matching
    string lower_rule = rule_value;
    string lower_test = test_value;
    
    for (char &c : lower_rule) c = tolower(static_cast<unsigned char>(c));
    for (char &c : lower_test) c = tolower(static_cast<unsigned char>(c));
    
    return lower_test.find(lower_rule) != string::npos;
}

Token ExpressionParser::get_current_token() {
    if (current_token >= tokens.size()) {
        return {TOKEN_EOF, ""};
    }
    return tokens[current_token];
}

void ExpressionParser::advance_token() {
    if (current_token < tokens.size()) {
        current_token++;
    }
}

bool ExpressionParser::is_at_end() {
    return current_token >= tokens.size() || get_current_token().type == TOKEN_EOF;
}

void ExpressionParser::print_tokens() {
    for (const auto& token : tokens) {
        cout << "Token: " << token.value << " (Type: " << token.type << ")" << endl;
    }
}
