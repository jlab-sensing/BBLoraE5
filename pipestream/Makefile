CFLAGS = -Wall -Wextra -g
CPPFLAGS = -I./external/libipc/include
LDFLAGS = -L./external/libipc/build
LDLIBS = -lipc

OBJS := pipestream.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all test
all: libipc build/pipestream

build/pipestream: build/%: build/%.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

.PHONY: libipc
libipc: external/libipc/Makefile
	cd external/libipc/ && make

.PHONY: clean
clean:
	rm -rf build/*
	cd external/libipc && make clean
