src =  simulation/src/*.cpp
incl = simulation/src/engine/*.cpp
lib = simulation/lib/rlImGui/*.cpp
in = $(src) $(incl) $(lib) 

out = ./paper

args = -std=c++23 -Wall -O3
linker = -lraylib -limgui

build:
	g++ $(args) $(in) $(linker) -o $(out)

run: build
	$(out)
