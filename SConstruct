import platform



env = Environment()
if platform.system().lower() == "linux":
    env = Environment(CC = 'clang',
                      CCFLAGS=["-fsanitize=address", "-fsanitize=undefined", "-g", "-Wall", "-Wextra", "-Wpedantic"],
                      LINKFLAGS=["-fsanitize=address", "-fsanitize=undefined", "-g"])
else:
    print("Your platform '%s' is not supported by Mountain at this time." % (platform.system()))
    print("As the bootstrap compiler is written in portable C with only minimal library use it may work")
    print("However it cannot be guarenteed")
    input("Press enter to continue building")


env.VariantDir('./build', './compiler')
env.Program(["./build/compiler.c", "./build/ast.c", "./build/parser.c", "./build/transpile.c", "./build/validator.c", "./build/symbols.c"])
