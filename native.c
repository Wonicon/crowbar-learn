// crowbar 内置函数相关实现

#include "crowbar.h"
#include "CRB_dev.h"
#include "DBG.h"

#define NATIVE_LIB_NAME "crowbar.lang.file"

static CRB_NativePointerInfo st_native_lib_info = {
    NATIVE_LIB_NAME
};

/**
 * 内置打印函数
 * 格式化输出在字符串运算时完成
 */
CRB_Value
crb_native_print(CRB_Interpreter *interpreter,
                 int              argc,
                 CRB_Value       *args)
{
    CRB_Value value = { .type = CRB_NULL_VALUE };

    DBG_assert(argc == 1, "argument miss match");
    
    CRB_Value arg = args[0];
    switch (arg.type) {
        case CRB_BOOLEAN_VALUE:
            if (arg.u.boolean_value == CRB_TRUE) {
                printf("true");
            }
            else {
                printf("false");
            }
            break;
        case CRB_INT_VALUE:
            printf("%d", arg.u.int_value);
            break;
        case CRB_DOUBLE_VALUE:
            printf("%f", arg.u.double_value);
            break;
        case CRB_STRING_VALUE:
            printf("%s", arg.u.string_value->string);
            break;
        case CRB_NATIVE_POINTER_VALUE:
            printf("(%s:%p)", arg.u.native_pointer.info->name, arg.u.native_pointer.pointer);
            break;
        case CRB_NULL_VALUE:
            printf("(null)");
            break;
    }

    return value;
}

void crb_add_std_fp(CRB_Interpreter *interpreter)
{
    CRB_Value fp_value;
    fp_value.type = CRB_NATIVE_POINTER_VALUE;
    fp_value.u.native_pointer.info = &st_native_lib_info;

    fp_value.u.native_pointer.pointer = stdin;
    CRB_add_global_variable(interpreter, "STDIN", &fp_value);

    fp_value.u.native_pointer.pointer = stdout;
    CRB_add_global_variable(interpreter, "STDOUT", &fp_value);

    fp_value.u.native_pointer.pointer = stderr;
    CRB_add_global_variable(interpreter, "STDERR", &fp_value);
}
