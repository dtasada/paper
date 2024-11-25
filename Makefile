src =  simulation/src/*.cpp
incl = simulation/src/engine/*.cpp
lib = simulation/lib/rlImGui/*.cpp simulation/lib/imgui/*.cpp
in = $(src) $(incl) $(lib)

out = ./paper

args = -std=c++23 -Wall -O3 -I/opt/homebrew/Cellar/raylib/5.0/include/ -L/opt/homebrew/Cellar/raylib/5.0/lib/
linker = -lraylib

build:
	g++ $(args) $(in) $(linker) -o $(out)

run: build
	$(out)
