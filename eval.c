#include "crowbar.h"
#include "DBG.h"
#include "CRB_dev.h"
#include <string.h>

static CRB_Value eval_expression(CRB_Interpreter  *interpreter,
                                 LocalEnvironment *env,
                                 Expression       *expr);

static CRB_Boolean eval_binary_null(ExpressionType type, CRB_Value *left, CRB_Value *right);

static CRB_Value eval_int_expression(int int_value)
{
    CRB_Value v = {
        .type = CRB_INT_VALUE,
        .u.int_value = int_value,
    };
    return v;
}

static CRB_Value eval_double_expression(double doulbe_value)
{
    CRB_Value v = {
        .type = CRB_DOUBLE_VALUE,
        .u.double_value = doulbe_value,
    };
    return v;
}

static CRB_Value eval_string_expression(char *string_value)
{
    CRB_Value v = {
        .type = CRB_STRING_VALUE,
        .u.string_value = crb_literal_to_crb_string(string_value),
    };
    return v;
}

static CRB_Value eval_boolean_expression(CRB_Boolean boolean_value)
{
    CRB_Value v = {
        .type = CRB_BOOLEAN_VALUE,
        .u.boolean_value = boolean_value,
    };
    return v;
}

void refer_if_string(CRB_Value *value)
{
    if (value->type == CRB_STRING_VALUE) {
        crb_refer_string(value->u.string_value);
    }
}

void release_if_string(CRB_Value *value)
{
    if (value->type == CRB_STRING_VALUE) {
        crb_release_string(value->u.string_value);
    }
}

static Variable *search_local_variable_from_env(LocalEnvironment *env,
                                                const char       *name)
{
    Variable *curr;
    if (env == NULL) {
        curr = NULL;
    }
    else {
        for (curr = env->variable; curr != NULL; curr = curr->next) {
            if (!strcmp(curr->name, name)) {
                break;
            }
        }
    }
    return curr;
}

static Variable *search_global_variable_from_env(CRB_Interpreter  *interpreter,
                                                 LocalEnvironment *env,
                                                 const char       *name)
{
    Variable *result = NULL;

    if (env == NULL) {
        result = crb_search_global(interpreter, name);
    }
    else {
        GlobalVariableRef *pos;
        for (pos = env->global_variable; pos != NULL; pos = pos->next) {
            if (!strcmp(name, pos->variable->name)) {
                result = pos->variable;
                break;
            }
        }
    }

    return result;
}

void crb_add_local_variable(LocalEnvironment *env, const char *identifier, CRB_Value *value)
{
    Variable *new_var = MEM_malloc(sizeof(Variable));
    new_var->name = identifier;
    new_var->value = *value;
    new_var->next = env->variable;
    env->variable = new_var;
}

static CRB_Value eval_identifier_expression(CRB_Interpreter  *interpreter,
                                            LocalEnvironment *env,
                                            Expression       *expr)
{
    CRB_Value value;
    Variable *variable = search_local_variable_from_env(env, expr->u.identifier);
    if (variable != NULL) {
        value = variable->value;
    }
    else {
        variable = search_global_variable_from_env(interpreter, env, expr->u.identifier);
        if (variable != NULL) {
            value = variable->value;
        }
        else {
            DBG_panic("%s undefined!", expr->u.identifier);
        }
    }
    refer_if_string(&value);
    return value;
}

static CRB_Value eval_assign_expression(CRB_Interpreter  *interpreter,
                                        LocalEnvironment *env,
                                        const char       *identifier,
                                        Expression       *expr)
{
    CRB_Value value = eval_expression(interpreter, env, expr);
    // 使用 Elvis 操作符 ?: 简化回滚写法
    Variable *left = search_local_variable_from_env(env, identifier) ?:
                     search_global_variable_from_env(interpreter, env, identifier);
    if (left != NULL) {
        release_if_string(&left->value);
        left->value = value;
    }
    else {
        if (env == NULL) {
            CRB_add_global_variable(interpreter, identifier, &value);
        }
        else {
            crb_add_local_variable(env, identifier, &value);
        }
    }
    refer_if_string(&value);
    return value;
}

static CRB_Value eval_null_expression()
{
    CRB_Value value = {
        .type = CRB_NULL_VALUE
    };
    return value;
}

static inline int
is_math_operator(ExpressionType type)
{
    return type == ADD_EXPRESSION || type == SUB_EXPRESSION || type == MUL_EXPRESSION || type == DIV_EXPRESSION || type == MOD_EXPRESSION;
}

static inline int
is_compare_operator(ExpressionType type)
{
    return type == EQ_EXPRESSION || type == NE_EXPRESSION || type == GT_EXPRESSION || type == GE_EXPRESSION || type == LE_EXPRESSION || type == LT_EXPRESSION;
}

/**
 * 计算整型二元表达式
 */
static void
eval_binary_int(ExpressionType   operator,
                int              left,
                int              right,
                CRB_Value       *result)
{
    if (is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    }
    else if (is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    }
    else {
        DBG_panic("Unexpected operator type");
    }

    switch (operator) {
        case ADD_EXPRESSION:
            result->u.int_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.int_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.int_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.int_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.int_value = left % right;
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = (left == right) ? CRB_TRUE : CRB_FALSE;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = (left != right) ? CRB_TRUE : CRB_FALSE;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = (left <= right) ? CRB_TRUE : CRB_FALSE;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = (left < right) ? CRB_TRUE : CRB_FALSE;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = (left >= right) ? CRB_TRUE : CRB_FALSE;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = (left > right) ? CRB_TRUE : CRB_FALSE;
        default:
            DBG_panic("bad case");
    }
}

/**
 * 计算浮点型二元表达式
 */
static void
eval_binary_double(ExpressionType operator,
                   double         left,
                   double         right,
                   CRB_Value     *result)
{
    if (is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    }
    else if (is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    }
    else {
        DBG_panic("Unexpected operator type");
    }

    switch (operator) {
        case ADD_EXPRESSION:
            result->u.double_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.double_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.double_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.double_value = left / right;
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = (left == right) ? CRB_TRUE : CRB_FALSE;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = (left != right) ? CRB_TRUE : CRB_FALSE;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = (left <= right) ? CRB_TRUE : CRB_FALSE;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = (left < right) ? CRB_TRUE : CRB_FALSE;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = (left >= right) ? CRB_TRUE : CRB_FALSE;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = (left > right) ? CRB_TRUE : CRB_FALSE;
        default:
            DBG_panic("bad case");
    }
}

/**
 * 计算布尔型二元表达式
 */
static void
eval_binary_boolean(ExpressionType type, CRB_Boolean left, CRB_Boolean right, CRB_Value *result)
{
    result->type = CRB_BOOLEAN_VALUE;
    if (type == EQ_EXPRESSION) {
        result->u.boolean_value = (left == right) ? CRB_TRUE : CRB_FALSE;
    }
    else if (type == NE_EXPRESSION) {
        result->u.boolean_value = (left != right) ? CRB_TRUE : CRB_FALSE;
    }
}

/**
 * 链接两个字符串，并自动扩容
 */
CRB_String *
chain_string(CRB_String *left, CRB_String *right)
{
    size_t left_len = strlen(left->string);
    size_t right_len = strlen(right->string);

    size_t len = left_len + right_len;
    char *str = MEM_malloc(len + 1);
    strcpy(str, left->string);
    strcpy(str + left_len, right->string);
    CRB_String *ret = crb_create_crb_string(str);
    crb_release_string(left);
    crb_release_string(right);

    return ret;
}

/**
 * 根据表达式类型比较字符串并减少临时变量的引用计数
 */
static CRB_Boolean
eval_compare_string(ExpressionType  type,
                    CRB_Value      *left,
                    CRB_Value      *right)
{
    int cmp = strcmp(left->u.string_value->string, right->u.string_value->string);

    CRB_Boolean result = CRB_FALSE;
    switch (type) {
        case EQ_EXPRESSION:
            result = cmp == 0 ? CRB_TRUE : CRB_FALSE;
            break;
        case NE_EXPRESSION:
            result = cmp != 0 ? CRB_TRUE : CRB_FALSE;
            break;
        case LE_EXPRESSION:
            result = cmp <= 0 ? CRB_TRUE : CRB_FALSE;
            break;
        case LT_EXPRESSION:
            result = cmp < 0 ? CRB_TRUE : CRB_FALSE;
            break;
        case GE_EXPRESSION:
            result = cmp >= 0 ? CRB_TRUE : CRB_FALSE;
            break;
        case GT_EXPRESSION:
            result = cmp > 0 ? CRB_TRUE : CRB_FALSE;
            break;
        default:
            DBG_panic("Unexpected type");
    }

    crb_release_string(left->u.string_value);
    crb_release_string(right->u.string_value);

    return result;
}

/**
 * 计算二元表达式
 */
static CRB_Value
crb_eval_binary_expression(CRB_Interpreter  *interpreter,
                           LocalEnvironment *env,
                           ExpressionType    type,
                           Expression       *left,
                           Expression       *right)
{
    CRB_Value left_val;
    CRB_Value right_val;
    CRB_Value result = {};

    left_val = eval_expression(interpreter, env, left);
    right_val = eval_expression(interpreter, env, right);

    /**
     * 根据不同的类型使用不同的函数
     */
    if (left_val.type == CRB_INT_VALUE && right_val.type == CRB_INT_VALUE) {
        eval_binary_int(type, left_val.u.int_value, right_val.u.int_value, &result);
    }
    else if (left_val.type == CRB_DOUBLE_VALUE && right_val.type == CRB_DOUBLE_VALUE) {
        eval_binary_double(type, left_val.u.double_value, right_val.u.double_value, &result);
    }
    else if (left_val.type == CRB_INT_VALUE && right_val.type == CRB_DOUBLE_VALUE) {
        left_val.u.double_value = left_val.u.int_value;
        eval_binary_double(type, left_val.u.double_value, right_val.u.double_value, &result);
    }
    else if (left_val.type == CRB_DOUBLE_VALUE && right_val.type == CRB_INT_VALUE) {
        right_val.u.double_value = right_val.u.int_value;
        eval_binary_double(type, left_val.u.double_value, right_val.u.double_value, &result);
    }
    else if (left_val.type == CRB_BOOLEAN_VALUE && right_val.type == CRB_BOOLEAN_VALUE) {
        eval_binary_boolean(type, left_val.u.boolean_value, right_val.u.boolean_value, &result);
    }
    else if (left_val.type == CRB_STRING_VALUE && type == ADD_EXPRESSION) {
        char buf[LINE_BUF_SIZE];
        CRB_String *right_str;

        if (right_val.type == CRB_INT_VALUE) {
            sprintf(buf, "%d", right_val.u.int_value);
            right_str = crb_create_crb_string(MEM_strdup(buf));
        }
        else if (right_val.type == CRB_DOUBLE_VALUE) {
            sprintf(buf, "%f", right_val.u.double_value);
            right_str = crb_create_crb_string(MEM_strdup(buf));
        }
        else if (right_val.type == CRB_BOOLEAN_VALUE) {
            right_str = crb_create_crb_string(MEM_strdup(right_val.u.boolean_value == CRB_TRUE ? "true" : "false"));
        }
        else if (right_val.type == CRB_STRING_VALUE) {
            right_str = right_val.u.string_value;
        }
        else if (right_val.type == CRB_NATIVE_POINTER_VALUE) {
            sprintf(buf, "(%s:%p)", right_val.u.native_pointer.info->name,
                    right_val.u.native_pointer.pointer);
            right_str = crb_create_crb_string(MEM_strdup(buf));
        }
        else if (right_val.type == CRB_NULL_VALUE) {
            right_str = crb_create_crb_string(MEM_strdup("null"));
        }
        else {
            right_str = NULL;
        }

        result.type = CRB_STRING_VALUE;
        result.u.string_value = chain_string(left_val.u.string_value, right_str);
    }
    else if (left_val.type == CRB_STRING_VALUE && right_val.type == CRB_STRING_VALUE && is_compare_operator(type)) {
        result.type = CRB_STRING_VALUE;
        result.u.boolean_value = eval_compare_string(type, &left_val, &right_val);
    }
    else if ((left_val.type == CRB_NULL_VALUE || right_val.type == CRB_NULL_VALUE) && is_compare_operator(type)) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_null(type, &left_val, &right_val);
    }
    return result;
}

static CRB_Boolean
eval_binary_null(ExpressionType type,
                 CRB_Value     *left,
                 CRB_Value     *right)
{
    CRB_Boolean result = CRB_FALSE;

    if (type == EQ_EXPRESSION) {
        result = (left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE) ?
                 CRB_TRUE : CRB_FALSE;
    }
    else if (type == NE_EXPRESSION) {
        result = !(left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE) ?
                 CRB_TRUE : CRB_FALSE;
    }
    else {
        DBG_panic("Unexpected type");
    }

    release_if_string(left);
    release_if_string(right);

    return result;
}

/**
 * 计算表达式，实现短路求值
 */
static CRB_Value
crb_eval_logic_and_or_expression(CRB_Interpreter *interpreter,
                                 LocalEnvironment *env,
                                 ExpressionType   type,
                                 Expression      *left,
                                 Expression      *right)
{
    CRB_Value result = { .type = CRB_BOOLEAN_VALUE };

    CRB_Value left_val = eval_expression(interpreter, env, left);

    DBG_assert(left_val.type == CRB_BOOLEAN_VALUE, "Unexpected value");

    if (type == LOGICAL_AND_EXPRESSION && left_val.u.boolean_value == CRB_FALSE) {
        result.u.boolean_value = CRB_FALSE;
    }
    else if (type == LOGICAL_OR_EXPRESSION && left_val.u.boolean_value == CRB_TRUE) {
        result.u.boolean_value = CRB_TRUE;
    }
    else {
        CRB_Value right_val = eval_expression(interpreter, env, right);
        DBG_assert(right_val.type == CRB_BOOLEAN_VALUE, "Unexpected value");

        /**
         * 进入这个分支，即 AND 的一个元素为 true，或者 OR 的一个元素为 false
         * 这个时候最后取值确实是与第二个元素的值保持一致的。
         */
        result.u.boolean_value = right_val.u.boolean_value;
    }

    return result;
}

#if 0
static CRB_Value crb_eval_minus_expression(CRB_Interpreter  *interpreter,
                                           LocalEnvironment * env,
                                           Expression       *expr);
static CRB_Value eval_function_call_expression(CRB_Interpreter  *interpreter,
                                               LocalEnvironment *env,
                                               Expression       *expr);
#endif
static CRB_Value eval_expression(CRB_Interpreter  *interpreter,
                                 LocalEnvironment *env,
                                 Expression       *expr)
{
    CRB_Value value = {
        .type = CRB_BOOLEAN_VALUE,
        .u.boolean_value = CRB_FALSE,
    };
    switch (expr->type) {
        case INT_EXPRESSION:
            value = eval_int_expression(expr->u.int_value);
            break;
        case NULL_EXPRESSION:
            value = eval_null_expression();
            break;
        case DOUBLE_EXPRESSION:
            value = eval_double_expression(expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            value = eval_string_expression(expr->u.string_value);
            DBG_debug_write(0, "string: %s\n", value.u.string_value->string);
            break;
        case BOOLEAN_EXPRESSION:
            value = eval_boolean_expression(expr->u.boolean_value);
            break;
        case IDENTIFIER_EXPRESSION:
            value = eval_identifier_expression(interpreter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            value = eval_assign_expression(interpreter, env,
                                           expr->u.assign_expression.variable,
                                           expr->u.assign_expression.operand);
            break;
        case ADD_EXPRESSION:
        case SUB_EXPRESSION:
        case MUL_EXPRESSION:
        case DIV_EXPRESSION:
        case MOD_EXPRESSION:
        case EQ_EXPRESSION:
        case NE_EXPRESSION:
        case GT_EXPRESSION:
        case GE_EXPRESSION:
        case LT_EXPRESSION:
        case LE_EXPRESSION:
            crb_eval_binary_expression(interpreter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
            crb_eval_logic_and_or_expression(interpreter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            break;
        case FUNCTION_CALL_EXPRESSION:
            break;
        default:
            DBG_panic("Invalid expression!\n");
    }
    return value;
}

CRB_Value crb_eval_expression(CRB_Interpreter  *interpreter,
                              LocalEnvironment *env,
                              Expression       *expr)
{
    return eval_expression(interpreter, env, expr);
}