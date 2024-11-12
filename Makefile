in = simulation/src/*.cpp simulation/src/engine/*.cpp 
out = ./paper

args = -std=c++23 -Wall -O3
linker = -lraylib

build:
	g++ $(args) $(in) $(linker) -o $(out)

run: build
	$(out)
