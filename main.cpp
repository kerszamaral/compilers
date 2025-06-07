// main.cpp file made by Ian Kersz Amaral - 2025/1
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "symbol.hpp"
#include "ast.hpp"
#include "parser.tab.hpp"
#include "checkers.hpp"
#include "tac.hpp"

extern int yylex_destroy(void);
extern FILE *yyin;

node g_AST = nullptr;

static constexpr auto WRONG_ARGS_ERROR = 1;
static constexpr auto NO_FILE_ERROR = 2;
static constexpr auto SEMANTIC_ERROR = 4;

int main(int argc, char **argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.size() == 1)
    {
        std::cerr << "No output file provided. ";
        std::cerr << "Using default output file: out.txt" << std::endl;    
        args.push_back("out.txt");
    }
    else if (args.size() != 2)
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
    if (result != 0)
    {
        std::cerr << "Error parsing the file. Please check the syntax." << std::endl;
        std::exit(result);
    }
    fclose(yyin);
    yylex_destroy();
    
    if (g_AST == nullptr)
    {
        std::cerr << "Error Generating AST!!!!" << std::endl;
        std::exit(-1);
    }

    const auto [number_of_errors, error_messages] = run_semantic_analysis(g_AST);
    std::cerr << "Generated the AST: \n";
    std::cerr << print_tree(g_AST);
    std::cerr << "Generated Symbol Table: \n";
    std::cerr << generateSymbolTable();
    std::cerr << "Lines: " << num_lines << std::endl;
    if (number_of_errors != 0)
    {
        std::cerr << "\n" << std::to_string(number_of_errors) << " Semantic Errors found:\n";
        std::cerr << error_messages;
        std::exit(SEMANTIC_ERROR);
    }

    const auto tac = TAC::generate_tacs(g_AST);
    if (tac == nullptr)
    {
        std::cerr << "Error generating TAC from AST." << std::endl;
        std::exit(-1);
    }
    std::cerr << "Generated TAC: \n";
    std::cerr << "TAC in backwards order: \n";
    std::cerr << TAC::tac_string_backwards(tac) << std::endl;
    std::cerr << "TAC in forward order: \n";
    const auto last_tac = TAC::build_forward_links(tac);
    std::cerr << TAC::tac_string(last_tac) << std::endl;

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
