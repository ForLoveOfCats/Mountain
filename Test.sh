
printf "===== Compiler Building =====\n"
sh ./Build.sh #build

if [ $? -eq 0 ]; then #if the compiler was built correctly
	printf "\n\n===== Tests Building =====\n"
	./build/compiler test ./tests ./output/output.c #run the compiler

	if [ $? -eq 0 ]; then #if the compiler succeeded
		printf "\n\n===== Transpiled Source Building =====\n"
		clang -g -fsanitize=address -fsanitize=undefined -Wreturn-type -Wpedantic -o ./output/output ./output/output.c #compile the transpiled results

		if [ $? -eq 0 ]; then #if the transpiled source compiled correctly
			printf "\n\n===== Executing Final Binary =====\n"
			./output/output #run the final binary
		fi
	fi
fi
