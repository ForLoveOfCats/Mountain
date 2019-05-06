import platform
env = Environment()
env['SYSTEM'] = platform.system().lower()

if env['SYSTEM'] == "linux":
    env.Append( CCFLAGS=["-fsanitize=address -g"] )


VariantDir('./build', './compiler')
Program(["./build/compiler.c", "./build/ast.c", "./build/parser.c", "./build/transpile.c", "./build/validator.c"])
