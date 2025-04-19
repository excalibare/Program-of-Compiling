//
// Created by LTY on 2025/4/19.
//

#ifndef UTILS_H
#define UTILS_H

// 提取以 := 开始后的表达式 token
std::vector<std::vector<std::pair<std::string, std::string>>> extractExpressions(const std::vector<std::pair<std::string, std::string>> &tokens)
{
    std::vector<std::vector<std::pair<std::string, std::string>>> expressions;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].first == "becomes")
        { // :=
            std::vector<std::pair<std::string, std::string>> expr;
            bool hasError = false;
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
                ++i;
            }
            if (!expr.empty() && !hasError) {
                expressions.push_back(expr);
            } else {
                std::cerr << "跳过包含词法错误的表达式：" << std::endl;
                for (const auto& t : expr) {
                    std::cerr << "  " << t.first << " " << t.second << std::endl;
                }
            }
        }
    }
    return expressions;
}

#endif //UTILS_H
