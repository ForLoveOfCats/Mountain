# Mountain Wishlist:
## (In no specific order)

* Full C interop
* Custom structs
* Methods on objects
* Struct embedding
* Interfaces
* Code generation for static dispath wherever possible
* Optional late binding when needed
* Function and struct generics
* Overriding methods from embeded structs (?)
* Adding methods to imported structs
* Explicit function overloading
* Operator overloading
* Some sort of foreach with iterators (interface?)
* Native enum iteration
* Native enum to string conversion
* Standardized pretty and debug to string interfaces
* Inline string building (see C#'s string interpolation)
* Defer statement
* A module system (file path agnostic)
* Built in build system
* Standard library methods for working with UTF-8
* Type aliases (with distinct types)
* Ergonomic inline type casting
* Some sort of mechanism for providing custom logic to functions from
  callsite (like first class functions) but without the runtime cost
  of a function pointer
* Scope tags with named break
* Block as expression which evaluate to a value
* Don't have to worry about what order one delcares types, functions,
  and global variables in.
* Optionals
* A `using` keyword to bring the contents of a value or module into scope
* Syntax to run a block in the scope of a value inline (inline `using`)
* Ability to instance any type on stack or heap (without anything like
  `malloc(sizeof(T))` followed by setting the fields. That is
  terrible)
* Ergonomically modify collections while iterating through them
* Tests as a language feature
* Automatic HTML (and other formats) documentation generation (with doc-coments)
* Ability to reference symbols in doc comments which can be checked by
  the compiler and updated by refactoring tools
* Some form of hygenic macro expansion system
* Compile time code execution (full language minus external libraries)
  to generate code at compile time (?)
* A language server for IDE features
* *Maybe eventually have* runtime code parsing and exectuion
* *Maybe eventually have* C++ library support (This is a doozy of a
  feature. C and C++ differ so much, let alone the differences between
  C++ and Mountain. TBD)
