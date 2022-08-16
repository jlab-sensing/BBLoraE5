CFLAGS = -Wall -Wextra -g
LDLIBS = -lipc

OBJS := csvstream.o libcsv.o

# Add build prefix
OBJS := $(addprefix build/,$(OBJS))

.PHONY: all test
all: build/csvstream.o build/libcsv.o

build/csvstream: build/%: build/%.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS) -o $@

build/%.o: src/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $^ -o $@

.PHONY: install clean

install: all
	install -m 777 build/csvstream /usr/bin

clean:
	rm -rf build/*
