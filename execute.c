#include "crowbar.h"
#include "DBG.h"
#include "CRB_dev.h"
#include <string.h>

/**
 * 统一函数签名与函数名格式
 * 存储类型, 返回值, 参数如 make_exec_helper 所示
 * make_exec_helper(xxx) 最终会生成 execute_xxx_statement 格式的函数名
 */

#define _concat(x, y) x ## y
#define concat(x, y) _concat(x, y)

#define make_exec_helper(name) \
static StatementResult \
concat(concat(execute_, name), _statement) (CRB_Interpreter  *interpreter, \
                                            LocalEnvironment *env, \
                                            Statement        *statement)

make_exec_helper(global)
{
    StatementResult result = { .type = NORMAL_STATEMENT_RESULT };

    if (env == NULL) {
        DBG_panic("env is null");
    }

    // 对逗号分割的标识符列表中所有元素进行遍历
    for (IdentifierList *curr = statement->u.global_s.identifier_list; curr != NULL; curr = curr->next) {
        // 首先判断全局变量是否已经引用
        for (GlobalVariableRef *ref_pos = env->global_variable; ref_pos != NULL; ref_pos = ref_pos->next) {
            if (!strcmp(ref_pos->variable->name, curr->name)) {
                goto NEXT_IDENTIFIER;
            }
        }

        Variable *variable = crb_search_global(interpreter, curr->name);
        if (variable == NULL) {
            DBG_panic("search failed\n");
        }

        GlobalVariableRef *new_ref = crb_execute_malloc(interpreter, sizeof(GlobalVariableRef));
        new_ref->variable = variable;
        new_ref->next = env->global_variable;
        env->global_variable = new_ref;
        NEXT_IDENTIFIER:
            ;
    }

    return result;
}

static StatementResult execute_elsif(CRB_Interpreter  *interpreter,
                                     LocalEnvironment *env,
                                     Elsif            *elsif,
                                     CRB_Boolean      *executed)
{
    StatementResult result = { .type = NORMAL_STATEMENT_RESULT };

    *executed = CRB_FALSE;
    for (Elsif *pos = elsif; pos != NULL; pos = pos->next) {
        CRB_Value condition = crb_eval_expression(interpreter, env, pos->condition);
        DBG_assert(condition.type == CRB_BOOLEAN_VALUE, "Invalid condition type");

        if (condition.u.boolean_value) {
            result = crb_execute_statement_list(interpreter, env, pos->block->statement_list);
            *executed = CRB_TRUE;
            /**
             * 原来这里的代码是这样的:
             * if (result.type != NORMAL_STATEMENT_RESULT) {
             *     goto FUNC_END;
             * }
             * 即只有在中断执行类语句下才结束 elsif 的循环, 照理来说 elsif 不是 fall down 的,
             * 不清楚是否是故意这么实现的?
             */
            break;
        }
    }
    return result;
}

make_exec_helper(if)
{
    StatementResult result;

    CRB_Value condition = crb_eval_expression(interpreter, env, statement->u.if_s.condition);
    DBG_assert(condition.type == CRB_BOOLEAN_VALUE, "Invalid condition type!");

    if (condition.u.boolean_value) {
        result = crb_execute_statement_list(interpreter, env, statement->u.if_s.then_block->statement_list);
    }
    else {
        CRB_Boolean elsif_executed;
        result = execute_elsif(interpreter, env, statement->u.if_s.elsif_list, &elsif_executed);
        if (result.type == NORMAL_STATEMENT_RESULT && !elsif_executed && statement->u.if_s.else_block) {
            result = crb_execute_statement_list(interpreter, env, statement->u.if_s.else_block->statement_list);
        }
    }

    return result;
}

make_exec_helper(while)
{
    StatementResult result = { .type = NORMAL_STATEMENT_RESULT };

    for (;;) {
        CRB_Value condition = crb_eval_expression(interpreter, env, statement->u.while_s.condition);
        DBG_assert(condition.type == CRB_BOOLEAN_VALUE, "Invalid condition type");

        if (condition.u.boolean_value == CRB_FALSE) break;

        result = crb_execute_statement_list(interpreter, env, statement->u.while_s.block->statement_list);

        /**
         * 如果是 return, 则中断 statement 解释循环的效果要一直传递到顶层, 结束函数 block 的执行.
         * 而如果是 break, 则只需要中断本层的循环, 不能影响执行这个 while 语句的 block 循环的执行.
         * 至于 continue, 中断的是上面那个 block 循环.
         */
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        }
        else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }
    }

    return result;
}

make_exec_helper(for)
{
    StatementResult result = { .type = NORMAL_STATEMENT_RESULT };

    if (statement->u.for_s.init != NULL) {
        crb_eval_expression(interpreter, env, statement->u.for_s.init);
    }

    CRB_Value condition;
    for (;;) {
        condition = crb_eval_expression(interpreter, env, statement->u.for_s.condition);
        DBG_assert(condition.type == CRB_BOOLEAN_VALUE, "Invalid condition type");

        if (condition.u.boolean_value == CRB_FALSE) break;

        result = crb_execute_statement_list(interpreter, env, statement->u.for_s.block->statement_list);

        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        }
        else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = NORMAL_STATEMENT_RESULT;
            break;
        }

        if (statement->u.for_s.post != NULL) {
            condition = crb_eval_expression(interpreter, env, statement->u.for_s.post);
        }
    }

    return result;
}

make_exec_helper(break)
{
    StatementResult result = { .type = BREAK_STATEMENT_RESULT };
    return result;
}

make_exec_helper(continue)
{
    StatementResult result = { .type = CONTINUE_STATEMENT_RESULT };
    return  result;
}

make_exec_helper(return)
{
    StatementResult result = { .type = RETURN_STATEMENT_RESULT };
    if (statement->u.return_s.return_value != NULL) {
        result.u.return_value = crb_eval_expression(interpreter, env, statement->u.return_s.return_value);
    }
    else {
        result.u.return_value.type = CRB_NULL_VALUE;
    }
    return result;
}

static StatementResult execute_statement(CRB_Interpreter  *interpreter,
                                         LocalEnvironment *env,
                                         Statement        *statement)
{
    StatementResult result = {
        .type = NORMAL_STATEMENT_RESULT
    };

    switch (statement->type) {
        case EXPRESSION_STATEMENT:
            crb_eval_expression(interpreter, env, statement->u.expression_s);
            break;
        case GLOBAL_STATEMENT:
            result = execute_global_statement(interpreter, env, statement);
            break;
        case IF_STATEMENT:
            result = execute_if_statement(interpreter, env, statement);
            break;
        case WHILE_STATEMENT:
            result = execute_while_statement(interpreter, env, statement);
            break;
        case FOR_STATEMENT:
            result = execute_for_statement(interpreter, env, statement);
            break;
        case BREAK_STATEMENT:
            result = execute_break_statement(interpreter, env, statement);
            break;
        case CONTINUE_STATEMENT:
            result = execute_continue_statement(interpreter, env, statement);
            break;
        case RETURN_STATEMENT:
            result = execute_return_statement(interpreter, env, statement);
            break;
        default:
            DBG_panic("Invalid statement %d", statement->type);
    }
    return result;
}

StatementResult crb_execute_statement_list(CRB_Interpreter  *interpreter,
                                           LocalEnvironment *env,
                                           StatementList    *list)
{
    StatementResult result = {};

    for (StatementList *curr = list; curr != NULL; curr = curr->next) {
        DBG_debug_write(0, "exec statement\n");
        result = execute_statement(interpreter, env, curr->statement);
        if (result.type != NORMAL_STATEMENT_RESULT) {
            break;
        }
    }

    return result;
}
