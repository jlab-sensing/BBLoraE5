CFLAGS = -Wall -Wextra -g
CPPFLAGS = -I./include -I./external/ctest
LDFLAGS = -L./build
LDLIBS = -lipc

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
	$(CC) -shared -Wl,-soname,$@ -o $@ $^

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

install: build/libipc.so
	mv $< /usr/lib/libipc.so

uninstall:
	rm /usr/lib/libipc.so

clean:
	rm -rf build/*
