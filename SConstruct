import platform
env = Environment()
if platform.system().lower() == "linux":
    env = Environment(CC = 'gcc',
                      CCFLAGS=["-fsanitize=address", "-fsanitize=undefined", "-g", "-Wall", "-Wextra", "-Wpedantic"],
                      LINKFLAGS=["-fsanitize=address", "-fsanitize=undefined", "-g"])

env.VariantDir('./build', './compiler')
env.Program(["./build/compiler.c", "./build/ast.c", "./build/parser.c", "./build/transpile.c", "./build/validator.c", "./build/symbols.c"])
