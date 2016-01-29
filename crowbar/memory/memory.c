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


static void
error_handler(MEM_Controller controller, char *filename, int line, const char *msg)
{
    if (controller->error_fp == NULL) {
        controller->error_fp = stderr;
    }
    controller->error_handler(controller, filename, line, msg);

    if (controller->fail_mode == MEM_FAIL_AND_EXIT) {
        exit(1);
    }
}


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


void *
MEM_realloc_func(MEM_Controller controller, const char *filename, int line, void *ptr, size_t size)
{
    void *new_ptr;
    size_t alloc_size;
    void *real_ptr;
#ifdef DEBUG
    Header old_header;
    int old_size;

    alloc_size = size + sizeof(Header) + MARK_SIZE;
    if (ptr != NULL) {
        real_ptr = (char *)ptr - sizeof(Header);
        check_mark((Header *)real_ptr);
        old_header = *(Header *)real_ptr;  // Save the header onto stack.
        old_size = old_header.size;
        unchain_block(controller, (Header *)real_ptr);
    }
    else {
        real_ptr = NULL;
        old_size = 0;
    }
#else
    alloc_size = size;
    real_ptr = ptr;
#endif

    new_ptr = realloc(real_ptr, alloc_size);
    if (new_ptr == NULL) {
        if (real_ptr == NULL) {
            error_handler(controller, filename, line, "realloc(malloc)");
        }
        else {
            error_handler(controller, filename, line, "realloc");
        }
    }

#ifdef DEBUG
    if (real_ptr != NULL) {
        // realloc
        *(Header *)new_ptr = old_header;
        ((Header *)new_ptr)->size = size;
        // Relink to where it used to be linked.
        rechain_block(controller, (Header *)new_ptr);
        set_tail(new_ptr, alloc_size);
    }
    else {
        // realloc acts as malloc
        set_header(new_ptr, size, filename, line);
        set_tail(new_ptr, alloc_size);
        chain_block(controller, (Header *)new_ptr);
    }
    new_ptr = (char *)new_ptr + sizeof(Header);
    if (size > old_size) {
        memset((char *)new_ptr + old_size, 0xCC, size - old_size);
    }
#endif

    return new_ptr;
}

