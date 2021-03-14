# Mountain, a rather simple language + compiler

This is the original compiler for a toy language I was building about a
year and a half ago now. It was written entirely in C and generated
executables by outputting C source which was then built with a C compiler.
There are numerious problems with the implementation which drove me to ditch
it and start over again before dropping the project entirely.

Issues:
 * Conflated the parse tree and semantic tree. This would not be an overly egregious
  issue if it weren't for the next issue as some compilers intentionally model their
  internals like this to reduce allocations for the sake of speed
 * Uses a single struct to represent all potential nodes in the program tree. This
  means that if even one node needs a field then all nodes had that field. This really
  should have been a union.
 * The expression parser is just plain god-awful. It was my first of many expression
  parsers and really had not business working as well as it did. In a desperate attempt
  to correctly handle operator precedence I turned it into the most difficult to follow
  spagetti.

To play around with it use the `Debug.sh`, `Test.sh`, and `Reproduction.sh` scripts to
build and run the compiler on various pieces of Mountain code. This will only work on a
POSIX-ish OS. By default they will use `clang` to build the compiler and emitted C source.
This can be trivially swapped out for `gcc` by simply modifying the scripts.
 * `Debug.sh`: This will build and run the Mountain code under `./examples`
 * `Test.sh`: This will build and run the "test suite" under `./tests`
 * `Reproduction.sh`: This will build and run the stub Mountain code under `./reproduction`
 * `Build.sh`: This will just build the Mountain compiler itself, used by the other
  scripts

As mentioned I eventually gave up on this implementation and began again with a new codebase
being implemented in Zig. The remains of how far I got is still present in another branch. I
ended up scraping the project in frustration in having Zig constantly being shifted and changed
beneath my feet. That of course is my fault in attempting to build a "serious" project in an
actively evolving language.

# Original ReadMe
## Mountain Programming Language
### A (hopefully) fast, C compatible, language designed to enable greatness


#### Welcome to the Github repo for the (WIP) Mountain bootstrap compiler

The goal is to have a self hosted compiler by the end of
summer 2020. At the moment Mountain rather far away from that goal.


#### Vision

Mountain has several core tenants:
* Usefulness is key, all other tenants can be sacrificed in the name
  of usefulness.
    * Seamless C interop is important for usefulness.
    * High performance is important for usefulness.
* Explicitness is important for readability and reason-ability.
    * Explicit does not always have to mean verbose
* Maximizing expressiveness is valuable where possible
    * Balancing this with performance is a razors edge, trust the
      developer to use the right language tools where appropriate.
* Avoid restricting the developer's freedom for philosophical reasons.
* It is not the language's job to enforce a specific coding "style".

Mountain draws much inspiration from Zig, Rust, and of course its
parent language C.


## This language is nowhere near usable for literally anything
This is currently little more than a fun side project with hopes of
eventually becoming more. Read the feature wishlist
[here](WISHLIST.md) for more information about what Mountain hopes to
be capable of. Feel free to poke around.

Be aware that this is *extremely* WIP. There are many areas where
things should change or be improved. Performance could be improved by
fixing a few rather egregious design cases (symbol lookup is a worst
case O(n) while loop checking each symbol with strcmp, the nodes
should be tagged unions, I should be using string interning,
ect). Originally this compiler was being built to be thrown away once
a self-hosted compiler could be built. However now the goal is to port
this codebase into Mountain so these design oversights are slowly
being corrected.

As of currently Mountain is a transpiling language in that the
compiler emits C source code which is then fed through GCC or Clang in
order to produce the final binary. Mountain should eventually move to
directly using LLVM. I decided to not use LLVM right out of the gate
due to the high barrier of entry to utilizing LLVM and the extra work
it will take to ensure ABI compatibility with various C
compilers. With the goal to get the language functional and useful
quickly I opted to transpile to C for now.


#### Currently supported features
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
* Stuct definition (TODO dependency loops not yet detected, reliant on
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
  think of


### This is currently only tested on Linux

It will probably work on macOS but *will not* work on Windows as it
uses `dirent.h`. This will change in the future, until then if you
really want to run this on Windows go grab an dirent wrapper or open
an issue to bug me into prioritizing this more.


### Building

Requires SCons and a recent revision of Clang. It will also build with
GCC but depending on the platform the `SConstruct` will need to be
modified.

To perform the most basic build simply execute the command `scons`
while in the repo root.

The shell script `Debug.sh` builds the compiler, compiles the code
under `./examples`, and then runs it. The other script `Test.sh`
builds the compiler, builds the test code under `./tests`, and then
runs those tests.
