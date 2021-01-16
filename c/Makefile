BIN_DIR=dist
SOURCES=$(wildcard src/*.c src/lib/*.c )
WARNINGS=-Wall -Wextra -Werror
LINKER=-lresolv -lz
LD_FLAGS=-Os
DNSDOMAIN=
CC=clang

# set the domain based on the value we get from the DOMAIN argument
ifneq ($(DOMAIN),)
	DNSDOMAIN := -DDOMAIN="\"$(DOMAIN)\""
endif

.PHONY: default

default: clean build

clean:
	$(RM) -rf $(BIN_DIR)/*

build:
	mkdir -p $(BIN_DIR)
	$(CC) $(SOURCES) $(WARNINGS) $(LINKER) $(DNSDOMAIN) $(LD_FLAGS) -o $(BIN_DIR)/godoh

debug:
	mkdir -p $(BIN_DIR)
	$(CC) -g -DDEBUG $(SOURCES) $(WARNINGS) $(LINKER) $(DNSDOMAIN) -o $(BIN_DIR)/godoh-debug
