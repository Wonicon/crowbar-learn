%{
#include <stdio.h>
#include <stdlib.h>
#define YYDEBUG 1
%}
%union {
    double double_value;
};
%token <double_value> DOUBLE_LITERAL;
%token ADD SUB MUL DIV CR
%type <double_value> expression term primary_expression
%%
line_list: line
         | line_list line
         ;
line: expression CR
    {
        printf(">>%lf\n", $1);
        extern nr_token;
        nr_token = 0;
    }
    | error CR
    {
        yyclearin;
        yyerrok;
        extern nr_token;
        nr_token = 0;
    }
    ;
expression: term
          | expression ADD term
          {
              $$ = $1 + $3;
          }
          | expression SUB term
          {
              $$ = $1 - $3;
          }
          ;
term: primary_expression
    | term MUL primary_expression
    {
        $$ = $1 * $3;
    }
    | term DIV primary_expression
    {
        $$ = $1 / $3;
    }
    ;
primary_expression: DOUBLE_LITERAL
                  {
                      $$ = $1;
                  }
                  ;
%%
int
yyerror(const char *str)
{
    extern char *yytext;
    extern int nr_token;
    fprintf(stderr, "parser error near %s at %d\n", yytext, nr_token);
    return 0;
}

int main()
{
    extern int yyparse();
    extern FILE *yyin;

    yyin = stdin;
    if (yyparse()) {
        fprintf(stderr, "Error!\n");
        exit(1);
    }
}

