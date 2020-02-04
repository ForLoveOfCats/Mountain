printf "===== Compiler Building =====\n"
zig build


if [ $? -eq 0 ]; then #if the compiler was built correctly
	printf "\n\n===== Running Compiler =====\n"
	./zig-cache/bin/Mountain --build ./reproduction/ #run the compiler
fi
