.PHONY: default

BIN_DIR=dist
SOURCES=$(wildcard src/*.c)
WARNINGS=-Wall -Wextra -Werror
LINKER=-lresolv
OPTI=-Os
CC=clang

default: clean build

clean:
	$(RM) $(BIN_DIR)/*

build:
	mkdir -p $(BIN_DIR)
	$(CC) $(SOURCES) $(WARNINGS) $(LINKER) $(OPTI) -o $(BIN_DIR)/godoh

debug:
	mkdir -p $(BIN_DIR)
	$(CC) -g -DDEBUG $(SOURCES) $(WARNINGS) $(LINKER) -o $(BIN_DIR)/godoh-debug
