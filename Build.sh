mkdir -p ./build;

clang \
	-g \
	-fsanitize=address -fsanitize=undefined \
	-Wall -Wextra -Wpedantic \
	./compiler/main.c -o ./build/compiler