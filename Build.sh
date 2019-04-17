scons #build

if [ $? -eq 0 ]; then #if the build succeeded
	printf "\n\n===============================\n\n\n"
	./build/compiler ./examples/example.mountain #run the executable
fi
