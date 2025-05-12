#include "Parser.h"
#include <string>    // For std::to_string

// Initialize static debug member
bool Parser::debug_mode_ = true;

Parser::Parser(const std::vector<std::pair<std::string, std::string> > &tokens)
    : tokens_(tokens), currentTokenIndex_(0), tempCounter_(0) {
}

void Parser::debug_on() {
    debug_mode_ = true;
}

void Parser::debug_off() {
    debug_mode_ = false;
}

void Parser::log(const std::string &msg) const {
    if (debug_mode_) {
        std::cerr << "[Debug Parser] " << msg << std::endl;
    }
}

void Parser::printQuads() const {
    std::cerr << "\n--- Generated Quadruples ---" << std::endl;
    for (size_t i = 0; i < quads_.size(); ++i) {
        const auto &q = quads_[i];
        std::cerr << i << ": ( " << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << " )" << std::endl;
    }
    std::cerr << "--- End of Quadruples ---" << std::endl;
}

std::string Parser::newTemp() {
    return "T" + std::to_string(tempCounter_++);
}

std::pair<std::string, std::string> Parser::peek(int offset) const {
    if (currentTokenIndex_ + offset >= tokens_.size() || currentTokenIndex_ + offset < 0) {
        return {"EOF", ""}; // Return EOF if out of bounds
    }
    return tokens_[currentTokenIndex_ + offset];
}

void Parser::advance() {
    if (!isAtEnd()) {
        currentTokenIndex_++;
    }
}

bool Parser::isAtEnd() const {
    return currentTokenIndex_ >= tokens_.size() || peek().first == "EOF";
}

void Parser::match(const std::string &expectedType) {
    if (isAtEnd()) {
        throw std::runtime_error("匹配失败: 期望类型为 '" + expectedType + "', 但已到达Token流末尾.");
    }
    if (peek().first == expectedType) {
        log("匹配成功: " + expectedType + " ('" + peek().second + "')");
        advance();
    } else {
        throw std::runtime_error(
            "匹配失败: 期望类型为 '" + expectedType +
            "', 实际类型为 '" + peek().first +
            "', 值为 '" + peek().second + "'");
    }
}

bool Parser::isAddOp(const std::string &type) const {
    return type == "plus" || type == "minus";
}

bool Parser::isMulOp(const std::string &type) const {
    return type == "times" || type == "slash";
}

bool Parser::isRelOp(const std::string &type) const {
    return type == "eql" || type == "neq" || type == "lss" ||
           type == "leq" || type == "gtr" || type == "geq";
}

void Parser::analyze() {
    log("开始语法分析...");
    try {
        // In a full PL/0 parser, you'd call program() or block().
        // For now, assuming analyze processes statements until EOF or error.
        while(!isAtEnd() && peek().first != "EOF") { // Process multiple statements if they appear sequentially without BEGIN/END
             statement();
             // If PL/0 expects semicolons between top-level statements, handle here or ensure statement() consumes them.
             // For now, this loop will parse statements one after another.
             // If a statement doesn't consume a token that leads to the next (e.g. semicolon),
             // and the next token is not start of a statement, it might fail or loop infinitely.
             // This simple loop is for parsing a sequence of statements directly.
             // If your input files contain full PL/0 programs (e.g. with a final '.'),
             // you'll need a top-level `program()` rule.
             if (isAtEnd() || peek().first == "EOF") break; // Exit if done

             // This check is a bit simplistic. A proper PL/0 program structure
             // (like block ending with '.') should be handled by a top-level rule.
             // If an unhandled token remains that's not the start of a known statement,
             // it will be caught by statement() or here.
        }


        if (!isAtEnd() && peek().first != "EOF") { // Check for unconsumed tokens that are not EOF
             throw std::runtime_error("存在多余符号，语句后仍有内容: '" + peek().second + "' (类型: " + peek().first + ")");
        }
        std::cerr << "语法分析成功!" << std::endl;
        printQuads();
    } catch (const std::exception &e) {
        std::cerr << "语法错误: " << e.what() << std::endl;
        // Optionally print quads generated so far for debugging
        if (!quads_.empty()) {
            std::cerr << "已生成的四元式 (可能不完整):" << std::endl;
            printQuads();
        }
    }
    log("语法分析结束.");
}

void Parser::factor() {
    log("进入 factor, 当前 Token: " + peek().first + " ('" + peek().second + "')");
    auto token = peek();
    if (token.first == "ident" || token.first == "number") {
        log("因子为 ident/number: " + token.second);
        semanticStack_.push(token.second);
        advance();
    } else if (token.first == "lparen") {
        log("因子为 (表达式)");
        advance(); // consume '('
        expression(); // Result of expression will be left on semanticStack_
        match("rparen"); // consume ')'
        // The result of the (expression) is already on top of semanticStack_
    } else {
        throw std::runtime_error("因子格式错误: 应为标识符、数字或 '(表达式)', 但得到: '" + token.first + " (" + token.second + ")'");
    }
    log("退出 factor, 栈顶: " + (semanticStack_.empty() ? "empty" : semanticStack_.top()));
}

void Parser::term() {
    log("进入 term");
    factor(); // Result of factor is on semanticStack_

    while (!isAtEnd() && isMulOp(peek().first)) {
        std::pair<std::string, std::string> opToken = peek();
        std::string opSymbol = opToken.second; // Default to token value
        if (opToken.first == "times") opSymbol = "*";
        else if (opToken.first == "slash") opSymbol = "/";

        log("项中匹配乘法运算符: " + opSymbol);
        advance(); // Consume operator
        factor();  // Result of the right-hand factor is on semanticStack_

        if (semanticStack_.size() < 2) throw std::runtime_error("语义栈错误: 乘除法运算缺少操作数");
        std::string rightOperand = semanticStack_.top(); semanticStack_.pop();
        std::string leftOperand = semanticStack_.top(); semanticStack_.pop();
        std::string tempVar = newTemp();

        quads_.push_back({opSymbol, leftOperand, rightOperand, tempVar});
        semanticStack_.push(tempVar);
        log("生成乘除四元式: (" + opSymbol + ", " + leftOperand + ", " + rightOperand + ", " + tempVar + ")");
    }
    log("退出 term, 栈顶: " + (semanticStack_.empty() ? "empty" : semanticStack_.top()));
}

void Parser::expression() {
    log("进入 expression");
    std::string unaryOpStr = ""; // For optional leading + or -
    if (peek().first == "plus" || peek().first == "minus") {
        unaryOpStr = (peek().first == "plus") ? "+" : "-";
        log("表达式前导一元运算符: " + unaryOpStr);
        advance(); // Consume unary operator
    }

    term(); // Result of the first term is on semanticStack_

    if (!unaryOpStr.empty()) {
        if (semanticStack_.empty()) throw std::runtime_error("语义栈错误: 一元运算缺少操作数");
        std::string operand = semanticStack_.top(); semanticStack_.pop();

        if (unaryOpStr == "-") {
            std::string tempVar = newTemp();
            // Represent unary minus, e.g., as (-, 0, operand, temp) or (UMINUS, operand, , temp)
            quads_.push_back({"-", "0", operand, tempVar}); // Using (op, arg1, arg2, result) for unary
            log("生成一元减四元式: (-, 0, " + operand + ", " + tempVar + ")");
            semanticStack_.push(tempVar);
        } else if (unaryOpStr == "+") {
            // Unary plus usually doesn't generate code. The operand itself is the result.
            // If it did, it would be similar to unary minus.
            // For now, assume no code, so push the original operand back.
            semanticStack_.push(operand);
            log("处理一元加运算符 (无四元式生成，操作数 " + operand + " 已在栈顶)");
        }
    }

    while (!isAtEnd() && isAddOp(peek().first)) {
        std::pair<std::string, std::string> opToken = peek();
        std::string opSymbol = opToken.second; // Default to token value
        if (opToken.first == "plus") opSymbol = "+";
        else if (opToken.first == "minus") opSymbol = "-";

        log("表达式中匹配加法运算符: " + opSymbol);
        advance(); // Consume operator
        term();    // Result of the right-hand term is on semanticStack_

        if (semanticStack_.size() < 2) throw std::runtime_error("语义栈错误: 加减法运算缺少操作数");
        std::string rightOperand = semanticStack_.top(); semanticStack_.pop();
        std::string leftOperand = semanticStack_.top(); semanticStack_.pop();
        std::string tempVar = newTemp();

        quads_.push_back({opSymbol, leftOperand, rightOperand, tempVar});
        semanticStack_.push(tempVar);
        log("生成加减四元式: (" + opSymbol + ", " + leftOperand + ", " + rightOperand + ", " + tempVar + ")");
    }
    log("退出 expression, 栈顶: " + (semanticStack_.empty() ? "empty" : semanticStack_.top()));
}

void Parser::statement() {
    log("进入 statement, 当前 Token: " + peek().first + " ('" + peek().second + "')");
    auto token = peek();

    if (token.first == "ident" && peek(1).first == "becomes") {
        log("匹配赋值语句");
        std::string varName = token.second;
        advance(); // consume ident
        match("becomes"); // consume := (advances past it)

        expression(); // Process the expression, result will be on semanticStack_

        if (semanticStack_.empty()) {
            throw std::runtime_error("赋值语句的表达式部分未能产生结果到语义栈");
        }
        std::string exprResultPlace = semanticStack_.top(); semanticStack_.pop();
        // PL/0 assignment is `variable := expression`
        // Quad: (":=", source, "", destination)
        quads_.push_back({":=", exprResultPlace, "", varName});
        log("生成赋值四元式: (:=, " + exprResultPlace + ", , " + varName + ")");

    } else if (token.first == "ifsym") {
        log("匹配 if 语句");
        advance(); // consume 'if'
        condition(); // condition result (a temp var) will be on semanticStack_
                     // For actual jumps, you'd pop this and generate JPC quad.
        if(semanticStack_.empty()) throw std::runtime_error("IF 条件未产生结果到语义栈");
        std::string condResultPlace = semanticStack_.top(); semanticStack_.pop();
        log("IF 条件结果: " + condResultPlace); // Placeholder for using condition

        // Placeholder for jump logic:
        int jpcQuadIndex = quads_.size();
        quads_.push_back({"JPC", condResultPlace, "", " GOTO ? "}); // Jump if condition is false

        match("thensym");
        statement(); // Parse THEN branch

        int jmpQuadIndex = -1;
        if (peek().first == "elsesym") {
            jmpQuadIndex = quads_.size();
            quads_.push_back({"JMP", "", "", " GOTO ? "}); // Jump over ELSE branch
            quads_[jpcQuadIndex].result = std::to_string(quads_.size()); // Backpatch JPC

            log("匹配 else 分支");
            advance(); // consume 'else'
            statement(); // Parse ELSE branch
            quads_[jmpQuadIndex].result = std::to_string(quads_.size()); // Backpatch JMP
        } else {
            quads_[jpcQuadIndex].result = std::to_string(quads_.size()); // Backpatch JPC to after THEN branch
        }
         log("结束 IF 语句");

    } else if (token.first == "whilesym") {
        log("匹配 while 语句");
        advance(); // consume 'while'

        int conditionStartIndex = quads_.size(); // Mark start of condition for jump back
        log("WHILE 条件开始地址: " + std::to_string(conditionStartIndex));

        condition();
        if(semanticStack_.empty()) throw std::runtime_error("WHILE 条件未产生结果到语义栈");
        std::string condResultPlace = semanticStack_.top(); semanticStack_.pop();
        log("WHILE 条件结果: " + condResultPlace);

        // Placeholder for jump logic:
        int jpcQuadIndex = quads_.size();
        quads_.push_back({"JPC", condResultPlace, "", " GOTO ? "}); // Jump if condition is false (exit loop)

        match("dosym");
        statement(); // Parse DO branch (loop body)

        quads_.push_back({"JMP", "", "", std::to_string(conditionStartIndex)}); // Jump back to condition
        quads_[jpcQuadIndex].result = std::to_string(quads_.size()); // Backpatch JPC to after loop
        log("结束 WHILE 语句");

    } else if (token.first == "beginsym") {
        log("匹配 begin ... end 语句块");
        advance(); // 消费 'begin'
        statement();

        while (peek().first == "semicolon") {
            advance(); // 消费分号
            if (peek().first == "endsym") { // Allows for optional semicolon before end
                break;
            }
            statement();
        }
        match("endsym"); // Consumes 'end'
        log("匹配 end");
    } else if (token.first == "readsym") {
        log("匹配 read 语句");
        advance(); // consume 'read'
        match("lparen");
        do {
            if (peek().first != "ident") throw std::runtime_error("READ 语句期望标识符");
            std::string varToRead = peek().second;
            quads_.push_back({"READ", "", "", varToRead});
            log("生成 READ 四元式 for " + varToRead);
            advance(); // consume ident
            if (peek().first == "comma") {
                advance(); // consume comma
            } else break;
        } while(true);
        match("rparen");
    } else if (token.first == "writesym") {
        log("匹配 write 语句");
        advance(); // consume 'write'
        match("lparen");
        do {
            expression(); // expression result on stack
            if(semanticStack_.empty()) throw std::runtime_error("WRITE 语句表达式未产生结果");
            std::string valToWrite = semanticStack_.top(); semanticStack_.pop();
            quads_.push_back({"WRITE", valToWrite, "", ""});
            log("生成 WRITE 四元式 for " + valToWrite);
            if (peek().first == "comma") {
                advance(); // consume comma
            } else break;
        } while(true);
        match("rparen");
    }
    // Add other statement types if necessary (call)
    else if (isAtEnd() || token.first == "EOF" || token.first == "endsym" /* already handled by begin-end */) {
        // Empty statement or end of token stream in a valid way.
        log("语句结束或空语句");
    }
    else {
       throw std::runtime_error("未知或不期望的语句开始: " + token.first + " ('" + token.second + "')");
    }
    log("退出 statement");
}

void Parser::condition() {
    log("进入 condition, 当前 Token: " + peek().first + " ('" + peek().second + "')");
    if (peek().first == "oddsym") {
        log("条件为 odd <表达式>");
        advance(); // consume 'odd'
        expression(); // Result of expression on semanticStack_
        if (semanticStack_.empty()) throw std::runtime_error("ODD 条件的表达式未产生结果");
        std::string exprPlace = semanticStack_.top(); semanticStack_.pop();
        std::string tempVar = newTemp();
        quads_.push_back({"ODD", exprPlace, "", tempVar});
        semanticStack_.push(tempVar);
        log("生成 ODD 四元式: (ODD, " + exprPlace + ", , " + tempVar + ")");
    } else {
        log("条件为 <表达式> <关系符> <表达式>");
        expression(); // Left expression result on semanticStack_

        auto relopToken = peek();
        if (isRelOp(relopToken.first)) {
            std::string relopSymbol = relopToken.second; // Keep original like '==' '<>', or map them to symbols
            // 映射
            if (relopToken.first == "eql") relopSymbol = "==";
            else if (relopToken.first == "neq") relopSymbol = "<>"; // or "!="
            else if (relopToken.first == "lss") relopSymbol = "<";
            else if (relopToken.first == "leq") relopSymbol = "<=";
            else if (relopToken.first == "gtr") relopSymbol = ">";
            else if (relopToken.first == "geq") relopSymbol = ">=";

            log("条件中匹配关系运算符: " + relopSymbol);
            advance(); // consume relop
            expression(); // Right expression result on semanticStack_

            if (semanticStack_.size() < 2) throw std::runtime_error("语义栈错误: 条件关系运算缺少操作数");
            std::string rightExprPlace = semanticStack_.top(); semanticStack_.pop();
            std::string leftExprPlace = semanticStack_.top(); semanticStack_.pop();
            std::string tempVar = newTemp();
            quads_.push_back({relopSymbol, leftExprPlace, rightExprPlace, tempVar});
            semanticStack_.push(tempVar);
            log("生成关系运算四元式: (" + relopSymbol + ", " + leftExprPlace + ", " + rightExprPlace + ", " + tempVar + ")");
        } else {
            throw std::runtime_error("条件中缺少关系运算符，得到: '" + relopToken.first + " (" + relopToken.second + ")'");
        }
    }
    log("退出 condition, 栈顶: " + (semanticStack_.empty() ? "empty" : semanticStack_.top()));
}