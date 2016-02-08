#include "crowbar.h"
#include "DBG.h"
#include <string.h>

static CRB_Value eval_expression(CRB_Interpreter  *interpreter,
                                 LocalEnvironment *env,
                                 Expression       *expr);

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

/**
 * 注册全局变量一定发生在顶层作用域, 所以不需要 env
 */
void crb_add_global_variable(CRB_Interpreter *interpreter, const char *identifier, CRB_Value *value)
{
    Variable *new_var = MEM_malloc(sizeof(Variable));
    new_var->name = identifier;
    new_var->value = *value;
    new_var->next = interpreter->variable;
    interpreter->variable = new_var;
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
            crb_add_global_variable(interpreter, identifier, &value);
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

#if 0
static CRB_Value crb_eval_binary_expression(CRB_Interpreter *interpreter,
                                            ExpressionType   type,
                                            Expression      *left,
                                            Expression      *right);
static CRB_Value crb_eval_logic_and_or_expression(CRB_Interpreter *interpreter,
                                                  ExpressionType   type,
                                                  Expression      *left,
                                                  Expression      *right);
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
            break;
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:
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