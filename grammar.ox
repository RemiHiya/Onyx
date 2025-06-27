extern lib*;   // exemple d'import de fichier externe
extern lib.foo;
extern lib{foo, bar};

int main([string] args) {
    return 42;
}

int var = 1;            // Auto int 32 bits
u32 var = 1i64;         // Unsigned int 32 bits

type variable = ...;

// exemples de fonctions
int foo(int a, int b) = a + b;
int foo(int a, int b) {
    return a + b;
}

// Struct with some generics
struct Name<T, K, V> {
    // all public fields of the class
    T var;
    [type, size] arr;
    someType someVar;
}

// Adds some methods for a struct, class equivalent
extends SomeStruct {
    K anotherVar; // private fields
    int othervar;
    constructor(...);
    type method(...);
}

ChildStruct extends SomeStruct {

}

