CC = gcc
#IDIR = /usr/include /usr/local/include
#LDIR = /usr/lib

#/usr/src /usr/local/src
SRC_DIR := /usr/local/BBLoraE5/teros_client/src
OBJ_DIR := /usr/local/BBLoraE5/teros_client/lib
BIN_DIR := .

EXE := $(BIN_DIR)/tcli
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS := -Wall
LDFLAGS := -Llib
LDLIBS := -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ /usr/lib/librobotcontrol.so
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@
	
clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)
	
-include $(OBJ:.o=.d)

#lora: lora.c librobotcontrol.so
#	gcc -o lora lora.c librobotcontrol.so
	#/usr/lib/librobotcontrol.so
	
#librobotcontrol.o:
