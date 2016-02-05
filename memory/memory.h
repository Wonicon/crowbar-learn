#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include "MEM.h"

typedef union Header_tag Header;

struct MEM_Controller_tag {
    const char *     error_file;
    MEM_ErrorHandler error_handler;
    MEM_FailMode     fail_mode;
    Header *         block_header;
};

#endif // MEMORY_MEMORY_H
