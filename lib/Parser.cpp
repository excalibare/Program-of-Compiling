#include "Parser.h"
#include "Error.h"

Parser::Parser(const std::vector<std::pair<std::string, std::string> > &tokens)
    : tokens(tokens), current(0) {
}

void Parser::analyze() {
    try {
        while (!isAtEnd()) {
            statement();
        }
        // statement();
        // if (!isAtEnd()) {
        //     throw std::runtime_error("存在多余符号，语句后仍有内容: '" + peek().second + "'");
        // }
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
    }else if (token.first == "constsym"){
        log("匹配 const 声明");
        advance(); // consume 'const'

        while (true) {
            std::string name = peek().second;
            match("ident");      // consume 标识符
            match("eql");        // consume '='
            auto valueToken = peek();
            match("number");     // consume 数字

            int value = std::stoi(valueToken.second);
            constTable[name] = value; // 添加到符号表
            log("常量定义: " + name + " = " + std::to_string(value));

            if (peek().first == "comma") {
                advance(); // 多个 const，用逗号隔开
            } else if (peek().first == "semicolon") {
                advance(); // const 声明结束
                break;
            } else {
                throw std::runtime_error("常量声明格式错误，缺少 ',' 或 ';'");
            }
        }
    }else if (token.first == "semicolon") {
        advance();
    }
    else {
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
    // 表达式 → 项 表达式'
    int val1 = term();
    return expressionPrime(val1);
}

// 表达式' → + 项 {val = val1 + val2} 表达式' | - 项 {val = val1 - val2} 表达式' | ε
int Parser::expressionPrime(int inherited) {
    auto token = peek();
    if (token.first == "plus" || token.first == "minus") {
        advance(); // 消费 + 或 -
        int val2 = term();
        int result = (token.first == "plus") ? (inherited + val2) : (inherited - val2);
        return expressionPrime(result); // 继续处理表达式'
    }
    return inherited; // ε
}

int Parser::term() {
    // 项 → 因子 项'
    int val1 = factor();
    return termPrime(val1);
}

// 项' → * 因子 {val = val1 * val2} 项' | / 因子 {val = val1 / val2} 项' | ε
int Parser::termPrime(int inherited) {
    auto token = peek();
    if (token.first == "times" || token.first == "slash") {
        advance(); // 消 * 或 /
        int val2 = factor();
        int result;
        if (token.first == "times") {
            result = inherited * val2;
        } else {
            if (val2 == 0) throw std::runtime_error("除以零错误");
            result = inherited / val2;
        }
        return termPrime(result); // 继续处理项'
    }
    return inherited; // ε
}

int Parser::factor() {
    auto token = peek();

    // 处理一元加减符号：+因子 | -因子
    if (token.first == "plus" || token.first == "minus") {
        advance();
        int val = factor();
        return (token.first == "minus") ? -val : val;
    }

    if (token.first == "number") {
        advance();
        return std::stoi(token.second);
    } else if (token.first == "ident") {
        advance();
        auto it = constTable.find(token.second);
        if (it == constTable.end()) {
            throw std::runtime_error("未定义的标识符: " + token.second);
        }
        return it->second;
    } else if (token.first == "lparen") {
        advance();
        int val = expression();
        match("rparen");
        return val;
    } else {
        throw std::runtime_error("因子格式错误，当前为: " + token.second);
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
