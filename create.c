// create.c
// 主要包含用来创建语法树的函数

#include "crowbar.h"

Expression *
crb_alloc_expression(ExpressionType type)
{
    Expression *expr = crb_malloc(sizeof(Expression));
    expr->type = type;
    expr->line_number = crb_get_current_interpreter()->current_line_number;
    return expr;
}

