#ifndef PUBLIC_DBG_H_INCLUEDED
#define PUBLIC_DBG_H_INCLUEDED
#include <stdio.h>
#include <stdarg.h>

typedef struct DBG_Controller_tag *DBG_Controller;
void DBG_set(DBG_Controller controller, const char *file, int line);
void DBG_set_expression(const char *expression);

#ifdef DBG_NO_DEBUG

#define DBG_create_controller()
#define DBG_set_debug_level(level)
#define DBG_set_debug_write_fp(fp)
#define DBG_assert(expression, ...)
#define DBG_panic(...)
#define DBG_debug_write(...)

#else  // DBG_NO_DEBUG

#ifdef DBG_CONTROLLER
#define DBG_CURRENT_CONTROLLER DBG_CONTROLLER
#else  // DBG_CONTROLLER
#define DBG_CURRENT_CONTROLLER dgb_default_controller
#endif // DBG_CONTROLLER

extern DBG_Controller DBG_CURRENT_CONTROLLER;

#define DBG_create_controller() (DBG_create_controller_func())
#define DBG_set_debug_level(level) \
    DBG_set_debug_level_func(DBG_CURRENT_CONTROLLER, (level))
#define DBG_set_debug_write_fp(fp) \
    DBG_set_debug_write_fp_func(DBG_CURRENT_CONTROLLER, (fp))
#define DBG_assert(expression, fmt, ...) \
    ((expression) ? (void)0 :\
     (DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__),\
      DBG_set_expression(#expression),\
      DBG_assert_func(fmt, ## __VA_ARGS__)))
#define DBG_panic(fmt, ...) \
    (DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__),\
     DBG_panic_func(fmt, ## __VA_ARGS__))
#define DBG_debug_write(fmt, ...) \
    (DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__),\
     DBG_debug_write_func(fmt, ## __VA_ARGS__))

#endif // DBG_NO_DEBUG

DBG_Controller DBG_create_controller_func();
void DBG_set_debug_level_func(DBG_Controller controller, int level);
void DBG_set_debug_write_fp_func(DBG_Controller controller, FILE *fp);
void DBG_assert_func(const char *fmt, ...);
void DBG_panic_func(const char *fmt, ...);
void DBG_debug_write_func(int level, const char *fmt, ...);

#endif  // PUBLIC_DBG_H_INCLUEDED
