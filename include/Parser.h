//
// Created by LTY on 2025/4/19.
//

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <stack>
#include <unordered_map>

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
    std::unordered_map<std::string, int> constTable;
    int current; // 当前token下标

    int expression();      // <表达式>
    int expressionPrime(int inherited);

    int term();            // <项>
    int termPrime(int inherited);

    int factor();          // <因子>
    int result;             // <表达式结果>
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
};

#endif
