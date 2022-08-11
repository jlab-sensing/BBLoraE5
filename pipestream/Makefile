CFLAGS = -Wall -Wextra -g
LDLIBS = -lipc

OBJS := pipestream.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all test
all: build/pipestream

build/pipestream: build/%: build/%.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

.PHONY: install clean

install: all
	install -m 777 build/pipestream /usr/bin

clean:
	rm -rf build/*
