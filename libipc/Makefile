CFLAGS = -Wall -Wextra -g
CPPFLAGS = -I./include -I./external/ctest
LDFLAGS = -L./build
LDLIBS = -lipc

PUB_HEADERS := ipc.h

OBJS := ipc.o generic.o client.o server.o
TESTS := ipc.test
EXAMPLES := client server

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))
TESTS := $(addprefix build/,$(TESTS))
EXAMPLES := $(addprefix build/,$(EXAMPLES))

.PHONY: all test examples
all: build/libipc.a build/libipc.so
test: build/libipc.a $(addsuffix .run,$(TESTS))
examples: build/libipc.a $(EXAMPLES)

# Marks .out as secondary
# aka prevents deletion due to intermediatry files
.SECONDARY: $(addsuffix .out,$(TESTS))

build/libipc.so: $(OBJS)
	$(CC) -shared -Wl,-soname,libipc.so -o $@ $^

# main library
build/libipc.a: $(OBJS)
	$(AR) rcs $@ $^

# Implicit targets
build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

build/%: examples/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

build/%.test.run: build/%.test.out
	$<

build/%.test.out: build/%.test.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: install clean

install: all
	install -m 777 build/libipc.so /usr/lib
	install -m 777 include/$(PUB_HEADERS) /usr/include

uninstall:
	rm /usr/lib/libipc.so
	rm /usr/include/$(PUB_HEADERS)

clean:
	rm -rf build/*
