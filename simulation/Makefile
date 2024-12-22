src = src/*.cpp
src_engine = src/engine/*.cpp
in = $(src) $(src_engine) 

out = ./paper

release_flags = -std=c++23 -Wall -O3 -fopenmp
debug_flags = -std=c++23 -Wall -g -fsanitize=address -fopenmp
linker = -lraylib -lrlimgui -limgui \
		 -lfcl -lccd -ltomlplusplus

cc ?= clang++

release:
	@echo "Building in release mode"
	$(cc) $(release_flags) \
		$(in) \
		$(linker) \
		-o $(out)

debug:
	@echo "Building in debug mode"
	$(cc) $(debug_flags) \
		$(in) \
		$(linker) \
		-o $(out)

run: debug
	$(out)

build: debug
