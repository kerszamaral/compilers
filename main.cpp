// main.cpp file made by Ian Kersz Amaral - 2025/1
#include <stdio.h>
#include <stdlib.h>
#include <string>

int yyparse();
extern int yylex_destroy(void);
extern FILE *yyin;

void initMe(void);
int getLineNumber(void);
void printSymbolTable(void);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No input file provided, use: %s <input file>\n", argv[0]);
        exit(1);
    }
    
    if (0 == (yyin = fopen(argv[1], "r")))
    {
        printf("Cannot open file %s... \n", argv[1]);
        exit(1);
    }
    
    initMe();

    int ret = yyparse();

    fclose(yyin);
    yylex_destroy();

    fprintf(stderr, "\nLines: %d\n", getLineNumber());
    
    fprintf(stderr, "Symbol Table:\n");
    printSymbolTable();
    
    return 0;
}
