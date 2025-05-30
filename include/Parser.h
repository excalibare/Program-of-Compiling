//
// Created by LTY on 2025/4/19.
//

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <utility>
#include <iostream>

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

    void expression();      // <表达式>
    void term();            // <项>
    void factor();          // <因子>
    void match(const std::string& expectedType); // 匹配当前token类型

    bool isAddOp(const std::string& type);
    bool isMulOp(const std::string& type);

    std::pair<std::string, std::string> peek(int offset = 0);
    void advance();
    bool isAtEnd() const;

    void log(const std::string& msg);
};

#endif
