CFLAGS = -Wall -Wextra -g
LDLIBS = -lipc

OBJS := csvpipe.o libcsv.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all test
all: build/csvpipe.o build/libcsv.o

build/csvpipe: build/%: build/%.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

.PHONY: install clean

install: all
	install -m 777 build/csvpipe /usr/bin

clean:
	rm -rf build/*
