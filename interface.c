#include "crowbar.h"
#include <stdlib.h>

/**
 * 添加默认内置函数
 * <del>按需添加</del>
 */
static void
add_default_native_functions(CRB_Interpreter *interpreter)
{
    CRB_add_native_function(interpreter, "print", crb_native_print);
}

CRB_Interpreter *
CRB_create_interpreter()
{
    MEM_Storage storage = MEM_open_storage(0);
    CRB_Interpreter *interpreter = MEM_storage_malloc(
            storage, sizeof(CRB_Interpreter));
    interpreter->interpreter_storage = storage;
    interpreter->execute_storage = NULL;
    interpreter->variable = NULL;
    interpreter->function_list = NULL;
    interpreter->statement_list = NULL;
    interpreter->current_line_number = 1;

    crb_set_current_interpreter(interpreter);
    return interpreter;
}

void
CRB_compile(CRB_Interpreter *interpreter, FILE *fp)
{
    extern int yyparse();
    extern FILE *yyin;

    crb_set_current_interpreter(interpreter);
    yyin = fp;
    if (yyparse()) {
        exit(1);
    }
    crb_reset_string_literal();
}

void
CRB_interpret(CRB_Interpreter *interpreter)
{
    interpreter->execute_storage = MEM_open_storage(0);
    crb_add_std_fp(interpreter);
    add_default_native_functions(interpreter);
    crb_execute_statement_list(interpreter, NULL, interpreter->statement_list);
}

void
CRB_add_native_function(CRB_Interpreter        *interpreter,
                        const char             *name,
                        CRB_NativeFunctionProc  proc)
{
    // 分配函数定义空间
    FunctionDefinition *fd = crb_malloc(sizeof(FunctionDefinition));
    // 初始化
    fd->name = name;
    fd->type = NATIVE_FUNCTION_DEFINITION;
    fd->u.native_f.proc = proc;
    // 插入解释器函数定义链表
    fd->next = interpreter->function_list;
    interpreter->function_list = fd;
}
