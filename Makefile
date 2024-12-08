src = simulation/src/*.cpp
src_engine = simulation/src/engine/*.cpp
lib = simulation/lib/rlImGui/*.cpp 
in = $(src) $(src_engine) $(lib)

out = ./paper

flags = -std=c++23 -Wall -O3 -g
linker = -lraylib -limgui

# cc = g++
cc = clang++

build:
	$(cc) $(flags) \
		$(in) \
		$(linker) \
		-o $(out)

run: build
	$(out)
