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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("call: ./etapa1 input.txt \n");
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
    fprintf(stderr, "\nLines: %d\n", getLineNumber());
    fclose(yyin);
    return 0;
}
