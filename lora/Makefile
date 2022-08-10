SRC_DIR := src
BIN_DIR := build

EXE := $(BIN_DIR)/lora
OBJ := lora.o main.o
OBJ := $(addprefix $(BIN_DIR)/,$(OBJ))

CPPFLAGS := -Isrc -Wall -Wextra -g
LDLIBS := -lm -lrobotcontrol

.PHONY: all
all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: install clean

install: all
	install -m 777 lora /usr/bin

clean:
	$(RM) -rv $(BIN_DIR)
