FLEX := flex
BISON := bison

YFILE := $(wildcard *.y)
LFILE := $(wildcard *.l)
LCFILE := lex.yy.c
YCFILE := y.tab.c
CFILE := $(filter-out $(LCFILE) $(YCFILE), $(wildcard *.c))

COBJS := $(CFILE:%.c=%.o)
LOBJ := $(LCFILE:%.c=%.o)
YOBJ := $(YCFILE:%.c=%.o)
OBJS := $(COBJS) $(LOBJ) $(YOBJ)
#OBJS := $(COBJS)

TARGET := crowbar

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

$(YCFILE): $(YFILE)
	$(BISON) --yacc -dv $(YFILE)

$(LCFILE): $(LFILE) $(YCFILE)
	$(FLEX) $(LFILE)

.PHONY: clean

clean:
	@rm -f *.yy.* 2> /dev/null
	@rm -f *.tab.* 2> /dev/null
	@rm -f *.output 2> /dev/null
	@rm -f *.o 2> /dev/null
	@rm -f $(TARGET) 2> /dev/null

