BIN_DIR := zig-cache

default: clean build

clean:
	$(RM) $(BIN_DIR)/bin/*

run:
	zig build run
	ls -l ls zig-cache/bin

build:
	zig build -Drelease-small=true
	ls -lah zig-cache/bin
