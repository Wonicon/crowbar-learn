%{
#include <stdio.h>
#include <string.h>
#include "DBG.h"
#include "crowbar.h"
#include "y.tab.h"

int
yywrap(void)
{
    return 1;
}

static void
increment_line_number()
{
    crb_get_current_interpreter()->current_line_number++;
}
%}
%start COMMENT STRING_LITERAL_STATE
%%
<INITIAL>"function" return FUNCTION;
<INITIAL>"if"       return IF;
<INITIAL>"else"     return ELSE;
<INITIAL>"elsif"    return ELSIF;
<INITIAL>"while"    return WHILE;
<INITIAL>"for"      return FOR;
<INITIAL>"return"   return RETURN_T;
<INITIAL>"break"    return BREAK;
<INITIAL>"continue" return CONTINUE;
<INITIAL>"null"     return NULL_T;
<INITIAL>"true"     return TRUE_T;
<INITIAL>"false"    return FALSE_T;
<INITIAL>"global"   return GLOBAL_T;
<INITIAL>"("        return LP;
<INITIAL>")"        return RP;
<INITIAL>"{"        return LC;
<INITIAL>"}"        return RC;
<INITIAL>";"        return SEMICOLON;
<INITIAL>","        return COMMA;
<INITIAL>"&&"       return LOGICAL_AND;
<INITIAL>"||"       return LOGICAL_OR;
<INITIAL>"="        return ASSIGN;
<INITIAL>"=="       return EQ;
<INITIAL>"!="       return NE;
<INITIAL>">"        return GT;
<INITIAL>">="       return GE;
<INITIAL>"<"        return LT;
<INITIAL>"<="       return LE;
<INITIAL>"+"        return ADD;
<INITIAL>"-"        return SUB;
<INITIAL>"*"        return MUL;
<INITIAL>"/"        return DIV;
<INITIAL>"%"        return MOD;
 /* 标识符: 变量名, 函数名等 */
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    yylval.identifier = crb_create_identifier(yytext);
    return IDENTIFIER;
}
 /* 整型数值 */
<INITIAL>([1-9][0-9]*)|"0"      {
    Expression *expr = crb_alloc_expression(INT_EXPRESSION);
    sscanf(yytext, "%d", &expr->u.int_value);
    yylval.expression = expr;
    return INT_LITERAL;
}
 /* 浮点数 */
<INITIAL>[0-9]+\.[0-9]+         {
    Expression *expr = crb_alloc_expression(DOUBLE_EXPRESSION);
    sscanf(yytext, "%lf", &expr->u.double_value);
    yylval.expression = expr;
    return DOUBLE_LITERAL;
}
 /* 开始扫描并存储字符串 */
<INITIAL>"\""                   {
    crb_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}
 /* 开始扫描并忽略注释 */
<INITIAL>#                      { BEGIN COMMENT; }
<INITIAL>[ \t]                  ;
 /* 遇到换行符则增加行号 */
<INITIAL>\n                     increment_line_number();
<INITIAL>.                      {
    fprintf(stderr, "Unexpected char(%s)", yytext);
    exit(1);
}
<COMMENT>\n                     {
    increment_line_number();
    BEGIN INITIAL;
}
<COMMENT>.                      ;
<STRING_LITERAL_STATE>\"        {
    Expression *expr = crb_alloc_expression(STRING_EXPRESSION);
    expr->u.string_value = crb_close_string_literal();
    yylval.expression = expr;
    BEGIN INITIAL;
    return STRING_LITERAL;
}
<STRING_LITERAL_STATE>\n        {
    crb_add_string_literal('\n');
    increment_line_number();
}
<STRING_LITERAL_STATE>\\\"      crb_add_string_literal('"');
<STRING_LITERAL_STATE>\\n       crb_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t       crb_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\      crb_add_string_literal('\\');
<STRING_LITERAL_STATE>.         crb_add_string_literal(yytext[0]);
%%

