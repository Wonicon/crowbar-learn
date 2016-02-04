#ifndef CROWBAR_H
#define CROWBAR_H

#include <stdio.h>
#include "MEM.h"
#include "CRB.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

// 集中提前 typedef struct 类型,
// 方便写链表的 next 域时简化代码,
// 同时允许灵活安排结构体的定义位置.
// 所有必须带 struct 修饰符的类型名都有 `_tag' 后缀.
typedef struct Expression_tag         Expression;
typedef struct StatementList_tag      StatementList;
typedef struct FunctionDefinition_tag FunctionDefinition;
typedef struct ArgumentList_tag       ArgumentList;
typedef struct ParameterList_tag      ParameterList;
typedef struct IdentifierList_tag     IdentifierList;
typedef struct Elsif                  Elsif;

typedef struct {
    void *TODO;
} Variable;

// 实参链表
struct ArgumentList_tag {
    Expression   *expression;
    ArgumentList *next;
};

// 形参链表
struct ParameterList_tag {
    const char    *name;
    ParameterList *next;
};

// 表达式类型
typedef enum {
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
} ExpressionType;

// 表达式
struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        char  *identifier;
        char  *string_value;
        int    int_value;
        double double_value;
    } u;
};

// 语句块
typedef struct {
    StatementList *statement_list;
} Block;

// 全局变量声明语句
// 由于 crowbar 是无类型语言, 所以没有定义语句,
// global 算是唯一的一例, identifier_list 存储的
// 是用逗号分割的标识符链表.
typedef struct {
    IdentifierList *identifier_list;
} GlobalStatement;

// 条件语句
// 包含了完整的条件语句的 4 个组成部分
// 比较特别的是, elsif 是独立的链表,
// 而不是说一个条件语句本身就是由链表构成的.
typedef struct {
    Expression *condition;
    Block      *then_block;
    Elsif      *elsif_list;
    Block      *else_block;
} IfStatement;

// while 循环语句
typedef struct {
    Expression *condition;
    Block      *block;
} WhileStatement;

// for 循环语句
typedef struct {
    Expression *init;
    Expression *condition;
    Expression *post;  // 循环体 block 结束后的处理
    Block      *block;
} ForStatement;

// 返回语句
typedef struct {
    Expression *return_value;
} ReturnStatement;

// 语句类型标签
typedef enum {
    INVALID_STATEMENT,
    EXPRESSION_STATEMENT,
    GLOBAL_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    STATEMENT_TYPE_COUNT
} StatementType;

// 语句
// 除了表达式是指针外, 其余语句结构直接在本层结点上获取.
typedef struct {
    StatementType type;
    int line_number;
    union {
        Expression      *expression_s;
        GlobalStatement  global_s;
        IfStatement      if_s;
        WhileStatement   while_s;
        ForStatement     for_s;
        ReturnStatement  return_s;
    } u;
} Statement;

// 语句链表
struct StatementList_tag {
    Statement     *statement;
    StatementList *next;
};

struct IdentifierList_tag {
    const char     *name;
    IdentifierList *next;
};

// 函数定义
struct FunctionDefinition_tag {
    void *implement_me;
    struct FunctionDefinition_tag *next;
};

// 解释器
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

// From util.c
CRB_Interpreter *crb_get_current_interpreter();
void *crb_malloc(size_t size);

#endif // CROWBAR_H
