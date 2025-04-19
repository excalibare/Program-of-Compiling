#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <cctype>
#include <algorithm>

#include "Error.h"

// �ʷ��������࣬���ڽ������Դ�����ַ���ת��Ϊһϵ�еĴʷ���Ԫ�����ƣ�
class Lexer
{
public:
    Lexer();
    // ���ܣ����дʷ������������������Դ�����ַ��������أ��ʷ���Ԫ����
    std::vector<std::pair<std::string, std::string>> analyze(const std::string &input);

private:
    std::map<std::string, std::string> keywords;  // �ؼ��ֱ�����Ϊ�ؼ��֣�ֵΪ��Ӧ�Ĵʷ���Ԫ����
    std::map<std::string, std::string> operators; // ������������Ϊ��������ֵΪ��Ӧ�Ĵʷ���Ԫ����
    std::map<char, std::string> delimiters;       // �ָ���������Ϊ�ָ����ַ���ֵΪ��Ӧ�Ĵʷ���Ԫ����

    std::vector<std::pair<std::string, std::string>> tokens; // �洢�ʷ��������������
    std::string source;                                      // �����Դ�����ַ���
    size_t pos;                                              // ��ǰ�������ַ�λ��
    size_t length;                                           // Դ�����ַ����ĳ���

    bool escaped;
    bool closed;
    bool has_dot;
    bool has_exp;
    bool valid;

    void initSymbolTables();
    void skipWhitespace();

    // ����Ƿ��Ѿ������������ַ���
    bool isAtEnd() const;

    // �鿴��ǰλ�õ��ַ��������ƶ�λ��
    char peek() const;

    // ��ȡ��ǰλ�õ��ַ�������λ������ƶ�һλ
    char advance();

    // ��һ���ʷ���Ԫ�����ͣ�ֵ�����ӵ����������
    void addToken(const std::string &type, const std::string &value);

    // �����ַ���������
    void handleString();

    // ������ʶ���͹ؼ���
    void handleIdentifier();

    // ��������������
    void handleNumber();

    // �����������ͷָ���
    void handleOperatorOrDelimiter();
};

#endif // LEXER_H