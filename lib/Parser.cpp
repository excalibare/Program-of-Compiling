#include "Parser.h"
#include "Error.h"

Parser::Parser(const std::vector<std::pair<std::string, std::string>>& tokens)
    : tokens(tokens), current(0) {}

void Parser::analyze() {
    try {
        expression();
        if (!isAtEnd()) {
            // Error::printError(Error::ErrorType::EXTRA_TOKEN);
            throw std::runtime_error("存在多余符号，表达式后仍有内容: '" + peek().second + "'");
        }
        std::cout << "语法正确!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "语法错误: " << e.what() << std::endl;
    }
}

void Parser::expression() {
    auto token = peek();
    // [+|-]
    if (isAddOp(token.first)) {
        log("匹配前导加法运算符: " + token.second);
        advance();
    }
    // <项>
    term();
    // {<加法运算符> <项>}
    while (!isAtEnd() && isAddOp(peek().first)) {
        log("匹配加法运算符: " + peek().second);
        advance();
        term();
    }
}

void Parser::term() {
    // <因子>
    factor();
    // {<乘法运算符> <因子>}
    while (!isAtEnd() && isMulOp(peek().first)) {
        log("匹配乘法运算符: " + peek().second);
        advance();
        factor();
    }
}

void Parser::factor() {
    // <标识符>|<无符号整数>| ‘(’<表达式>‘)’
    auto token = peek();
    if (token.first == "ident" || token.first == "number") {
        log("匹配因子: " + token.second);
        advance();
    } else if (token.first == "lparen") {
        log("匹配左括号: " + token.second);
        advance();
        expression();   // 进入表达式递归
        if (peek().first == "rparen") {
            log("匹配右括号: " + peek().second);
            advance();
        } else {
            // Error::printError(Error::ErrorType::MISSING_RPAREN,"当前位置: " + peek(-1).second);
            throw std::runtime_error("缺少右括号，当前位置: " + peek(-1).second);
        }
    } else {
        // Error::printError(Error::ErrorType::INVALID_FACTOR,"应为标识符、数字或括号表达式，但实际为: '" + token.second + "'");
        throw std::runtime_error("因子格式错误，应为标识符、数字或括号表达式，但实际为: '" + token.second + "'");
    }
}

void Parser::match(const std::string& expectedType) {
    if (peek().first == expectedType) {
        advance();
    } else {
        auto actual = peek();
        throw std::runtime_error(
            "匹配失败，期望类型为: '" + expectedType +
            "'，实际类型为: '" + actual.first +
            "'，值为: '" + actual.second + "'"
        );
    }
}

// 判断加法运算符
bool Parser::isAddOp(const std::string& type) {
    return type == "plus" || type == "minus";
}

// 判断乘法运算符
bool Parser::isMulOp(const std::string& type) {
    return type == "times" || type == "slash";
}

std::pair<std::string, std::string> Parser::peek(int offset) {
    if (current + offset >= tokens.size()) return {"", ""};
    return tokens[current + offset];
}

void Parser::advance() {
    if (!isAtEnd()) current++;
}

bool Parser::isAtEnd() const {
    return current >= tokens.size();
}

bool Parser::debug = false;

void Parser::debug_on() {
    debug = true;
}

void Parser::debug_off() {
    debug = false;
}

void Parser::log(const std::string& msg) {
    if (debug) {
        std::cerr << "[Debug] " << msg << std::endl;
    }
}
