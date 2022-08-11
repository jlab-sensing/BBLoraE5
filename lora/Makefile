SRC_DIR := src
BIN_DIR := build

EXE := lora
OBJ := lora.o main.o libcsv.o uart.o
OBJ := $(addprefix $(BIN_DIR)/,$(OBJ))

CPPFLAGS := -Isrc -Wall -Wextra -g
LDLIBS := -lm -lipc

.PHONY: all
all: $(BIN_DIR)/$(EXE)

$(BIN_DIR)/$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: install clean

install: all
	install -m 777 lora /usr/bin

clean:
	$(RM) -rv $(BIN_DIR)/*
