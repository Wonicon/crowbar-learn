CC := clang
FLEX := flex
BISON := bison

CFLAGS := -Wall -Werror -Wfatal-errors -I. -ggdb3 -MD -std=gnu11

YFILE := $(wildcard *.y)
LFILE := $(wildcard *.l)
LCFILE := ./lex.yy.c
YCFILE := ./y.tab.c
CFILE := $(filter-out $(LCFILE) $(YCFILE), $(shell find -name "*.c"))
LOBJ := $(LCFILE:%.c=%.o)
YOBJ := $(YCFILE:%.c=%.o)
COBJS := $(CFILE:%.c=%.o)
OBJS := $(COBJS) $(LOBJ) $(YOBJ)

TARGET := crowbar

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)
	@ctags -R * 2> /dev/null

$(YCFILE): $(YFILE)
	$(BISON) --yacc -dv $(YFILE)

$(LCFILE): $(LFILE) $(YCFILE)
	$(FLEX) $(LFILE)

$(LOBJ): $(LCFILE)
	$(CC) -c $(LCFILE)

$(YOBJ): $(YCFILE)
	$(CC) -c $(YCFILE)

.PHONY: clean

clean:
	@rm -f *.yy.* 2> /dev/null
	@rm -f *.tab.* 2> /dev/null
	@rm -f *.output 2> /dev/null
	@rm -f *.o 2> /dev/null
	@rm -f `find -name "*.d"` 2> /dev/null
	@rm -f $(TARGET) 2> /dev/null

