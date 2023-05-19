CFLAGS = -Wall -Wextra -g
LDLIBS = -lipc

OBJS := teroslogger.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all 
all: build/teroslogger

build/teroslogger: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: clean install uninstall

install: all
	install -m 777 build/teroslogger /usr/bin

uninstall:
	rm -f /usr/bin/teroslogger

clean:
	rm -rf build/*
