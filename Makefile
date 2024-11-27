src =  simulation/src/*.cpp
src_engine = simulation/src/engine/*.cpp
lib = simulation/lib/rlImGui/*.cpp 
in = $(src) $(src_engine) $(lib)

out = ./paper

args = -std=c++23 -Wall -O3
linker = -lraylib -limgui

# cc = g++
cc = clang++

build:
	$(cc) $(args) \
		$(in) \
		$(linker) \
		-o $(out)

run: build
	$(out)
