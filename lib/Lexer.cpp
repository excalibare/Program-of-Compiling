#include "Lexer.h"

#include <algorithm>

#include "Error.h"

Lexer::Lexer()
{
    initSymbolTables();
}

void Lexer::initSymbolTables()
{
    // ��ʼ���ؼ��ֱ�
    keywords = {
        {"begin", "beginsym"},
        {"call", "callsym"},
        {"const", "constsym"},
        {"do", "dosym"},
        {"end", "endsym"},
        {"if", "ifsym"},
        {"odd", "oddsym"},
        {"procedure", "proceduresym"},
        {"read", "readsym"},
        {"then", "thensym"},
        {"var", "varsym"},
        {"while", "whilesym"},
        {"write", "writesym"},
        {"else", "elsesym"}};

    // ��ʼ����������
    operators = {
        {":=", "becomes"},
        {"<=", "leq"},
        {">=", "geq"},
        {"<", "lss"},
        {">", "gtr"},
        {"=", "eql"},
        {"#", "neq"},
        {"+", "plus"},
        {"-", "minus"},
        {"*", "times"},
        {"/", "slash"}};

    // ��ʼ���ָ�����
    delimiters = {
        {'(', "lparen"},
        {')', "rparen"},
        {',', "comma"},
        {';', "semicolon"},
        {'.', "period"}};
}

std::vector<std::pair<std::string, std::string>> Lexer::analyze(const std::string &input)
{
    tokens.clear(); // ���֮ǰ�ķ������
    source = input; // ���������Դ�����ַ���
    pos = 0;        // ��ʼ����ǰ����λ��Ϊ 0
    length = source.length();

    while (!isAtEnd())
    {
        skipWhitespace();
        // ��������հ��ַ����Ѿ�����ĩβ�����˳�ѭ��
        if (isAtEnd())
            break;

        // �鿴��ǰ�ַ�
        char ch = peek();

        // �����ǰ�ַ���˫���ţ������ַ���������
        if (ch == '"')
            handleString();
        // �����ǰ�ַ�����ĸ���»��ߣ������ʶ���͹ؼ���
        else if (std::isalpha(ch) || ch == '_')
            handleIdentifier();
        // �����ǰ�ַ������֣���������������
        else if (std::isdigit(ch))
            handleNumber();
        // ������������ͷָ���
        else
            handleOperatorOrDelimiter();
    }

    return tokens;
}

void Lexer::skipWhitespace()
{
    while (!isAtEnd() && std::isspace(peek()))
        advance();
}

bool Lexer::isAtEnd() const
{
    return pos >= length;
}

char Lexer::peek() const
{
    return source[pos];
}

char Lexer::advance()
{
    return source[pos++];
}

void Lexer::addToken(const std::string &type, const std::string &value)
{
    tokens.emplace_back(type, value);
}

void Lexer::handleString()
{
    size_t start = pos;
    advance();       // skip initial quote
    escaped = false; // ��ʼ������
    closed = false;  // ��ʼ������

    while (!isAtEnd())
    {
        char ch = advance();
        if (escaped)
        {
            escaped = false;
            continue;
        }
        if (ch == '\\')
        {
            escaped = true;
        }
        else if (ch == '"')
        {
            closed = true;
            break;
        }
    }

    if (closed)
    {
        addToken("string", source.substr(start, pos - start));
    }
    else
    {
        addToken(Error::ERROR_UNCLOSED_STR, source.substr(start, pos - start));
    }
}

void Lexer::handleNumber()
{
    size_t start = pos;
    has_dot = false; // ��ʼ������
    has_exp = false; // ��ʼ������
    valid = true;    // ��ʼ������

    while (!isAtEnd())
    {
        char c = peek();

        if (std::isdigit(c))
        {
            advance();
        }
        else if (c == '.' && !has_dot && !has_exp)
        {
            has_dot = true;
            advance();
        }
        else if ((c == 'e' || c == 'E') && !has_exp)
        {
            has_exp = true;
            advance();
            if (!isAtEnd() && (peek() == '+' || peek() == '-'))
            {
                advance();
            }
        }
        else if (std::isalpha(c))
        {
            valid = false;
            advance();
        }
        else
        {
            break;
        }
    }

    std::string num = source.substr(start, pos - start);
    if (!valid || std::count(num.begin(), num.end(), '.') > 1)
    {
        addToken(Error::ERROR_INVALID_NUMBER, num);
    }
    else
    {
        addToken("number", num);
    }
}

void Lexer::handleIdentifier()
{
    size_t start = pos;
    advance();
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_'))
        advance();
    std::string ident = source.substr(start, pos - start);

    std::string lower_ident = ident;
    std::transform(lower_ident.begin(), lower_ident.end(), lower_ident.begin(), ::tolower);

    if (keywords.count(lower_ident))
    {
        addToken(keywords[lower_ident], ident);
    }
    else
    {
        if (!std::isalpha(ident[0]) && ident[0] != '_')
            addToken(Error::ERROR_INVALID_IDENT, ident);
        else
            addToken("ident", lower_ident);
    }
}

void Lexer::handleOperatorOrDelimiter()
{
    char ch = peek();
    bool matched = false;

    if (pos + 1 < length)
    {
        std::string two_chars = source.substr(pos, 2);
        if (operators.count(two_chars))
        {
            addToken(operators[two_chars], two_chars);
            pos += 2;
            matched = true;
        }
    }

    if (!matched)
    {
        std::string one_char(1, ch);
        if (operators.count(one_char))
        {
            addToken(operators[one_char], one_char);
            advance();
        }
        else if (delimiters.count(ch))
        {
            addToken(delimiters[ch], one_char);
            advance();
        }
        else
        {
            addToken(Error::ERROR_ILLEGAL_CHAR, one_char);
            advance();
        }
    }
}