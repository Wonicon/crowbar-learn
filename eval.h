// 表达式求值接口

#ifndef LEARN_EVAL_H
#define LEARN_EVAL_H

#include "crowbar.h"
#include "execute.h"

CRB_Value crb_eval_expression(CRB_Interpreter  *interpreter,
                              LocalEnvironment *env,
                              Expression       *expr);

#endif // CROWBAR_LEARN_EVAL_H
