target = ./target/paper

build:
	g++ src/*.cpp -o $(target) -lraylib

run: build
	$(target)
