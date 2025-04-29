#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
// 强制使用UTF-8
#include <windows.h>

#include "Lexer.h"
#include "Parser.h"
#include "Utils.h"


#define endl "\n"

using namespace std;

int main(){
    // 强制使用UTF-8
    // 切输出/输入都用 UTF‑8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // 如有需要，也让 C++ IO 流用本地环境
    std::ios::sync_with_stdio(true);
    std::locale::global(std::locale(""));


    // 从文件读取输入
    vector<string> inputFiles = {
        "../src/input/case01.txt",
        "../src/input/case02.txt",
        "../src/input/case03.txt",
        "../src/input/case04.txt",
        "../src/input/case05.txt",
        "../src/input/case06.txt",
    "../src/input/parse.txt",};


    for (size_t i = 0; i < inputFiles.size(); i++) {
        ifstream file(inputFiles[i]);
        if (!file.is_open()) {
            cout << "Could not open file" << endl;
            return 0;
        }else {
            cerr << "现分析第" << i+1 << "个文件: " << inputFiles[i] << endl;
        }

        string input((istreambuf_iterator<char>(file)),
                     istreambuf_iterator<char>());
        Lexer lexer;
        // 执行词法分析
        auto tokens = lexer.analyze(input);
        Parser parser(tokens);
        parser.debug_off(); // 开启调试信息
        parser.analyze();  // 分析整个 token 流（支持 if, while, begin 等）
    }


    // vector<vector<pair<string,string>>> expressions = {
    //     {{"lparen","("},{"number","3"},{"plus","+"},{"number","4"}},
    //     {{"number","3"},{"plus","+"},{"times","*"},{"number","5"}},
    //     {{"times","*"},{"number","22"},{"plus","+"},{"number","43"}},
    //     {{"ifsym","if"},{"number","2"},{"plus","+"},{"number","4"}},
    //     {{"number","1"},{"plus","+"},{"number","2"},{"ident","x"}},
    // };

    // 从结果中提取所有表达式并进行词法分析
    // auto expressions = extractExpressions(tokens);
    // for (const auto &[rawText, exprTokens] : expressions)
    // {
    //
    //     Parser parser(exprTokens);
    //     parser.debug_on();
    //     // parser.debug_off();
    //     std::cout << "\n正在分析表达式: " << rawText << endl;
    //     parser.analyze();
    //
    // }
}

// int main()
// {
//     // 从文件读取输入
//     vector<string> inputFiles = {
//         "../src/input/case01.txt",
//         "../src/input/case02.txt",
//         "../src/input/case03.txt",
//         "../src/input/case04.txt",
//         "../src/input/case05.txt",
//         "../src/input/case06.txt",
//     "../src/input/parse.txt",};
//
//     vector<string> outputFiles = {
//         "../src/output/output01.txt",
//         "../src/output/output02.txt",
//         "../src/output/output03.txt",
//         "../src/output/output04.txt",
//         "../src/output/output05.txt",
//         "../src/output/output06.txt",
//         "../src/output/outputParse.txt"};
//
//     for (size_t i = 0; i < inputFiles.size(); i++)
//     {
//         ifstream file(inputFiles[i]);
//         if (!file) {
//             cout << "<UNKNOW_FILE>" << inputFiles[i] << "<UNKNOW_FILE>" << endl;
//         } else {
//             cout << "<SUCCESS>" << inputFiles[i] << "<SUCCESS>" << endl;
//         }
//
//         string input((istreambuf_iterator<char>(file)),
//                      istreambuf_iterator<char>());
//         Lexer lexer;
//         // 执行词法分析
//         auto tokens = lexer.analyze(input);
//
//         // 计算最大类型长度用于对齐
//         size_t max_type_len = 0;
//         for (const auto &t : tokens)
//         {
//             if (t.first.length() > max_type_len)
//             {
//                 max_type_len = t.first.length();
//             }
//         }
//
//         // 输出结果
//         ofstream out(outputFiles[i]);
//         for (const auto &t : tokens)
//         {
//             out << "(" << left << setw(max_type_len) << t.first
//                 << ", " << setw(4) << t.second << ")" << endl;
//         }
//     }
//     // system("pause");
//     return 0;
// }