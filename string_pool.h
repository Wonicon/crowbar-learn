#ifndef STRING_POOL_H
#define STRING_POOL_H

/**
 * crowbar 的字符串运算模块, 采用垃圾回收机制
 */
#include "crowbar.h"

/**
 * 构造字面字符串变量, C字符串不会被释放
 */
CRB_String *crb_literal_to_crb_string(char *str);

/**
 * 构造非字面字符串变量, C字符串在引用计数为0时同字符串变量一同释放
 */
CRB_String *crb_create_crb_string(char *str);

#endif // STRING_POOL_H
