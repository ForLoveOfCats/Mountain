# Mountain Wishlist:
## (In no specific order)

* Full C interop
* Custom structs
* Methods on objects
* Struct embedding
* Interfaces (for function arguments, code generation for static dispatch)
* Function and struct generics
* Overriding methods from embeded structs
* Method overloading
* Operator overloading
* Infix functions
* Some sort of foreach with iterators (interface?)
* Native enum iteration
* Native enum to string conversion
* Standardized pretty and debug to string methods (interfaces?)
* Inline string building (see C#'s string interpolation)
* Defer statement
* A module system
* Built in build system
* Standard library methods for working with UTF-8
* Variable redeclaration with different type (see Rust's variable shadowing)
* Special form of pointer which manages autofreeing
* First class types
* Inline type casting without enclosing value in parenthesis
* First class methods
* Infix methods
* Named blocks/named break
* Block as expression which evaluates to a value
* Don't have to worry about what order one delcares methods in (automatic
function prototype in resulting C source)
* Optionals (on pointers too? probably not)
* Failable methods are marked as such and have to be handled
* Syntax to call multiple methods (inline) on a value without using its name
* Ability to instance any type on stack or heap (without anything like
`malloc(sizeof(T))` followed by setting the fields. That is terrible)
* Tests as a language feature
* Automatic HTML (and other formats) documentation generation (with doc-coments)
* Some form of metaprogramming/compiler time code generation
* Modify collections while foreaching through them (possibly feasable?)
* A language server for IDE features
* Compile time code execution (bytecode interpreter or AST walker)
* *Maybe eventually have* runtime code parsing and exectuion (bytecode
interpreter or AST walker)
