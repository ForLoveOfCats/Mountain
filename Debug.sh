
printf "===== Compiler Building =====\n"
scons #build

if [ $? -eq 0 ]; then #if the compiler was built correctly
	printf "\n\n===== Example Building =====\n"
	./build/compiler ./examples ./output/output.c #run the compiler

	if [ $? -eq 0 ]; then #if the compiler succeeded
		printf "\n\n===== Resulting C Source =====\n"
		cat ./output/output.c

		printf "\n\n===== Transpiled Source Building =====\n"
		clang -g -fsanitize=address -fsanitize=undefined -Wreturn-type -Wpedantic -o ./output/output ./output/output.c #compile the transpile results

			if [ $? -eq 0 ]; then #if the transpiled source compiled correctly
				printf "\n\n===== Executing Final Binary =====\n"
				./output/output #run the final binary
			fi
	fi
fi
