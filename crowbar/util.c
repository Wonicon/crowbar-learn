// util.c
// 主要包含供 crowbar 其它模块使用的共通函数

#include "MEM.h"
#include "crowbar.h"

static CRB_Interpreter *st_current_interpreter = NULL;

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

// crb_malloc
// 用于分配语法树结点, 使用 MEM_storage_malloc 以方便集中释放语法树
void *
crb_malloc(size_t size)
{
    MEM_Storage storage =
        crb_get_current_interpreter()->interpreter_storage;
    void *p = MEM_storage_malloc(storage, size);
    return p;
}

