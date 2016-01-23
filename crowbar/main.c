#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    extern int yyparse();
    extern FILE *yyin;
    yyparse();
}
