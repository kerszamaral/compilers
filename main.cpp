// main.cpp file made by Ian Kersz Amaral - 2025/1
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

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
    if (args.size() != 1)
    {
        printf("No input file provided, use: %s <input file>\n", argv[0]);
        std::exit(WRONG_ARGS_ERROR);
    }

    auto infile = fopen(args[0].c_str(), "r");
    if (!infile)
    {
        printf("Cannot open file %s... \n", args[0].c_str());
        std::exit(NO_FILE_ERROR);
    }
    yyin = infile;
    
    initMe();
    
    const auto result = yy::parser().parse();
    const auto num_lines = getLineNumber();
    if (g_AST != nullptr)
    {
        std::cerr << print_tree(g_AST) << std::endl;
    }
    fclose(yyin);
    yylex_destroy();
    std::cerr << "Generated Symbol Table: \n";
    std::cerr << generateSymbolTable();
    std::cerr << "Lines: " << num_lines << std::endl;
    return result;
}
