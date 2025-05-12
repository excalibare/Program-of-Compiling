#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <stack>
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For std::cerr (used in log)

// Forward declaration of Lexer if needed, or include Lexer.h if it defines pair
// For now, assuming std::pair<std::string, std::string> is well-understood.

class Parser {
public:
    // Structure for Quadruples
    struct Quad {
        std::string op;
        std::string arg1;
        std::string arg2;
        std::string result;
    };

    Parser(const std::vector<std::pair<std::string, std::string> > &tokens);
    void analyze();
    void printQuads() const;

    // Debugging controls
    static void debug_on();
    static void debug_off();

private:
    std::vector<std::pair<std::string, std::string> > tokens_;
    size_t currentTokenIndex_;
    std::vector<Quad> quads_;

    // For S-attributed semantic processing
    std::stack<std::string> semanticStack_;
    int tempCounter_;

    // Parsing methods
    void program(); // Assuming a program can be a single statement or block
    void block(); // Placeholder for block structure if you expand PL/0 grammar
    void statement();
    void condition();
    void expression();
    void term();
    void factor();

    // Helper methods
    std::string newTemp();
    void match(const std::string &expectedType);
    bool isAddOp(const std::string &type) const;
    bool isMulOp(const std::string &type) const;
    bool isRelOp(const std::string &type) const;
    std::pair<std::string, std::string> peek(int offset = 0) const;
    void advance();
    bool isAtEnd() const;

    static bool debug_mode_; // Static member for debug state
    void log(const std::string &msg) const;
};

#endif // PARSER_H