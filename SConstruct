import platform
env = Environment()
if platform.system().lower() == "linux":
    env = Environment(CC = 'gcc', CCFLAGS=["-fsanitize=address", "-g"], LINKFLAGS=["-fsanitize=address", "-g", "-lasan"])

env.VariantDir('./build', './compiler')
env.Program(["./build/compiler.c", "./build/ast.c", "./build/parser.c", "./build/transpile.c", "./build/validator.c"])
