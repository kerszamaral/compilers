// main.cpp file made by Ian Kersz Amaral - 2025/1
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "parser.tab.hpp"
extern int yylex_destroy(void);
extern FILE *yyin;

extern void initMe(void);
extern LineNumber getLineNumber(void);
extern void printSymbolTable(void);

node g_AST = nullptr;

#define WRONG_ARGS_ERROR 1
#define NO_FILE_ERROR 2

int main(int argc, char **argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.size() != 2)
    {
        std::cerr << "No input or output file provided. ";
        std::cerr << "Usage: " << argv[0] << " <input file> <output file>" << std::endl;
        std::exit(WRONG_ARGS_ERROR);
    }

    auto infile = fopen(args[0].c_str(), "r");
    if (!infile)
    {
        std::cerr << "Error opening file " << args[0] << std::endl;
        std::cerr << "Please check if the file exists and is readable." << std::endl;
        std::exit(NO_FILE_ERROR);
    }
    
    yyin = infile;
    
    initMe();
    
    const auto result = yy::parser().parse();
    const auto num_lines = getLineNumber();
    if (g_AST != nullptr)
    {
        std::cerr << "Generated the AST: \n";
        std::cerr << print_tree(g_AST) << std::endl;
        
    }
    if (result != 0)
    {
        std::cerr << "Error parsing the file. Please check the syntax." << std::endl;
        std::exit(result);
    }
    fclose(yyin);
    yylex_destroy();
    std::cerr << "Generated Symbol Table: \n";
    std::cerr << generateSymbolTable();
    std::cerr << "Lines: " << num_lines << std::endl;

    std::ofstream outfile(args[1], std::ios::out);
    if (!outfile || !outfile.is_open() || outfile.bad())
    {
        std::cerr << "Error opening file " << args[1] << std::endl;
        std::cerr << "Please check if the file exists and is writable." << std::endl;
        std::exit(NO_FILE_ERROR);
    }
    if (g_AST != nullptr && result == 0)
    {
        outfile << "// Exported AST -- Ian Kersz - 2025/1 \n";
        outfile << g_AST->export_tree();
    }
    outfile.close();
    
    std::cerr << "Exported AST to file: " << args[1] << std::endl;

    return result;
}
