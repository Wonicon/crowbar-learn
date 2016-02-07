#ifndef CRB_H
#define CRB_H

#include <stdio.h>

// CRB_Interpreter is an unimplemented type,
// which could only be used as a handler.
typedef struct CRB_Interpreter_tag CRB_Interpreter;

CRB_Interpreter *CRB_create_interpreter();
void CRB_compile(CRB_Interpreter *, FILE *);
void CRB_interpret(CRB_Interpreter *);

#endif // CRB_H
