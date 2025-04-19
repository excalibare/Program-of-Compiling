#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>

class Error
{
public:
    enum class ErrorType
    {
        ILLEGAL_CHAR,
        INVALID_NUMBER,
        UNCLOSED_STR,
        INVALID_IDENT,
        MISSING_RPAREN,       // 缺少右括号
        UNEXPECTED_TOKEN,     // 非法 token
        EXTRA_TOKEN,          // 表达式后多余符号
        INVALID_FACTOR        // 非法因子
    };

    static const std::string ERROR_ILLEGAL_CHAR;
    static const std::string ERROR_INVALID_NUMBER;
    static const std::string ERROR_UNCLOSED_STR;
    static const std::string ERROR_INVALID_IDENT;
    static const std::string ERROR_MISSING_RPAREN;
    static const std::string ERROR_UNEXPECTED_TOKEN;
    static const std::string ERROR_EXTRA_TOKEN;
    static const std::string ERROR_INVALID_FACTOR;

    static std::string getErrorMessage(ErrorType type)
    {
        switch (type)
        {
        case ErrorType::ILLEGAL_CHAR:
            return ERROR_ILLEGAL_CHAR;
        case ErrorType::INVALID_NUMBER:
            return ERROR_INVALID_NUMBER;
        case ErrorType::UNCLOSED_STR:
            return ERROR_UNCLOSED_STR;
        case ErrorType::INVALID_IDENT:
            return ERROR_INVALID_IDENT;
        case ErrorType::MISSING_RPAREN:
            return ERROR_MISSING_RPAREN;
        case ErrorType::UNEXPECTED_TOKEN:
            return ERROR_UNEXPECTED_TOKEN;
        case ErrorType::EXTRA_TOKEN:
            return ERROR_EXTRA_TOKEN;
        case ErrorType::INVALID_FACTOR:
            return ERROR_INVALID_FACTOR;
        default:
            return "UNKNOWN_ERROR";
        }
    }

    static void printError(ErrorType type, const std::string &context = "", int line = -1, int column = -1)
    {
        std::cerr << "[";
        if (line != -1 && column != -1)
        {
            std::cerr << "Line " << line << ", Column " << column << "] ";
        }
        std::cerr << getErrorMessage(type);
        if (!context.empty())
        {
            std::cerr << ": " << context;
        }
        std::cerr << std::endl;
    }
};

class ParseException : public std::exception {
    std::string message;

public:
    ParseException(Error::ErrorType type, const std::string& context = "")
    {
        message = Error::getErrorMessage(type);
        if (!context.empty()) {
            message += ": " + context;
        }
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};


#endif // ERROR_H