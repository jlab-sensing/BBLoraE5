CFLAGS = -Wall -Wextra -g
CPPFLAGS = -I./external/libipc/include
LDFLAGS = -L./external/libipc/build
LDLIBS = -lipc

OBJS := teroslogger.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all install uninstall
all: libipc build/teroslogger

install: all
	cp build/teroslogger /usr/bin

uninstall:
	rm -f /usr/bin/teroslogger

build/teroslogger: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: libipc
libipc: external/libipc/Makefile
	cd external/libipc/ && make

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf build/*
	cd external/libipc/ && make clean
