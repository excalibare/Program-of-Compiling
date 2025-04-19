#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

#include "Lexer.h"
#include "Parser.h"
#include "Utils.h"

#define endl "\n"

using namespace std;



int main(){
    ifstream file("../src/input/parse.txt");
    if (!file.is_open()) {
        cout << "Could not open file" << endl;
        return 0;
    }else {
        cout << "File opened successfully" << endl;
    }

    string input((istreambuf_iterator<char>(file)),
                 istreambuf_iterator<char>());
    Lexer lexer;
    // 执行词法分析
    auto tokens = lexer.analyze(input);

    // vector<vector<pair<string,string>>> expressions = {
    //     {{"lparen","("},{"number","3"},{"plus","+"},{"number","4"}},
    //     {{"number","3"},{"plus","+"},{"times","*"},{"number","5"}},
    //     {{"times","*"},{"number","22"},{"plus","+"},{"number","43"}},
    //     {{"ifsym","if"},{"number","2"},{"plus","+"},{"number","4"}},
    //     {{"number","1"},{"plus","+"},{"number","2"},{"ident","x"}},
    // };

    // 从结果中提取所有表达式并进行词法分析
    auto expressions = extractExpressions(tokens);
    for (const auto &expr : expressions)
    {
        Parser parser(expr);
        parser.debug_on();
        parser.analyze();
    }
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