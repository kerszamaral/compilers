#include <stdio.h>
#include <stdlib.h>
#include <string>

// lex.yy.h
int yylex();
extern char *yytext;
extern FILE *yyin;

int isRunning(void);
void initMe(void);
int getLineNumber(void);
std::string tokenName(int token);
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

    while (isRunning())
    {
        int token = yylex();

        if (!isRunning())
            break;
        fprintf(stderr, "(%s, %s) %s", yytext, tokenName(token).c_str(), token == ';' ? "\n" : "");
    }
    fclose(yyin);
    
    fprintf(stderr, "\nLines: %d\n", getLineNumber());
    
    fprintf(stderr, "Symbol Table:\n");
    printSymbolTable();
    
    return 0;
}
