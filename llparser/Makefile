CFILE := $(wildcard *.c)

TARGET := llparser

$(TARGET): $(CFILE)
	$(CC) -o $@ $(CFILE)

.PHONY: clean

clean:
	@rm -f $(TARGET)
