// util.c
// 主要包含供 crowbar 其它模块使用的共通函数

#include "MEM.h"
#include "crowbar.h"
#include <string.h>

static CRB_Interpreter *st_current_interpreter = NULL;

// Getter 和 Setter...
CRB_Interpreter *
crb_get_current_interpreter()
{
    return st_current_interpreter;
}

void
crb_set_current_interpreter(CRB_Interpreter *inter)
{
    st_current_interpreter = inter;
}

// 用于分配语法树结点, 使用 MEM_storage_malloc 以方便集中释放语法树
void *
crb_malloc(size_t size)
{
    MEM_Storage storage =
        crb_get_current_interpreter()->interpreter_storage;
    void *p = MEM_storage_malloc(storage, size);
    return p;
}

// 用于分配运行时变量内存
void *crb_execute_malloc(CRB_Interpreter *interpreter,
                         size_t           size)
{
    void *p = MEM_storage_malloc(interpreter->execute_storage, size);
    return p;
}

// 遍历解释器的函数定义链表, 按名字找函数定义指针.
// 没有找到的情况下返回 NULL.
FunctionDefinition *
crb_search_function(const char *name)
{
    CRB_Interpreter *interpreter =
        crb_get_current_interpreter();
    FunctionDefinition *curr = interpreter->function_list;
    for (; curr != NULL; curr = curr->next) {
        if (!strcmp(curr->name, name)) {
            break;
        }
    }
    return curr;
}

Variable *crb_search_global(CRB_Interpreter *interpreter,
                            const char      *name)
{
    for (Variable *curr = interpreter->variable; curr != NULL; curr = curr->next) {
        if (!strcmp(curr->name, name)) {
            return curr;
        }
    }
    return NULL;
}

// 注册全局变量
void CRB_add_global_variable(CRB_Interpreter *interpreter,
                             const char      *identifier,
                             CRB_Value       *value)
{
    Variable *new_variable = crb_execute_malloc(interpreter, sizeof(Variable));
    new_variable->name = crb_execute_malloc(interpreter, strlen(identifier) + 1);
    strcpy(new_variable->name, identifier);
    new_variable->value = *value;
    new_variable->next = interpreter->variable;
    interpreter->variable = new_variable;
}