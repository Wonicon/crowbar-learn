#include "MEM.h"
#include "DBG.h"
#include "crowbar.h"
#include <stdlib.h>

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
}

