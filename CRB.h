#ifndef CRB_H
#define CRB_H

#include <stdio.h>

// CRB_Interpreter is an unimplemented type,
// which could only be used as a handler.
typedef struct CRB_Interpreter_tag CRB_Interpreter;

CRB_Interpreter *CRB_create_interpreter();

#endif // CRB_H
