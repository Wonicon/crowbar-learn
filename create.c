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

static Statement *
alloc_statement(StatementType type)
{
    Statement *stmt = crb_malloc(sizeof(Statement));
    stmt->type = type;
    stmt->line_number = crb_get_current_interpreter()->current_line_number;
    return stmt;
}

IdentifierList *
crb_create_global_identifier(const char *identifier)
{
    IdentifierList *list = crb_malloc(sizeof(IdentifierList));
    list->name = identifier;
    list->next = NULL;
    return list;
}

IdentifierList *
crb_create_chain_identifier(IdentifierList *list, const char *identifier)
{
    IdentifierList *new_node = crb_create_global_identifier(identifier);
    IdentifierList *curr;
    for (curr = list; curr->next != NULL; curr = curr->next) ;
    curr->next = new_node;
    return curr;
}

Statement *
crb_create_global_statement(IdentifierList *identifier_list)
{
    Statement *stmt = alloc_statement(GLOBAL_STATEMENT);
    stmt->u.global_s.identifier_list = identifier_list;
    return stmt;
}

Statement *
crb_create_expression_statement(Expression *expression)
{
    Statement *stmt = alloc_statement(EXPRESSION_STATEMENT);
    stmt->u.expression_s = expression;
    return stmt;
}

Statement *
crb_create_if_statement(Expression *condition, Block *then_block, Elsif *elsif_list, Block *else_block)
{
    Statement *stmt = alloc_statement(IF_STATEMENT);
    stmt->u.if_s.condition = condition;
    stmt->u.if_s.then_block = then_block;
    stmt->u.if_s.elsif_list = elsif_list;
    stmt->u.if_s.else_block = else_block;
    return stmt;
}

Elsif *
crb_create_elsif(Expression *condition, Block *block)
{
    Elsif *elsif = crb_malloc(sizeof(Elsif));
    elsif->condition = condition;
    elsif->block = block;
    elsif->next = NULL;
    return elsif;
}

Elsif *
crb_chain_elsif_list(Elsif *list, Elsif *add)
{
    Elsif *curr;
    for (curr = list; curr->next != NULL; curr = curr->next) ;
    curr->next = add;
    return list;
}

Statement *
crb_create_while_statement(Expression *condition, Block *block)
{
    Statement *stmt = alloc_statement(WHILE_STATEMENT);
    stmt->u.while_s.condition = condition;
    stmt->u.while_s.block = block;
    return stmt;
}

Statement *
crb_create_for_statement(Expression *init, Expression *condition, Expression *post, Block *block)
{
    Statement *stmt = alloc_statement(FOR_STATEMENT);
    stmt->u.for_s.init = init;
    stmt->u.for_s.condition = condition;
    stmt->u.for_s.post = post;
    stmt->u.for_s.block = block;
    return stmt;
}

Statement *
crb_create_return_statement(Expression *expression)
{
    Statement *stmt = alloc_statement(RETURN_STATEMENT);
    stmt->u.return_s.return_value = expression;
    return stmt;
}

Statement *
crb_create_break_statement()
{
    Statement *stmt = alloc_statement(BREAK_STATEMENT);
    return stmt;
}

Statement *
crb_create_continue()
{
    Statement *stmt = alloc_statement(CONTINUE_STATEMENT);
    return stmt;
}

Block *
crb_create_block(StatementList *list)
{
    Block *block = crb_malloc(sizeof(Block));
    block->statement_list = list;
    return block;
}

