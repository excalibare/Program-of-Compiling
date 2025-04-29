//
// Created by LTY on 2025/4/19.
//

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <utility>
#include <iostream>

using namespace std;

struct Quadruple {
    std::string op;     // 运算符
    std::string arg1;   // 左操作数
    std::string arg2;   // 右操作数
    std::string result; // 结果
};

// 采用递归子程序法进行语法分析
class Parser {
public:
    explicit Parser(const std::vector<std::pair<std::string, std::string>>& tokens);
    void analyze();
    void debug_on();
    void debug_off();

private:
    static bool debug;
    std::vector<std::pair<std::string, std::string>> tokens;
    int current; // 当前token下标
    std::vector<Quadruple> quads; // 四元式序列
    int tempVarCount = 0;         // 临时变量计数器

    string expression();      // <表达式>
    string term();            // <项>
    string factor();          // <因子>
    void statement();       // <语句>
    void condition();       // <判断条件>
    void match(const std::string& expectedType); // 匹配当前token类型

    bool isAddOp(const std::string& type);
    bool isMulOp(const std::string& type);
    bool isRelOp(const std::string& type); // 是否为判断语句

    std::pair<std::string, std::string> peek(int offset = 0);
    void advance();
    bool isAtEnd() const;

    void log(const std::string& msg);
    void printQuads() const;

    // 提供给中间代码生成
    std::string newTemp() {
        return "t" + std::to_string(tempVarCount++);
    }
};

#endif
