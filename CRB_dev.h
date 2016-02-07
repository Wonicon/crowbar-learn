// crowbar的变量数据类型

#ifndef CRB_DEV_H
#define CRB_DEV_H

#include "CRB.h"

// 值类型标签
typedef enum {
    CRB_INT_VALUE,
    CRB_DOUBLE_VALUE,
    CRB_STRING_VALUE,
    CRB_BOOLEAN_VALUE,
    CRB_NATIVE_POINTER_VALUE,
    CRB_NULL_VALUE,
} CRB_ValueType;

typedef enum {
    CRB_FALSE,
    CRB_TRUE,
} CRB_Boolean;

typedef struct {
    int         ref_count;
    char       *string;
    CRB_Boolean is_literal;
} CRB_String;

// 内置指针信息, 就使用场景来看, 记录了对应的库名
typedef struct {
    const char *name;
} CRB_NativePointerInfo;

typedef struct {
    CRB_NativePointerInfo *info;
    void                  *pointer;
} CRB_NativePointer;

// 值类型
typedef struct {
    CRB_ValueType type;
    union {
        CRB_Boolean       boolean_value;
        int               int_value;
        double            double_value;
        CRB_String       *string_value;
        CRB_NativePointer native_pointer;
    } u;
} CRB_Value;

// 变量
typedef struct Value_tag {
    const char       *name;
    CRB_Value         value;
    struct Value_tag *next;
} Variable;

// 向给定的解释器 interpreter 添加名称为 identifier, 值为 value 的全局变量
// identifier 和 value 采用深拷贝
void CRB_add_global_variable(CRB_Interpreter *interpreter,
                             const char      *identifier,
                             CRB_Value       *value);

#endif // CRB_DEV_H
