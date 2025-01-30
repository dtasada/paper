src = *.cpp
in = $(src)

out = ./test

flags = -std=c++23 -Wall -O3
linker = -lraylib -lfcl -lccd

cc ?= clang++

release:
	@echo "Building in release mode"
	$(cc) $(release_flags) \
		$(in) \
		$(linker) \
		-o $(out)

run: release
	$(out)

build: release
