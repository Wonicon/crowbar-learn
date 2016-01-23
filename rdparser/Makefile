CFILE := $(wildcard *.c)

TARGET := rdparser

$(TARGET): $(CFILE)
	$(CC) -o $@ $(CFILE)

.PHONY: clean

clean:
	@rm -f $(TARGET)
