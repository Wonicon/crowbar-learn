#include "MEM.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#ifdef DEBUG

// Unit test for MEM.
void ut_mem()
{
    size_t nr_bytes = 0xff;
    uint8_t *bytes = MEM_malloc(nr_bytes);
    for (size_t i = 0; i < nr_bytes; i++) {
        assert(bytes[i] == 0xcc);
    }

    bytes = MEM_malloc(nr_bytes);
    for (size_t i = 0; i < nr_bytes; i++) {
        assert(bytes[i] == 0xcc);
    }

    MEM_dump_blocks(stdout);

    MEM_check_all_blocks();
}
#endif


int main(int argc, char *argv[])
{
#ifdef DEBUG
    ut_mem();
#endif

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
