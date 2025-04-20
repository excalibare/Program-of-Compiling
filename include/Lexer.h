#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <utility>

// 词法分析器类，用于将输入的源代码字符串转换为一系列的词法单元（令牌）
class Lexer
{
public:
    Lexer();
    // 功能：进行词法分析；参数：输入的源代码字符串；返回：词法单元向量
    std::vector<std::pair<std::string, std::string>> analyze(const std::string &input);

private:
    std::map<std::string, std::string> keywords;  // 关键字表，键为关键字，值为对应的词法单元类型
    std::map<std::string, std::string> operators; // 操作符表，键为操作符，值为对应的词法单元类型
    std::map<char, std::string> delimiters;       // 分隔符表，键为分隔符字符，值为对应的词法单元类型

    std::vector<std::pair<std::string, std::string>> tokens; // 存储词法分析结果的向量
    std::string source;                                      // 输入的源代码字符串
    size_t pos;                                              // 当前处理的字符位置
    size_t length;                                           // 源代码字符串的长度

    bool escaped;
    bool closed;
    bool has_dot;
    bool has_exp;
    bool valid;

    void initSymbolTables();
    void skipWhitespace();

    // 检查是否已经处理完输入字符串
    bool isAtEnd() const;

    // 查看当前位置的字符，但不移动位置
    char peek() const;

    // 获取当前位置的字符，并将位置向后移动一位
    char advance();

    // 将一个词法单元（类型，值）添加到结果向量中
    void addToken(const std::string &type, const std::string &value);

    // 处理字符串字面量
    void handleString();

    // 处理标识符和关键字
    void handleIdentifier();

    // 处理数字字面量
    void handleNumber();

    // 处理操作符和分隔符
    void handleOperatorOrDelimiter();
};

#endif // LEXER_H