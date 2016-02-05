#include "MEM.h"
#include "CRB.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    CRB_Interpreter *interpreter = CRB_create_interpreter();
    CRB_compile(interpreter, fopen(argv[1], "r"));
    return 0;
}

