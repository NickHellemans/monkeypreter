# Monkeypreter

This repository is the result of going through the amazing book [Writing An Interpreter In Go](https://interpreterbook.com/) by Thorsten Ball, but using a different language 
instead to challenge myself to truely understand what's going on. Following the book I am currently implementing the Monkey programming language as a tree-walking 
interpreter, using the language of all languages C. 

### Parts:
- Lexer: Done
- Parser: Done
- Evaluation: Ongoing
  
### TO DO:
- Error handling 
	- Check every created malloc ptr for null
 	- Handle null checks: exit or return? 
   	- Free memory before exit / return
## Monkey

### Features
- C-like syntax
- Variable bindings
- Integers and booleans
- Arithmetic expressions
- Built-in functions
- First-class and higher-order functions
- Closures
- A string data structure
- An array data structure
- A hash data structure
- Whitespace irrelevant

### Syntax
#### Bindings ints, strings, booleans
``` C
let age = 1;
let name = "Monkey";
let result = 10 * (20 / 2);
```

#### Bindings arrays and maps
``` C
let myArray = [1, 2, 3, 4, 5];
let thorsten = {"name": "Thorsten", "age": 28};
myArray[0] // => 1
thorsten["name"] // => "Thorsten"
```

#### Bindings functions
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
