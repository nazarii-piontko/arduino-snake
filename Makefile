.PHONY: all

all:
	emcc main.cpp ../game.cpp -s WASM=1 -o ../wasm-test/snake.js