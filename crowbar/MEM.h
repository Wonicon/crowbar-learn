#ifndef MEM_H
#define MEM_H

#include <stdio.h>

typedef struct MEM_Controller_tag *MEM_Controller;

extern MEM_Controller mem_default_controller;

// MEM_CONTROLLER is the customiszing controller.
#ifdef MEM_CONTROLLER
#define CURRENT_MEM_CONTROLLER MEM_CONTROLLER
#else
#define CURRENT_MEM_CONTROLLER mem_default_controller
#endif // MEM_CONTROLLER

MEM_Controller
MEM_create_controller();

void *
MEM_malloc_func(MEM_Controller controller, const char *filename, int line, size_t size);

#define MEM_malloc(size) \
    MEM_malloc_func(CURRENT_MEM_CONTROLLER, __FILE__, __LINE__, size)

#endif // MEM_H
