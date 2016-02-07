// 定义执行语法树所需要的数据结构和函数接口

#ifndef EXECUTE_H
#define EXECUTE_H

#include "crowbar.h"

// 语句返回值类型标签, 主要是为了识别 break 和 continue
typedef enum {
    INVALID_STATEMENT_RESULT,
    NORMAL_STATEMENT_RESULT,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_COUNT
} StatementResultType;

typedef struct {
    StatementResultType type;
    union {
        CRB_Value return_value;
    } u;
} StatementResult;

// 函数作用域可能不拥有所有的全局变量, 需要新的容器来维护自身引用的全局变量
// 变量本身虽然有链表结构, 但是对全局变量而言, 那个链表是全局的.
// 不能修改全局变量的 Variable->next 来构造新的的符号表.
// 所以出现了下面这种链表结构.
// 另外局部变量在本地定义生成, 所以直接使用 Variable 链表并采用头部插入的方法
// 即可达到作用域覆盖, 并且不会影响到上层作用域的 Variable 链表

typedef struct GlobalVariableRef_tag GlobalVariableRef;
struct GlobalVariableRef_tag {
    Variable          *variable;
    GlobalVariableRef *next;
};

typedef struct {
    Variable          *variable;         // 局部变量
    GlobalVariableRef *global_variable;  // 全局变量
} LocalEnvironment;

StatementResult crb_execute_statement_list(CRB_Interpreter  *interpreter,
                                           LocalEnvironment *env,
                                           StatementList    *list);

#endif // EXECUTE_H
