#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define MARK_SIZE (4)


union Header_tag {
    struct {
        int size;  // The allocated size
        const char *filename;
        int line;
        Header *prev;
        Header *next;
        uint8_t mark[MARK_SIZE];  // Check
    };
};


#define MARK 0xCD


static struct MEM_Controller_tag st_default_controller;
MEM_Controller mem_default_controller = &st_default_controller;


// MEM_create_controller:
//   malloc a new controller under the default memory controller.
MEM_Controller
MEM_create_controller()
{
    MEM_Controller p;
    p = MEM_malloc_func(&st_default_controller, __FILE__, __LINE__, sizeof(*p));
    *p = st_default_controller;
    return p;
}


#ifdef DEBUG
// chain_block: add a new block node in the head.
static void
chain_block(MEM_Controller controller, Header *new_header)
{
    if (controller->block_header != NULL) {
        controller->block_header->prev = new_header;
    }
    new_header->prev = NULL;
    new_header->next = controller->block_header;
    controller->block_header = new_header;
}


// rechain_block: used by realloc
static void
rechain_block(MEM_Controller controller, Header *header)
{
    if (header->prev != NULL) {
        header->prev->next = header;
    }
    else {
        controller->block_header = header;
    }
    if (header->next != NULL) {
        header->next->prev = header;
    }
}


// unchain_block: delete a block
static void
unchain_block(MEM_Controller controller, Header *header)
{
    if (header->prev != NULL) {
        header->prev->next = header->next;
    }
    else {
        controller->block_header = header->next;
    }
    if (header->next != NULL) {
        header->next->prev = header->prev;
    }
}


// set_header: initialize the memory block header.
void
set_header(Header *header, int size, const char *filename, int line)
{
    header->size = size;
    header->filename = filename;
    header->line = line;
    memset(header->mark, MARK, (uint8_t *)(header + 1) - (uint8_t *)(header->mark));
}


// set_tail: mark the tail with 0xCD
void
set_tail(void *ptr, int alloc_size)
{
    uint8_t *tail = ((uint8_t *)ptr) + alloc_size - MARK_SIZE;
    memset(tail, MARK, MARK_SIZE);
}


// check_mark_sub: sub procedure of check_mark
void
check_mark_sub(unsigned char *mark, int size)
{
    for (int i = 0; i < size; i++) {
        if (mark[i] != MARK) {
            fprintf(stderr, "bad mark\n");
            abort();
        }
    }
}


void
check_mark(Header *header)
{
    // Check header mark.
    // Since mark is the last field of Header,
    // (char *)&header[1] - (char *)header->mark can cover the mark and aligned bytes.
    check_mark_sub(header->mark, (char *)&header[1] - (char *)header->mark);

    // Check tail mark.
    unsigned char *tail = ((unsigned char *)header) + header->size + sizeof(Header);
    check_mark_sub(tail, MARK_SIZE);
}
#endif // DEBUG


// MEM_malloc_func: the wrapper of the standard malloc function.
void *
MEM_malloc_func(MEM_Controller controller, const char *filename, int line, size_t size)
{
#ifdef DEBUG
    size_t alloc_size = sizeof(Header) + size + MARK_SIZE;
#else
    size_t alloc_size = size;
#endif

    void *ptr = malloc(alloc_size);

    if (ptr == NULL) {
        fprintf(stderr, "malloc error\n");
        exit(1);
    }

#ifdef DEBUG
    memset(ptr, 0xCC, alloc_size);  // Avoid uninitialization by caller.
    set_header(ptr, size, filename, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, ptr);
    ptr = (uint8_t *)ptr + sizeof(Header);  // Jump the header.
#endif

    return ptr;
}

