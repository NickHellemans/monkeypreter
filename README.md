# Monkeypreter

This repository is the result of going through the amazing book [Writing An Interpreter In Go](https://interpreterbook.com/) by Thorsten Ball, but using a different implementation language to challenge myself to truely understand what's going on. Following the book I implemented the Monkey programming language as a tree-walking 
interpreter, using the language of all languages C. The interpreter comes with it's own mark & sweep garbage collector to take the trash out. 

<p align="center" width="100%">
<img src="https://monkeylang.org/images/logo.png" width="120" height="120"/>
</p>

## Monkey

### Features
- C-like syntax
- Variable bindings
- Integers and booleans
- Arithmetic expressions
- Built-in functions (len, first, last, cdr, push & print)
- First-class and higher-order functions
- Closures
- A string data structure
- An array data structure

### Syntax
#### Bindings ints, strings, booleans
``` C
let age = 1;
let name = "Monkey";
let result = 10 * (20 / 2);
```

#### Bindings: arrays
``` C
let myArray = [1, 2, 3, 4, 5];
myArray[0] // => 1
```

#### Bindings: functions
``` C
let add = fn(a, b) { return a + b; };
let add = fn(a, b) { a + b; }; //--> Implicit return
```

#### Fibonacci example (recursive calls)
``` C
let fibonacci = fn(x) {
	if (x == 0) {
		0
	} else {
	if (x == 1) {
		1
	} else {
		fibonacci(x - 1) + fibonacci(x - 2);
		}
	}
};
```

### Higher order functions (fn as arguments)
``` C
let twice = fn(f, x) {
	return f(f(x));
};
let addTwo = fn(x) {
	return x + 2;
};
twice(addTwo, 2); // => 6
