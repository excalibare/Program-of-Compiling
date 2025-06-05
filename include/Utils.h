//
// Created by LTY on 2025/4/19.
//

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
using namespace std;
bool debug = true;

// 提取以 := 开始后的表达式 token
vector<pair<string, vector<pair<string, string>>>> extractExpressions(const std::vector<std::pair<std::string, std::string>> &tokens)
{
    vector<pair<string, vector<pair<string, string>>>> expressions;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        vector<pair<string, string>> current;

        if (tokens[i].first == "becomes")
        { // :=
            std::vector<std::pair<std::string, std::string>> expr;
            bool hasError = false;
            string original_expr;
            ++i;
            while (i < tokens.size() && tokens[i].first != "semicolon")
            {
                const std::string &type = tokens[i].first;
                if (type == "ERROR_ILLEGAL_CHAR" ||
                    type == "ERROR_INVALID_NUMBER" ||
                    type == "ERROR_UNCLOSED_STR" ||
                    type == "ERROR_INVALID_IDENT")
                {
                    hasError = true;
                }

                expr.push_back(tokens[i]);
                // std::cout << tokens[i].first << " " << tokens[i].second << std::endl;
                original_expr += tokens[i].second + " ";
                ++i;
            }
            if (!expr.empty() && !hasError) {
                expressions.push_back({original_expr,expr});
            } else {
                if (debug) {
                    std::cerr << "跳过包含词法错误的表达式：" << std::endl;
                    for (const auto& t : expr) {
                        std::cerr << "  " << t.first << " " << t.second << std::endl;
                    }
                }
            }
        }
    }
    return expressions;
}

#endif //UTILS_H
