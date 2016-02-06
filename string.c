#include "crowbar.h"
#include <stdio.h>
#include <string.h>

#define STRING_ALLOC_SIZE (256)

static char *st_string_literal_buffer = NULL;
static int st_string_literal_buffer_size = 0;
static int st_string_literal_buffer_alloc_size = 0;

void
crb_open_string_literal()
{
    st_string_literal_buffer_size = 0;
}

void
crb_add_string_literal(char ch)
{
    if (st_string_literal_buffer_size == st_string_literal_buffer_alloc_size) {
        st_string_literal_buffer_alloc_size += STRING_ALLOC_SIZE;
        st_string_literal_buffer = MEM_realloc(st_string_literal_buffer, st_string_literal_buffer_alloc_size);
    }
    st_string_literal_buffer[st_string_literal_buffer_size++] = ch;
}

// 此时字符串空间已经固定了, 所以可以使用利用 Storage 的 crb_malloc 来分配
char *
crb_close_string_literal()
{
    char *new_str = crb_malloc(st_string_literal_buffer_size + 1);
    memcpy(new_str, st_string_literal_buffer, st_string_literal_buffer_size);
    new_str[st_string_literal_buffer_size] = '\0';
    return new_str;
}

// 回收动态分配的字符串缓冲区
void
crb_reset_string_literal()
{
    MEM_free(st_string_literal_buffer);
    st_string_literal_buffer = NULL;
    st_string_literal_buffer_size = 0;
    st_string_literal_buffer_alloc_size = 0;
}

// 用来分配标识符字符串空间, 功能相当与 strdup,
// 但是将分配空间置于 Storage 的管理之下
char *
crb_create_identifier(const char *str)
{
    char *new_str = crb_malloc(strlen(str) + 1);
    strcpy(new_str, str);
    return new_str;
}

