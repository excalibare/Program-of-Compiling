#include "Parser.h"
#include "Error.h"

Parser::Parser(const std::vector<std::pair<std::string, std::string> > &tokens)
    : tokens(tokens), current(0) {
}

void Parser::analyze() {
    try {
        statement();
        if (!isAtEnd()) {
            throw std::runtime_error("存在多余符号，语句后仍有内容: '" + peek().second + "'");
        }
        std::cerr << "语法正确!" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "语法错误: " << e.what() << std::endl;
    }
}


void Parser::statement() {
    auto token = peek();

    if (token.first == "ifsym") {
        log("匹配 if 语句");
        advance(); // consume 'if'
        condition(); // 解析条件
        match("thensym"); // consume 'then'
        statement(); // then 后接一个语句
        if (peek().first == "elsesym") {
            log("匹配 else 分支");
            advance(); // consume 'else'
            statement(); // else 后接一个语句
        }
    } else if (token.first == "whilesym") {
        log("匹配 while 语句");
        advance(); // consume 'while'
        condition();
        match("dosym"); // consume 'do'
        statement();
        advance();
    } else if (token.first == "beginsym") {
        log("匹配 begin ... end 语句块");
        advance(); // 消费 'begin'
        statement(); // 解析第一条语句

        // 修改后的分号处理逻辑
        while (peek().first == "semicolon") {
            advance(); // 消费分号
            // 检查分号后是否直接跟着 'end'
            if (peek().first == "endsym") {
                break; // 允许最后的分号，直接退出循环
            }
            statement(); // 解析后续语句
        }

        if (peek().first == "endsym") {
            advance(); // 消费 'end'
        } else {
            throw std::runtime_error("缺少 'end'，当前位置: " + peek().second);
        }
    } else if (token.first == "ident" && peek(1).first == "becomes") { // 处理赋值语句
        log("匹配赋值语句");
        advance(); // consume 标识符
        advance(); // consume :=
        int value = expression(); // 解析右侧表达式并计算值
        std::cerr << "赋值语句计算结果: " << value << std::endl;
    } else {
        int value = expression(); // 计算表达式的值
        std::cerr << "表达式计算结果: " << value << std::endl;
    }
}

void Parser::condition() {
    if (peek().first == "oddsym") {
        log("匹配 odd 条件");
        advance(); // consume 'odd'
        expression(); // odd <表达式>
    } else {
        expression(); // 左表达式
        auto relop = peek();
        if (isRelOp(relop.first)) {
            log("匹配关系运算符: " + relop.second);
            advance(); // consume relop
            expression(); // 右表达式
        } else {
            throw std::runtime_error("缺少关系运算符，当前位置: '" + relop.second + "'");
        }
    }
}


int Parser::expression() {
    int value = 0; // 用于存储表达式的值
    auto token = peek();
    if (isAddOp(token.first)) {
        advance();
    }
    value = term(); // 计算第一个项的值
    while (!isAtEnd() && isAddOp(peek().first)) {
        auto op = peek();
        advance();
        int nextTerm = term(); // 计算下一个项的值
        if (op.first == "plus") {
            value += nextTerm;
        } else if (op.first == "minus") {
            value -= nextTerm;
        }
    }
    return value; // 返回最终的表达式值
}

int Parser::term() {
    int value = factor(); // 计算第一个因子的值
    while (!isAtEnd() && isMulOp(peek().first)) {
        auto op = peek();
        advance();
        int nextFactor = factor(); // 计算下一个因子的值
        if (op.first == "times") {
            value *= nextFactor;
        } else if (op.first == "slash") {
            if (nextFactor == 0) {
                throw std::runtime_error("除以零错误");
            }
            value /= nextFactor;
        }
    }
    return value;
}

int Parser::factor() {
    auto token = peek();
    if (token.first == "number") {
        advance();
        return std::stoi(token.second); // 返回数字的值
    } else if (token.first == "lparen") {
        advance();
        int value = expression(); // 递归计算括号内的表达式值
        if (peek().first == "rparen") {
            advance();
        } else {
            throw std::runtime_error("缺少右括号");
        }
        return value;
    } else {
        throw std::runtime_error("因子格式错误");
    }
}

void Parser::match(const std::string &expectedType) {
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
bool Parser::isAddOp(const std::string &type) {
    return type == "plus" || type == "minus";
}

// 判断乘法运算符
bool Parser::isMulOp(const std::string &type) {
    return type == "times" || type == "slash";
}

bool Parser::isRelOp(const std::string &type) {
    return type == "eql" || type == "neq" || type == "lss" ||
           type == "leq" || type == "gtr" || type == "geq";
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

void Parser::log(const std::string &msg) {
    if (debug) {
        std::cerr << "[Debug] " << msg << std::endl;
    }
}
