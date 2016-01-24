#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    extern int yyparse();
    extern FILE *yyin;

#ifdef DEBUG
    const char *file = "test/test.crb";
#else
    const char *file = argv[1];
#endif // DEBUG

    yyin = fopen(file, "r");
    if (yyin == NULL) {
        fprintf(stderr, "source file %s not found!\n", file);
    }
    yyparse();
    return 0;
}
