CC = gcc
#IDIR = /usr/include /usr/local/include
#LDIR = /usr/lib

#/usr/src /usr/local/src
SRC_DIR := src
OBJ_DIR := lib
BIN_DIR := .

EXE := $(BIN_DIR)/lora #/usr/local/BBLoraE5/bin/lora 
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP -g
CFLAGS := -Wall
LDFLAGS := -Llib
LDLIBS := -lm

.PHONY: all clean

install: all
	install -m 777 lora /usr/bin

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
