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
default_error_handler(FILE *error_fp, const char *filename, int line, const char *msg)
{
    fprintf(error_fp,
            "MEM:%s failed in %s at %d\n", msg, filename, line);
}


static void
error_handler(MEM_Controller controller, const char *filename, int line, const char *msg)
{
    FILE *error_fp = (controller->error_file == NULL) ? stderr :
        fopen(controller->error_file, "a");

    controller->error_handler(error_fp, filename, line, msg);

    if (controller->fail_mode == MEM_FAIL_AND_EXIT) {
        exit(1);
    }
}


static struct MEM_Controller_tag st_default_controller = {
    NULL,
    default_error_handler,
    MEM_FAIL_AND_EXIT
};
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
        error_handler(controller, filename, line, "malloc");
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
        // Re-alloc
        *(Header *)new_ptr = old_header;
        // Update size field.
        ((Header *)new_ptr)->size = size;

        // Relink to where it used to be linked.
        // When debugging, reallocated memory block may have a new memory area,
        // but the data is reserved, which means that using this reallocated
        // memory header, we can find its preccedence and its successor,
        // but its preccedence and its successor may contain a wrong pointer
        // which points to the old memory block. So a re-chain process is needed.
        // TODO check whether we need to unchain the node and save the content
        // of the header onto stack?
        rechain_block(controller, (Header *)new_ptr);

        set_tail(new_ptr, alloc_size);
    }
    else {
        // Re-alloc acts as malloc
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


// MEM_strdup_func:
//   Private function that does the same work as strdup
//   but would add more information when debugging.
//   As you can see, this function share most of its body
//   with MEM_malloc_func. The reason why we do not implement
//   MEM_strdup_func as a wrapper upon MEM_malloc_func is that
//   the cause string of error_handler needs to be different.
char *
MEM_strdup_func(MEM_Controller controller, const char *filename, int line, const char *str)
{
    int size = strlen(str) + 1;

#ifdef DEBUG
    size_t alloc_size = size + sizeof(Header) + MARK_SIZE;
#else
    size_t alloc_size = size;
#endif

    char *ptr = malloc(alloc_size);
    if (ptr == NULL) {
        error_handler(controller, filename, line, "strdup");
    }

#ifdef DEBUG
    memset(ptr, 0xCC, alloc_size);
    set_header((Header *)ptr, size, filename, line);
    set_tail(ptr, alloc_size);
    chain_block(controller, (Header *)ptr);
    ptr = (char *)ptr + sizeof(Header);  // Jump the header
#endif
    // TODO: check whether strcpy automatically insert '\0'?
    strcpy(ptr, str);

    return ptr;
}


void
MEM_free_func(MEM_Controller controller, void *ptr)
{
    if (ptr == NULL) {
        return;
    }

#ifdef DEBUG
    void *real_ptr = (char *)ptr - sizeof(Header);
    check_mark((Header *)real_ptr);
    int size = ((Header *)real_ptr)->size;
    unchain_block(controller, real_ptr);
    memset(real_ptr, 0xCC, size + sizeof(Header) + MARK_SIZE);
#else
    void *real_ptr = ptr;
#endif

    free(real_ptr);
}


void
MEM_set_error_handler(MEM_Controller controller, MEM_ErrorHandler handler)
{
    controller->error_handler = handler;
}


void
MEM_set_fail_mode(MEM_Controller controller, MEM_FailMode mode)
{
    controller->fail_mode = mode;
}


// MEM_dump_blocks_func:
//   Private function.
//   The soul of this memory module
void
MEM_dump_blocks_func(MEM_Controller controller, FILE *fp)
{
#ifdef DEBUG
    int counter = 0;
    for (Header *pos = controller->block_header; pos != NULL; pos = pos->next) {
        check_mark(pos);
        fprintf(fp, "[%04d]%p:\n", counter, (char *)pos + sizeof(Header));
        fprintf(fp, "allocater %s line %d size %d\n", pos->filename, pos->line, pos->size);
        counter++;
    }
#endif
}


void
MEM_check_block_func(MEM_Controller controller, char *filename, int line, void *p)
{
#ifdef DEBUG
    void *real_ptr = ((char *)p) - sizeof(Header);
    check_mark(real_ptr);
#endif
}


void
MEM_check_all_blocks_func(MEM_Controller controller, const char *filename, int line)
{
#ifdef DEBUG
    for (Header *pos = controller->block_header; pos != NULL; pos = pos->next) {
        check_mark(pos);
    }
#endif
}

