#ifndef CROWBAR_H
#define CROWBAR_H

#include <stdio.h>
#include "MEM.h"
#include "CRB.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

typedef struct {
    void *TODO;
} Variable;

// 表达式类型标签
typedef enum {
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
} ExpressionType;

// 表达式结构体
typedef struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        char  *identifier;
        char  *string_value;
        int    int_value;
        double double_value;
    } u;
} Expression;

// 语句结构体
typedef struct StatementList_tag {
    void *TODO;
    struct StatementList_tag *next;
} StatementList;

// 函数定义结构体
typedef struct FunctionDefinition_tag {
    void *implement_me;
    struct FunctionDefinition_tag *next;
} FunctionDefinition;

// 解释器结构体
struct CRB_Interpreter_tag {
    MEM_Storage         interpreter_storage;
    MEM_Storage         execute_storage;
    Variable           *variable;
    FunctionDefinition *function_list;
    StatementList      *statement_list;
    int                 current_line_number;
};

// 词法分析中用于设置字符串常量的相关 api
void crb_open_string_literal();
void crb_add_string_literal(char ch);
char *crb_close_string_literal();

char *crb_create_identifier(const char *id);

// 表达式结构体构造函数
Expression *crb_alloc_expression(ExpressionType type);

#endif // CROWBAR_H
