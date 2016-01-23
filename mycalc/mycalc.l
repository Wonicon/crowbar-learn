%{
#include <stdio.h>
#include "y.tab.h"

int
yywrap()
{
    return 1;
}
%}
%%
"+" return ADD;
"-" return SUB;
"*" return MUL;
"/" return DIV;
"\n" return CR;
([1-9][0-9]*)|0|([0-9]+\.[0-9]+) {
    double temp;
    sscanf(yytext, "%lf", &temp);
    return DOUBLE_LITERAL;
}
[ \t] ;
. {
    fprintf(stderr, "lexical error.\n");
    exit(1);
}
%%

