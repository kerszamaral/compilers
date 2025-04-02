// main.cpp file made by Ian Kersz Amaral - 2025/1
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

extern int yyparse();
extern int yylex_destroy(void);
extern FILE *yyin;

extern void initMe(void);
extern int getLineNumber(void);
extern void printSymbolTable(void);


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

    auto result = yyparse();
    fprintf(stderr, "\nLines: %d\n", getLineNumber());

    fclose(yyin);
    yylex_destroy();
    
    fprintf(stderr, "Symbol Table:\n");
    printSymbolTable();
    
    return result;
}
