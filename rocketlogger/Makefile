CFLAGS = -Wall -Wextra -g
#CPPFLAGS = -I./external/libipc/include
#LDFLAGS = -L/usr/lib
LDLIBS = -lipc

OBJS := teroslogger.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all install uninstall
all: build/teroslogger

install: all
	cp build/teroslogger /usr/bin

uninstall:
	rm -f /usr/bin/teroslogger

build/teroslogger: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf build/*
