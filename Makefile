# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

# define target
TARGET=image_editor

build: $(TARGET)

image_editor: main.c memory.c file_io.c transformations.c commands.c
	$(CC) $(CFLAGS) main.c memory.c file_io.c transformations.c commands.c -o image_editor -lm

clean:
	rm -f $(TARGET)

.PHONY: build pack clean