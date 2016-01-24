#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include "MEM.h"

typedef union Header_tag Header;

struct MEM_Controller_tag {
    Header *block_header;
};

#endif // MEMORY_MEMORY_H
