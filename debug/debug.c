#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "MEM.h"
#include "debug.h"

static DBG_Controller st_current_controller;
static const char *st_current_file_name;
static int st_current_line;
static const char *st_assert_expression;

static struct DBG_Controller_tag st_default_controller = {
    NULL,
    INT_MAX,
};

DBG_Controller dbg_default_controller = &st_default_controller;

// DBG_Controller's contructor
DBG_Controller
DBG_create_controller_func()
{
    DBG_Controller controller = MEM_malloc(sizeof(struct DBG_Controller_tag));
    controller->debug_write_fp = stderr;
    controller->current_debug_level = INT_MAX;
    return controller;
}

void
DBG_set_debug_level_func(DBG_Controller controller, int level)
{
    controller->current_debug_level = level;
}

static void
initialize_debug_write_fp()
{
    if (st_default_controller.debug_write_fp == NULL) {
        st_default_controller.debug_write_fp = stderr;
    }
}

static void
assert_func(FILE *log_file, const char *src_file, int line, const char *expr, const char *fmt, va_list args)
{
    fprintf(log_file, "Assertion failure (%s) file: %s line: %d\n", expr, src_file, line);
    if (fmt != NULL) {
        vfprintf(log_file, fmt, args);
    }
}

void
DBG_assert_func(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    initialize_debug_write_fp();
    assert_func(st_current_controller->debug_write_fp, st_current_file_name, st_current_line,
            st_assert_expression, fmt, args);
    if (stderr != st_current_controller->debug_write_fp) {
        assert_func(stderr, st_current_file_name, st_current_line,
                st_assert_expression, fmt, args);
    }
    va_end(args);
    abort();
}

static void
panic_func(FILE *fp, const char *file, int line, const char *fmt, va_list args)
{
    fprintf(fp, "Panic! file: %s line:%d\n", file, line);
    if (fmt != NULL) {
        vfprintf(fp, fmt, args);
    }
}

void
DBG_panic_func(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    initialize_debug_write_fp();
    panic_func(st_current_controller->debug_write_fp,
            st_current_file_name, st_current_line, fmt, args);
    if (stderr != st_current_controller->debug_write_fp) {
        panic_func(stderr, st_current_file_name, st_current_line, fmt, args);
    }
}

void
DBG_debug_write_func(int level, const char *fmt, ...)
{
    if (level > 0 && level > st_current_controller->current_debug_level) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    initialize_debug_write_fp();
    if (fmt != NULL) {
        vfprintf(st_current_controller->debug_write_fp, fmt, args);
    }
    va_end(args);
}

void
DBG_set(DBG_Controller controller, const char *file, int line)
{
    st_current_controller = controller;
    st_current_file_name = file;
    st_current_line = line;
}

void
DBG_set_expression(const char *expression)
{
    st_assert_expression = expression;
}
