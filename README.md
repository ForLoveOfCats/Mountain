# Mountain Programming Language
## A (hopefully) fast, C compatible, language designed to enable greatness


### Welcome to the Github repo for the (WIP) Mountain bootstrap compiler

My current goal with Mountain it to have it useful for writing real
software as soon as possible. **It is not yet usable**


### Vision

Mountain has several core tenants:
* Usefulness is key, all other tenants can be sacrificed in the name
  of usefulness.
    * Seamless C interop is important for usefulness.
    * High performance is important for usefulness.
	* Readability of code is important for usefulness by way of
      maintainability.
* Encourage writing good code by making it easy to do the "right
  thing".
    * Yet at the same time avoid *significantly* restricting the
    developer's freedom.
    * A specific coding "style" should not be enforced but instead
    encouraged.
* Explicitness is important for readability and reason-ability.
    * Explicit does not always have to mean verbose.
* Maximizing expressiveness is valuable where possible.
    * This includes such things as block expressions, operator
      overloading, explicit function overloading, ect.

Mountain draws heavily from Zig and C with Rust, Odin, and Jai
providing some auxiliary inspiration.


# This language is nowhere near usable for literally anything
This is currently little more than a fun side project. While it is
currently defined as a toy language I fully intend on bringing it
beyond "toy" status. Read the feature wishlist [here](WISHLIST.md) for
more information about what Mountain hopes to be capable of. Feel free
to poke around.

Be aware that this is *extremely* WIP and undergoing a complete
rewrite. The compiler was initially written in C but is being
redesigned from the ground up in Zig. The original C implementation
was capable of compiling and running all sources in the `./examples`
and `./tests` and is accessable from the `OriginalC` branch.

Currently the compiler is being built to transpile. That means that
the compiler emits C source code which is then fed through GCC or
Clang in order to produce the final binary. Mountain should eventually
move to directly using LLVM in addition to a custom, minimally
optimizing, direct machine code generating backend. I decided to not
use LLVM right out of the gate due to the high barrier of entry to
utilizing LLVM and the extra work it will take to ensure ABI
compatibility with various C compilers. With the goal to get the
language functional and useful quickly I opted to transpile to C for
now.

The gameplan at the moment is to finish fleshing out the parser until
it is capable of correctly parsing a useful subset of the Mountain
syntax. Then the validator can begin to be built which will transform
the parse tree into a semantic tree. Once the validator is
sufficiently capable the transpiler can be easily built to spit out C
code from the valid semantic tree.


### Features **originally** supported by the **C implementation**
* Variable declaration (requires a value or explicitly left undefined)
* Variable value set
* Global variables
* Symbol scoping
* Basic type system (current builtin types are `u8`, `i32`, `Bool`,
  and `Ptr`, also supports custom types (enums and structs))
* Function declarations (with parameters and return type)
* Return values from non-Void functions (can also return no value from
  Void functions to early exit)
* Basic function return tracing
* Function calls (with and without return value)
* Enums definition and usage
* Stuct definition (TODO dependency loops were not detected, reliant on
  order relative to other structs)
* Stack allocated struct initialization
* Read and write to struct fields
* Order of global symbols does not matter
* Expression parsing with precedence
* Boolean inversion with `!`
* Full set of comparison operators (==, !=, >, <, >= <=)
* Basic pointers (take address of, compare, dereference, and overwrite
  value pointed at)
* Basic module system (each file declares what module it exists in,
  all files in a module share a single namespace (but not imports) and
  modules can be imported (accessing contents requires prefixing with
  module name) or `using` imported (does not require module name
  prefix))
* Conditional flow control with `if`/`elif`/`else`
* Conditional flow control with `while` (includes `break` and `continue`)
* Basic in-langauge test system (simply returns a bool for
  success/failure, not final design)
* Functions can be defined inside other functions
* Single line comments
* Character literal parsing (with escape sequence support)
* Semi-decent parse errors
* As many validation error checks (with decent messages) that I could
  think of at the time


## This is currently only tested on Linux

However it *should* work unmodified on Windows and macOS as well. If
you are on one of these platforms and it does not build please open an
issue.


## Building

Requires Zig 0.5.* On Linux/macOS the `./Debug.sh` script will build
the compiler then attempt to run it on the files under
`./reproduction`. To just build (on any platform) simply `zig build`
which will output a binary at `./zig-cache/bin/Mountain`. Cheers!
