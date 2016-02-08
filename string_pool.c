#include "crowbar.h"
#include "DBG.h"

/**
 * 在存储器以外的空间分配 CRB_String, 字符串采用浅拷贝,
 * 这样在引用计数为 0 时能够将其删除. 如果是分配在存储器中,
 * 则不能即时回收. 但是不分配在存储器中, 则存在泄露的风险.
 */
static CRB_String *alloc_crb_string(char *str, CRB_Boolean is_literal)
{
    CRB_String *ret = MEM_malloc(sizeof(CRB_String));
    ret->ref_count = 0;
    ret->is_literal = is_literal;
    ret->string = str;
    return ret;
}

/**
 * 常量字符串封装
 */
CRB_String *crb_literal_to_crb_string(char *str)
{
    CRB_String *ret = alloc_crb_string(str, CRB_TRUE);
    ret->ref_count = 1;
    return ret;
}

void crb_refer_string(CRB_String *str)
{
    str->ref_count++;
}

/**
 * 如果不是字面量, 则会释放掉字符串.
 * 字面量在词法分析时构建, 为字符串类型表达式语法结点拥有.
 */
void crb_release_string(CRB_String *str)
{
    str->ref_count--;
    DBG_assert(str->ref_count >= 0, "ref count < 0");

    if (str->ref_count == 0) {
        if (str->is_literal == CRB_FALSE) {
            MEM_free(str->string);
        }
        MEM_free(str);
    }
}

/**
 * 对应于 crb_literal_to_crb_string, 这个函数用来构造非字面量 CRB_String
 */
CRB_String *crb_create_crb_string(char *str)
{
    CRB_String *ret = alloc_crb_string(str, CRB_FALSE);
    ret->ref_count = 1;
    return ret;
}