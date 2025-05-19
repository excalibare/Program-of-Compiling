// Main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <windows.h> // Consider if still needed or use cross-platform UTF-8 handling

#include "Lexer.h"
#include "Parser.h"
#include "DAGOptimizer.h" // Include the new optimizer
#include "Utils.h"


#define endl "\n" // Be careful with this if std::endl's flushing behavior is desired

using namespace std;

// Function to print quads to a stream (e.g., cerr or an ofstream)
void printQuadruples(ostream& os, const std::vector<Parser::Quad>& quads, const std::string& title) {
    os << "\n--- " << title << " ---" << endl;
    for (size_t i = 0; i < quads.size(); ++i) {
        const auto& q = quads[i];
        os << i << ": ( " << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << " )" << endl;
    }
    os << "--- End of " << title << " ---" << endl;
}


int main() {
    // 强制使用UTF-8
    // 切输出/输入都用 UTF‑8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // 如有需要，也让 C++ IO 流用本地环境
    std::ios::sync_with_stdio(true);
    std::locale::global(std::locale(""));

    vector<string> inputFiles = {
        "../src/input/case01.txt", // Add your test case file paths
        "../src/input/case02.txt",
        "../src/input/case03.txt",
        "../src/input/case04.txt",
        "../src/input/case05.txt",
        // Add more test cases: common subexpression, constant folding, dead code, etc.
        "../src/input/test_cse.txt",
        "../src/input/test_const_fold.txt",
        "../src/input/test_assignment.txt",
        "../src/input/test_combined.txt"
    };

    for (size_t i = 0; i < inputFiles.size(); i++) {
        cerr << "--- Analyzing file: " << inputFiles[i] << " ---" << endl;
        ifstream file(inputFiles[i]);
        if (!file.is_open()) {
            cerr << "Could not open file: " << inputFiles[i] << endl;
            continue;
        }

        string input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        Lexer lexer;
        auto tokens = lexer.analyze(input);

        // It seems your parser expects a sequence of statements.
        // The provided Parser.cpp analyze() method processes tokens until EOF.
        // If your input files are full PL/0 programs (e.g. VAR X,Y; BEGIN X:=1; END.),
        // ensure your Parser can handle that structure or adapt inputs.
        // For basic block optimization, the input quads are key.
        // The current Parser generates quads internally.

        Parser parser(tokens);
        // parser.debug_on(); // Enable parser debug if needed
        parser.debug_off();
        try {
            parser.analyze();
        } catch (const std::exception& e) {
            cerr << "Error during parsing " << inputFiles[i] << ": " << e.what() << endl;
            if (!parser.getQuads().empty()) {
                 printQuadruples(cerr, parser.getQuads(), "Generated Quads (incomplete due to error)");
            }
            continue;
        }


        const auto& originalQuads = parser.getQuads();
        printQuadruples(cerr, originalQuads, "Original Quadruples from " + inputFiles[i]);

        // Prepare output file for optimized quads
        string outputFileName = "../src/output/optimized_case" + std::to_string(i + 1) + ".txt";
        ofstream outFile(outputFileName);
        if (!outFile.is_open()) {
            cerr << "Could not open output file: " << outputFileName << endl;
        } else {
             outFile << "Source File: " << inputFiles[i] << endl;
             printQuadruples(outFile, originalQuads, "Original Quadruples");
        }


        // --- DAG Optimization ---
        if (!originalQuads.empty()) {
            DAGOptimizer dag_opt(originalQuads);
            dag_opt.buildAndOptimizeDAG();

            // For debugging the DAG structure:
            // cerr << "--- DAG Structure for " << inputFiles[i] << " ---" << endl;
            // dag_opt.printDAG(); // You'll need to implement printDAG()

            dag_opt.reconstructQuadsFromDAG();
            const auto& optimizedQuads = dag_opt.getOptimizedQuads();
            printQuadruples(cerr, optimizedQuads, "Optimized Quadruples for " + inputFiles[i]);
            if(outFile.is_open()){
                printQuadruples(outFile, optimizedQuads, "Optimized Quadruples");
            }

        } else {
            cerr << "No quadruples generated for " << inputFiles[i] << " to optimize." << endl;
            if(outFile.is_open()){
                 outFile << "No quadruples generated to optimize." << endl;
            }
        }
         if(outFile.is_open()){
            outFile.close();
         }
         cerr << "--- Finished analyzing file: " << inputFiles[i] << " ---" << endl << endl;
    }

    return 0;
}