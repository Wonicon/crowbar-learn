#ifndef CROWBAR_H
#define CROWBAR_H

#include <stdio.h>
#include "MEM.h"
#include "CRB.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

typedef enum {
    CRB_BOOLEN_VALUE,
    CRB_INT_VALUE,
    CRB_DOUBLE_VALUE,
    CRB_NATIVE_POINTER_VALUE,
    CRB_NULL_VALUE,
} CRB_ValueType;

typedef enum {
    CRB_FALSE,
    CRB_TRUE,
} CRB_Boolean;

typedef struct {
    int         ref_coutn;
    char       *string;
    CRB_Boolean is_literal;
} CRB_String;
typedef struct {
    const char *name;
} CRB_NativePointerInfo;

typedef struct {
    CRB_NativePointerInfo *info;
    void                  *pointer;
} CRB_NativePointer;

typedef struct {
    CRB_ValueType type;
    union {
        CRB_Boolean       boolean_type;
        int               int_value;
        double            double_valube;
        CRB_String       *string_value;
        CRB_NativePointer native_pointer;
    } u;
} CRB_Value;

typedef struct {
    void *TODO;
} Variable;


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
typedef struct Elsif_tag              Elsif;

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
    BOOLEAN_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    ASSIGN_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    NULL_EXPRESSION,
    EXPRESSION_TYPE_COUNT,
} ExpressionType;

typedef struct {
    const char *variable;
    Expression *operand;
} AssignExpression;

typedef struct {
    Expression *left;
    Expression *right;
} BinaryExpression;

typedef struct {
    const char   *identifier;
    ArgumentList *argument;
} FunctionCallExpression;

// 表达式
struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        const char            *identifier;
        char                  *string_value;
        int                    int_value;
        int                    boolean_value;
        double                 double_value;
        AssignExpression       assign_expression;
        BinaryExpression       binary_expression;
        Expression            *minus_expression;
        FunctionCallExpression function_call_expression;
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

struct Elsif_tag {
    Expression *condition;
    Block      *block;
    Elsif      *next;
};

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
typedef enum {
    CROWBAR_FUNCTION_DEFINITION,
    NATIVE_FUNCTION_DEFINITION,
} FunctionDefinitionType;

struct FunctionDefinition_tag {
    FunctionDefinitionType type;
    const char            *name;
    FunctionDefinition    *next;
    union {
        struct {
            ParameterList *parameter;
            Block         *block;
        } crowbar_f;
    } u;
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

// From string.c
void crb_open_string_literal();

void crb_add_string_literal(char ch);

char *crb_close_string_literal();

void crb_reset_string_literal();

char *crb_create_identifier(const char *id);

// From create.c
void
crb_function_define(const char *name, ParameterList *parameter_list, Block *block);

StatementList *
crb_create_statement_list(Statement *statement);

StatementList *
crb_chain_statement_list(StatementList *list, Statement *statement);

ParameterList *
crb_create_parameter(const char *name);

ParameterList *
crb_chain_parameter(ParameterList *list, const char *name);

ArgumentList *
crb_create_argument_list(Expression *expression);

ArgumentList *
crb_chain_argument_list(ArgumentList *list, Expression *expression);

Expression *
crb_alloc_expression(ExpressionType type);

Expression *
crb_create_assign_expression(const char *variable, Expression *operand);

Expression *
crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right);

Expression *
crb_create_minus_expression(Expression *expression);

Expression *
crb_create_function_call_expression(const char *identifier, ArgumentList *argument);

Expression *
crb_create_identifier_expression(const char *identifier);

Expression *
crb_create_boolean_expression(CRB_Boolean value);

Expression *
crb_create_null_expression();

IdentifierList *
crb_create_global_identifier(const char *name);

IdentifierList *
crb_chain_identifier(IdentifierList *list, const char *name);

Statement *
crb_create_global_statement(IdentifierList *identifier_list);

Statement *
crb_create_expression_statement(Expression *expression);

Statement *
crb_create_if_statement(Expression *condition, Block *then_block, Elsif *elsif_list, Block *else_block);

Elsif *
crb_create_elsif(Expression *condition, Block *block);

Elsif *
crb_chain_elsif_list(Elsif *list, Elsif *add);

Statement *
crb_create_while_statement(Expression *condition, Block *block);

Statement *
crb_create_for_statement(Expression *init, Expression *condition, Expression *post, Block *block);

Statement *
crb_create_return_statement(Expression *return_value);

Statement *
crb_create_break_statement();

Statement *
crb_create_continue_statement();

Block *
crb_create_block(StatementList *list);

// From util.c
CRB_Interpreter *crb_get_current_interpreter();

void crb_set_current_interpreter(CRB_Interpreter *interpreter);

FunctionDefinition *
crb_search_function(const char *name);

void *crb_malloc(size_t size);

#endif // CROWBAR_H
