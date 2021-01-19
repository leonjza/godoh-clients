BIN_DIR=dist
SOURCES=$(wildcard src/*.c src/lib/*.c src/lib/tinyaes/*.c src/lib/parson/*.c src/lib/base64/*.c)
WARNINGS=-Wall -Wextra -Werror
LINKER=-lresolv -lz
DNSDOMAIN=
CC=clang

DEFAULT=$(CC) $(SOURCES) $(WARNINGS) $(LINKER) $(DNSDOMAIN)

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
	$(DEFAULT) -Os -o $(BIN_DIR)/godoh

debug:
	mkdir -p $(BIN_DIR)
	$(DEFAULT) -g -DDEBUG -o $(BIN_DIR)/godoh-debug

static:
	mkdir -p $(BIN_DIR)
	$(DEFAULT) -Os -g -o $(BIN_DIR)/godoh-static -static
