// create.c
// 主要包含用来创建语法树的函数

#include "crowbar.h"
#include <stdlib.h>

void
crb_function_define(const char *identifier, ParameterList *parameter_list, Block *block)
{
    if (crb_search_function(identifier) == NULL) {
        fprintf(stderr, "Redefined of function %s", identifier);
        abort();
    }

    CRB_Interpreter *interpreter =
        crb_get_current_interpreter();

    FunctionDefinition *function =
        crb_malloc(sizeof(FunctionDefinition));
    function->name = identifier;
    function->type = CROWBAR_FUNCTION_DEFINITION;
    function->u.crowbar_f.block = block;
    function->u.crowbar_f.parameter = parameter_list;

    function->next = interpreter->function_list;
    interpreter->function_list = function;
}

ParameterList *
crb_create_parameter(const char *name)
{
    ParameterList *parameter = crb_malloc(sizeof(ParameterList));
    parameter->name = name;
    parameter->next = NULL;
    return parameter;
}

ParameterList *
crb_chain_parameter(ParameterList *list, const char *name)
{
    ParameterList *new_param_node = crb_create_parameter(name);
    if (list == NULL) {
        return new_param_node;
    }
    else {
        ParameterList *curr;
        for (curr = list; curr->next != NULL; curr = curr->next) ;
        curr->next = new_param_node;
        return list;
    }
}

ArgumentList *
crb_create_argument_list(Expression *expression)
{
    ArgumentList *argument = crb_malloc(sizeof(ParameterList));
    argument->expression = expression;
    argument->next = NULL;
    return argument;
}

ArgumentList *
crb_chain_argument_list(ArgumentList *list, Expression *expression)
{
    ArgumentList *new_arg_node = crb_create_argument_list(expression);
    if (list == NULL) {
        return new_arg_node;
    }
    else {
        ArgumentList *curr;
        for (curr = list; curr->next != NULL; curr = curr->next) ;
        curr->next = new_arg_node;
        return list;
    }
}

// 创建 Statement 链表结点, 将 statement 封装
StatementList *
crb_create_statement_list(Statement *statement)
{
    StatementList *list_node =
        crb_malloc(sizeof(StatementList));
    list_node->statement = statement;
    list_node->next = NULL;
    return list_node;
}

// 从尾部增加结点, 每次增加需要遍历整个链表
StatementList *
crb_chain_statement_list(StatementList *list, Statement *statement)
{
    StatementList *tail = crb_create_statement_list(statement);

    if (list == NULL) {
        return tail;
    }
    else {
        StatementList *curr;
        for (curr = list; curr->next != NULL; curr = curr->next) ;
        curr->next = tail;
        return list;
    }
}

Expression *
crb_alloc_expression(ExpressionType type)
{
    Expression *expr = crb_malloc(sizeof(Expression));
    expr->type = type;
    expr->line_number = crb_get_current_interpreter()->current_line_number;
    return expr;
}

Expression *
crb_create_assign_expression(const char *variable, Expression *operand)
{
    Expression *exp = crb_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.variable = variable;
    exp->u.assign_expression.operand = operand;
    return exp;
}

Expression *
crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right)
{
    Expression *exp = crb_alloc_expression(operator);
    exp->u.binary_expression.left = left;
    exp->u.binary_expression.right = right;
    return exp;
}

Expression *
crb_create_minus_expression(Expression *expression)
{
    Expression *exp = crb_alloc_expression(MINUS_EXPRESSION);
    exp->u.minus_expression = expression;
    return exp;
}

Expression *
crb_create_function_call_expression(const char *identifier, ArgumentList *argument)
{
    Expression *expression = crb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    expression->u.function_call_expression.identifier= identifier;
    expression->u.function_call_expression.argument = argument;
    return expression;
}

Expression *
crb_create_identifier_expression(const char *identifier)
{
    Expression *expression = crb_alloc_expression(IDENTIFIER_EXPRESSION);
    expression->u.identifier = identifier;
    return expression;
}

Expression *
crb_create_boolean_expression(CRB_Boolean value)
{
    Expression *expression = crb_alloc_expression(BOOLEAN_EXPRESSION);
    expression->u.boolean_value = value;
    return expression;
}

Expression *
crb_create_null_expression()
{
    Expression *expression = crb_alloc_expression(NULL_EXPRESSION);
    return expression;
}

