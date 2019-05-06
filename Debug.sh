
printf "===== Compiler Building =====\n"
scons #build

if [ $? -eq 0 ]; then #if the compiler was built correctly
	printf "\n\n===== Example Building =====\n"
	./build/compiler ./output/output.c ./examples/example.mountain #run the compiler

	if [ $? -eq 0 ]; then #if the compiler succeeded
		printf "\n\n===== Transpiled Source Building =====\n"
		gcc -fsanitize=address -g -o ./output/output ./output/output.c #compile the transipile results

			if [ $? -eq 0 ]; then #if the transpiled source compiled correctly
				printf "\n\n===== Executing Final Binary =====\n"
				./output/output #run the final binary
			fi
	fi
fi
