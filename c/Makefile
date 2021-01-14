.PHONY: default

BIN_DIR=dist
SOURCES=$(wildcard *.c)
WARNING=-Wall
LINKER=-lresolv

default: clean build

clean:
	$(RM) $(BIN_DIR)/*

build:
	mkdir -p $(BIN_DIR)
	clang $(SOURCES) $(WARNING) $(LINKER) -o $(BIN_DIR)/godoh

debug:
	mkdir -p $(BIN_DIR)
	clang -DDEBUG $(SOURCES) $(WARNING) $(LINKER) -o $(BIN_DIR)/godoh-debug
