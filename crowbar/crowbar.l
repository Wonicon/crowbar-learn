%{
#include <stdio.h>
#include <string.h>
//#include "DBG.h"
#include "crowbar.h"
#include "y.tab.h"

int yywrap(void)
{
    return 1;
}

static void increment_line_number()
{
    // TODO
}
%}
%start COMMENT STRING_LITERAL_STATE
%%
<INITIAL>"function" return FUNCTION;
<INITIAL>"if" return IF;
<INITIAL>"else" return ELSE;
<INITIAL>"elsif" return ELSIF;
<INITIAL>"while" return WHILE;
<INITIAL>"for" return FOR;
<INITIAL>"return" return RETURN_T;
<INITIAL>"break" return BREAK;
<INITIAL>"continue" return CONTINUE;
<INITIAL>"null" return NULL_T;
<INITIAL>"true" return TRUE_T;
<INITIAL>"false" return FALSE_T;
<INITIAL>"global" return GLOBAL_T;
<INITIAL>"(" return LP;
<INITIAL>")" return RP;
<INITIAL>"{" return LC;
<INITIAL>"}" return RC;
<INITIAL>";" return SEMICOLON;
<INITIAL>"," return COMMA;
<INITIAL>"&&" return LOGICAL_AND;
<INITIAL>"||" return LOGICAL_OR;
<INITIAL>"=" return ASSIGN;
<INITIAL>"==" return EQ;
<INITIAL>"!=" return NE;
<INITIAL>">" return GT;
<INITIAL>">=" return GE;
<INITIAL>"<" return LT;
<INITIAL>"<=" return LE;
<INITIAL>"+" return ADD;
<INITIAL>"-" return SUB;
<INITIAL>"*" return MUL;
<INITIAL>"/" return DIV;
<INITIAL>"%" return MOD;
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    return IDENTIFIER;
}
<INITIAL>([1-9][0-9]*)|"0" {
    return INT_LITERAL;
}
<INITIAL>[0-9]+\.[0-9]+ {
    return DOUBLE_LITERAL;
}
<INITIAL>"\"" {
    BEGIN STRING_LITERAL_STATE;
}
<INITIAL>[ \t] { ; }
<INITIAL>\n { increment_line_number(); }
<INITIAL># { BEGIN COMMENT; }
<INITIAL>. {
    fprintf(stderr, "Unexpected char(%s)", yytext);
    exit(1);
}
<COMMENT>\n {
    increment_line_number();
    BEGIN INITIAL;
}
<COMMENT>. ;
<STRING_LITERAL_STATE>"\"" {
    BEGIN INITIAL;
    return STRING_LITERAL;
}
<STRING_LITERAL_STATE>\n {
    increment_line_number();
}
<STRING_LITERAL_STATE>"\\\"" {
    ; // TODO
}
<STRING_LITERAL_STATE>"\\\n" {
    ; // TODO
}
<STRING_LITERAL_STATE>"\\t" {
    ; // TODO
}
<STRING_LITERAL_STATE>"\\\\" {
    ; // TODO
}
<STRING_LITERAL_STATE>. {
    ; // TODO
}
%%

