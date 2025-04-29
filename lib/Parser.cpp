#include "Parser.h"
#include "Error.h"

Parser::Parser(const std::vector<std::pair<std::string, std::string> > &tokens)
    : tokens(tokens), current(0) {
}

void Parser::printQuads() const {
    for (size_t i = 0; i < quads.size(); ++i) {
        const auto &q = quads[i];
        std::cerr << i << ": ( " << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << " )" << std::endl;
    }
}

void Parser::analyze() {
    try {
        statement();
        if (!isAtEnd()) {
            throw std::runtime_error("存在多余符号，语句后仍有内容: '" + peek().second + "'");
        }
        std::cerr << "语法正确!" << std::endl;
        printQuads();
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
    } else if (token.first == "ident" && peek(1).first == "becomes") {
        log("匹配赋值语句");
        std::string varName = token.second;
        advance(); // consume ident
        advance(); // consume :=
        std::string exprResult = expression(); // 得到右边的计算结果
        quads.push_back({"=", exprResult, "", varName});
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


// void Parser::expression() {
//     auto token = peek();
//     // [+|-]
//     if (isAddOp(token.first)) {
//         log("匹配前导加法运算符: " + token.second);
//         advance();
//     }
//     // <项>
//     term();
//     // {<加法运算符> <项>}
//     while (!isAtEnd() && isAddOp(peek().first)) {
//         log("匹配加法运算符: " + peek().second);
//         advance();
//         term();
//     }
// }


std::string Parser::expression() {
    std::string left = term();

    while (!isAtEnd() && isAddOp(peek().first)) {
        std::string op = peek().second;
        log("匹配加法运算符: " + op);
        advance();
        std::string right = term();
        std::string temp = newTemp();
        quads.push_back({op, left, right, temp});
        left = temp;
    }

    return left;
}

// void Parser::term() {
//     // <因子>
//     factor();
//     // {<乘法运算符> <因子>}
//     while (!isAtEnd() && isMulOp(peek().first)) {
//         log("匹配乘法运算符: " + peek().second);
//         advance();
//         factor();
//     }
// }

std::string Parser::term() {
    string left = factor();

    while (!isAtEnd() && isMulOp(peek().first)) {
        std::string op = peek().second; // 获取运算符（* 或 /）
        log("匹配乘法运算符: " + op);
        advance(); // 消费运算符
        std::string right = factor(); // 获取下一个因子
        std::string temp = newTemp(); // 新建临时变量
        quads.push_back({op, left, right, temp}); // 生成四元式
        left = temp; // 更新 left 为临时变量，为下一轮可能的乘除做准备
    }

    return left; // 返回最终结果（变量名或临时变量）
}


// void Parser::factor() {
//     // <标识符>|<无符号整数>| ‘(’<表达式>‘)’
//     auto token = peek();
//     if (token.first == "ident" || token.first == "number") {
//         log("匹配因子: " + token.second);
//         advance();
//     } else if (token.first == "lparen") {
//         log("匹配左括号: " + token.second);
//         advance();
//         expression(); // 进入表达式递归
//         if (peek().first == "rparen") {
//             log("匹配右括号: " + peek().second);
//             advance();
//         } else {
//             // Error::printError(Error::ErrorType::MISSING_RPAREN,"当前位置: " + peek(-1).second);
//             throw std::runtime_error("缺少右括号，当前位置: " + peek(-1).second);
//         }
//     } else {
//         // Error::printError(Error::ErrorType::INVALID_FACTOR,"应为标识符、数字或括号表达式，但实际为: '" + token.second + "'");
//         throw std::runtime_error("因子格式错误，应为标识符、数字或括号表达式，但实际为: '" + token.second + "'");
//     }
// }


std::string Parser::factor() {
    auto token = peek();
    if (token.first == "ident" || token.first == "number") {
        log("匹配因子: " + token.second);
        advance();
        return token.second; // 返回标识符名或数字
    } else if (token.first == "lparen") {
        log("匹配左括号: " + token.second);
        advance();
        std::string value = expression(); // 递归
        if (peek().first == "rparen") {
            log("匹配右括号: " + peek().second);
            advance();
            return value;
        } else {
            throw std::runtime_error("缺少右括号，当前位置: " + peek(-1).second);
        }
    } else {
        throw std::runtime_error("因子格式错误，应为标识符、数字或括号表达式，但实际为: '" + token.second + "'");
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
