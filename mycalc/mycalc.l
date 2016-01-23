%{
#include <stdio.h>
#include "y.tab.h"

int
yywrap(void)
{
    return 1;
}

int nr_token = 0;
%}
%%
"+" { 
    nr_token++;
    return ADD;
}
"-" {
    nr_token++;
    return SUB;
}
"*" {
    nr_token++;
    return MUL;
}
"/" {
    nr_token++;
    return DIV;
}
"(" {
    nr_token++;
    return LP;
}
")" {
    nr_token++;
    return RP;
}
"\n" return CR;
([1-9][0-9]*)|0|([0-9]+\.[0-9]+) {
    nr_token++;
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.double_value = temp;
    return DOUBLE_LITERAL;
}
[ \t] ;
. {
    fprintf(stderr, "lexical error.\n");
    exit(1);
}
%%

