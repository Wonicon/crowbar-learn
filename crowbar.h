#ifndef CROWBAR_H
#define CROWBAR_H

#include <stdio.h>
#include "MEM.h"
#include "CRB.h"
#include "CRB_dev.h"

#define LINE_BUF_SIZE 1024
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
typedef struct Elsif_tag              Elsif;

// 解释器
struct CRB_Interpreter_tag {
    MEM_Storage         interpreter_storage;
    MEM_Storage         execute_storage;
    Variable           *variable;
    FunctionDefinition *function_list;
    StatementList      *statement_list;
    int                 current_line_number;
};

/**
 * 拷贝源代码中的字符串和符号的相关函数
 */

// 初始化拷贝状态, 开始拷贝字符串字面量, 在词法分析遇到字符串常量时调用
void crb_open_string_literal();

// 在拷贝字符串字面量过程中调用, 添加字符
void crb_add_string_literal(char ch);

// 结束拷贝字符串字面量, 返回该字符串字面量的拷贝, 由用户负责释放
char *crb_close_string_literal();

// 释放拷贝字符串字面量时使用的动态资源, 初始化拷贝状态.
void crb_reset_string_literal();

// 拷贝符号, 由用户负责释放
char *crb_create_identifier(const char *id);


/**
 * 语法树相关的数据类型定义
 */

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

// 赋值表达式
typedef struct {
    const char *variable;
    Expression *operand;
} AssignExpression;

// 二元表达式
typedef struct {
    Expression *left;
    Expression *right;
} BinaryExpression;

// 函数调用表达式
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
        CRB_Boolean            boolean_value;
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
// global 是唯一的一例, identifier_list 存储的是用逗号分割的标识符链表.
typedef struct {
    IdentifierList *identifier_list;
} GlobalStatement;

// 标识符链表
struct IdentifierList_tag {
    const char     *name;
    IdentifierList *next;
};

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

// elsif 链表结点
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

// 函数类型标签
typedef enum {
    CROWBAR_FUNCTION_DEFINITION,  // crowbar 代码定义的函数
    NATIVE_FUNCTION_DEFINITION,   // crowbar 解释器内置的函数
} FunctionDefinitionType;

// 函数, 链表结构
// TODO 添加对内置函数的支持
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


/**
 * 下面是构造语法树所需要的函数, 主要在 yacc 文件中调用
 */

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


/**
 * 与解释器指针直接相关的一些函数
 */

// 获取当前解释器
CRB_Interpreter *crb_get_current_interpreter();

// 设置当前解释器
void crb_set_current_interpreter(CRB_Interpreter *interpreter);

// 向解释器存储器申请空间, 主要用来分配语法树内容, 注意与运行时存储器的区别
void *crb_malloc(size_t size);

// 向运行时存储器申请空间, 主要用来分配变量
void *crb_execute_malloc(CRB_Interpreter *interpreter, size_t size);

// 搜索函数 <name>, 没找到时返回 NULL
FunctionDefinition *crb_search_function(const char *name);

// 从顶层作用域搜索全局变量
Variable *crb_search_global(CRB_Interpreter *interpreter, const char *name);


/**
 * 与内置函数和变量有关的函数
 */

void crb_add_std_fp(CRB_Interpreter *interpreter);


/**
 * 与解释执行有关的函数
 */

// 语句返回值类型标签, 主要是为了识别中断执行流程的语句
typedef enum {
    INVALID_STATEMENT_RESULT,
    NORMAL_STATEMENT_RESULT,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_COUNT
} StatementResultType;

// 语句结果, 传递执行状态以及可能的返回值
typedef struct {
    StatementResultType type;
    union {
        CRB_Value return_value;
    } u;
} StatementResult;

// 函数作用域可能不拥有所有的全局变量, 需要新的容器来维护自身引用的全局变量
// 变量本身虽然有链表结构, 但是对全局变量而言, 那个链表是全局的.
// 不能修改全局变量的 Variable->next 来构造新的的符号表.
// 所以需要使用额外的链表结构.
// 另外局部变量在本地定义生成, 所以直接使用 Variable 链表并采用头部插入的方法
// 即可达到作用域覆盖, 并且不会影响到上层作用域的 Variable 链表

typedef struct GlobalVariableRef_tag GlobalVariableRef;
struct GlobalVariableRef_tag {
    Variable          *variable;
    GlobalVariableRef *next;
};

// 相当于符号表的存在
typedef struct {
    Variable          *variable;         // 局部变量
    GlobalVariableRef *global_variable;  // 全局变量
} LocalEnvironment;

// 遍历执行语句块的语句
StatementResult crb_execute_statement_list(CRB_Interpreter  *interpreter,
                                           LocalEnvironment *env,
                                           StatementList    *list);

// 表达式求值
CRB_Value crb_eval_expression(CRB_Interpreter  *interpreter,
                              LocalEnvironment *env,
                              Expression       *expr);

// 构造字面字符串变量, C字符串不会被释放
CRB_String *crb_literal_to_crb_string(char *str);

// 构造非字面字符串变量, C字符串在引用计数为0时同字符串变量一同释放
CRB_String *crb_create_crb_string(char *str);

// 增加字符串变量的引用计数
void crb_refer_string(CRB_String *str);

// 减少字符串变量的引用计数, 如果引用计数为0, 释放变量
void crb_release_string(CRB_String *str);

#endif // CROWBAR_H
