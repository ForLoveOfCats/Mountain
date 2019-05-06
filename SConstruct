import platform
env = Environment()
if platform.system().lower() == "linux":
    env = Environment(CC = 'gcc', CCFLAGS=["-fsanitize=address -g"])

VariantDir('./build', './compiler')
Program(["./build/compiler.c", "./build/ast.c", "./build/parser.c", "./build/transpile.c", "./build/validator.c"])
