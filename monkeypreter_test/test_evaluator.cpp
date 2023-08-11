#include "gtest/gtest.h"

extern "C" {
	#include "parser/parser.h"
	#include "parser/parser.c"
	#include "parser/ast.h"
	#include "parser/ast.c"
	#include "evaluator/object.h"
	#include "evaluator/object.c"
	#include "evaluator/environment.h"
	#include "evaluator/environment.c"
	#include "evaluator/hash_map.h"
	#include "evaluator/hash_map.c"
	#include "evaluator/gc.h"
	#include "evaluator/gc.c"
	#include "evaluator/builtins.c"
	#include "evaluator/builtins.h"
	#include "evaluator/evaluator.h"
	#include "evaluator/evaluator.c"
}

struct Object* testEval(const char* input) {
	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);
	Program* program = parseProgram(&parser);
	struct MonkeyGC* gc = createMonkeyGC();
	struct ObjectEnvironment* env = newEnvironment(gc);
	struct Object* obj = evalProgram(program, env);
	freeProgram(program);
	freeParser(&parser);
	deleteEnvironment(env);
	//Can't delete GC --> Deletes all objects including last object + object might depend on other objects (array)
	//deleteMonkeyGC(gc);
	return obj;
}

bool testIntegerObject(const struct Object* obj, int64_t expected) {

	if(obj->type != OBJ_INT) {
		printf("Object is not an integer, expected %s, got %s\n", objectTypeToStr(OBJ_INT), objectTypeToStr(obj->type));
		return false;
	}
	if(obj->value.integer != expected) {
		printf("Object has wrong value, expected %lld, got %lld\n", expected, obj->value.integer);
		return false;
	}

	return true;
}

bool testBooleanObject(const struct Object* obj, const bool expected) {

	if (obj->type != OBJ_BOOL) {
		printf("Object is not a bool, expected %s, got %s\n", objectTypeToStr(OBJ_BOOL), objectTypeToStr(obj->type));
		return false;
	}

	if (obj->value.boolean != expected) {
		printf("Object has wrong value, expected %hhd, got %hhd\n", expected, obj->value.boolean);
		return false;
	}

	return true;
}

bool testNullObject(const struct Object* obj) {

	if(obj->type != OBJ_NULL) {
		printf("Object not NULL, got %s", objectTypeToStr(obj->type));
		return false;
	}

	return true;
}

struct ExpectedArray {
	size_t expectedArrSize;
	struct Object expectedObjects[4];
};

union ExpectedVal {
	int64_t expectedInt;
	char expectedString[128];
	void* ptr;
	//If array
	struct ExpectedArray expectedArray;
};

bool testIntegerArrayObject(const struct Object* obj, struct ExpectedArray expected) {

	if(obj->type != OBJ_ARRAY) {
		printf("Object not a array, got %s", objectTypeToStr(obj->type));
		return false;
	}

	struct ObjectList arr = obj->value.arr;

	if(arr.size != expected.expectedArrSize) {
		printf("wrong array size, expected: %llu, got: %llu\n", expected.expectedArrSize, arr.size);
		return false;
	}

	for (size_t i = 0; i < (size_t) arr.size; i++) {
		if (!testIntegerObject(obj->value.arr.objects[i], expected.expectedObjects[i].value.integer)) {
			return false;
		}
	}

	return true;

}
TEST(TestEval, TestEval_01_IntegerExpr) {
	struct TestInteger {
		char input[32];
		int64_t expected;
	} tests[]{
		{"5", 5},
		{"10", 10},
		{"-5", -5},
		{"-10", -10},
		{"5 + 5 + 5 + 5 - 10", 10},
		{"2 * 2 * 2 * 2 * 2", 32},
		{"-50 + 100 + -50", 0},
		{"5 * 2 + 10", 20},
		{"5 + 2 * 10", 25},
		{"20 + 2 * -10", 0},
		{"50 / 2 * 2 + 10", 60},
		{"2 * (5 + 10)", 30},
		{"3 * 3 * 3 + 10", 37},
		{"3 * (3 * 3) + 10", 37},
		{"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
	};

	for(int i = 0; i < 15; i++) {
		printf("Starting test %d\n", i);
		printf("\t - Input = %s\n", tests[i].input);
		printf("\t - Expected = %lld\n", tests[i].expected);
		const struct Object* evaluated = testEval(tests[i].input);
		if(!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
		printf("Ending test %d\n", i);
		printf("\n");
	}

}

TEST(TestEval, TestEval_02_BoolExpr) {
	struct TestInteger {
		char input[20];
		int64_t expected;
	} tests[]{
		{"true", true},
		{"false", false},
		{"1 < 2", true},
		{"1 > 2", false},
		{"1 < 1", false},
		{"1 > 1", false},
		{"1 == 1", true},
		{"1 != 1", false},
		{"1 == 2", false},
		{"1 != 2", true},
		{"true == true", true},
		{"false == false", true},
		{"true == false", false},
		{"true != false", true},
		{"false != true", true},
		{"(1 < 2) == true", true},
		{"(1 < 2) == false", false},
		{"(1 > 2) == true", false},
		{"(1 > 2) == false", true},
	};

	for (int i = 0; i < 19; i++) {
		const struct Object* evaluated = testEval(tests[i].input);
		if (!testBooleanObject(evaluated, tests[i].expected)) {
			FAIL();
		}
	}

}

TEST(TestEval, TestEval_03_BangOperator) {
	struct TestInteger {
		char input[10];
		bool expected;
	} tests[]{
		{"!true", false},
		{"!false", true},
		{"!5", false},
		{"!!true", true},
		{"!!false", false},
		{"!!5", true},
	};

	for (int i = 0; i < 6; i++) {
		struct Object* evaluated = testEval(tests[i].input);
		if (!testBooleanObject(evaluated, tests[i].expected)) {
			FAIL();
		}
	}

}

TEST(TestEval, TestEval_04_IfElseExpr) {
	struct TestInteger {
		char input[30];
		int64_t expected;
	} tests[]{
		{"if (true) { 10 }", 10},
		{"if (false) { 10 }", NULL},
		{"if (1) { 10 }", 10},
		{"if (1 < 2) { 10 }", 10},
		{"if (1 > 2) { 10 }", NULL},
		{"if (1 > 2) { 10 } else { 20 }", 20},
		{"if (1 < 2) { 10 } else { 20 }", 10},
	};

	for (int i = 0; i < 7; i++) {
		struct Object* evaluated = testEval(tests[i].input);

		if(tests[i].expected == NULL) {
			if(!testNullObject(evaluated)) {
				FAIL();
			}
		}
		else {
			if(!testIntegerObject(evaluated, tests[i].expected)) {
				FAIL();
			}
		}
	}

}

TEST(TestEval, TestEval_05_ReturnStatements) {
	struct TestInteger {
		char input[50];
		int64_t expected;
	} tests[]{
	{"return 10;", 10},
	{"return 10; 9;", 10},
	{"return 2 * 5; 9;", 10},
	{"9; return 2 * 5; 9;", 10},
	{
		"if (10 > 1) {if (10 > 1) {return 10;} return 1;}",
		10,
	   },
	};

	for (int i = 0; i < 5; i++) {
		struct Object* evaluated = testEval(tests[i].input);
		if (!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
	}
}

TEST(TestEval, TestEval_06_ErrorHandling) {
	struct TestInteger {
		char input[60];
		char expected[128];
	} tests[]{
		{
		"5 + true;",
		"type mismatch: INTEGER + BOOLEAN",
		},
		{
		"5 + true; 5;",
		"type mismatch: INTEGER + BOOLEAN",
		},
		{
		"-true",
		"unknown operator: -BOOLEAN",
		},
		{
		"true + false;",
		"unknown operator: BOOLEAN + BOOLEAN",
		},
		{
		"5; true + false; 5",
		"unknown operator: BOOLEAN + BOOLEAN",
		},
		{
		"if (10 > 1) { true + false; }",
		"unknown operator: BOOLEAN + BOOLEAN",
		},
		{
		"if (10 > 1) {if (10 > 1) {return true + false;}return 1;}",
		"unknown operator: BOOLEAN + BOOLEAN",
		},
		{
		"foobar",
		"identifier not found: foobar",
		},
		{
			R"("Hello" - "World")",
		"unknown operator: STRING - STRING",
		},
	};

	for (int i = 0; i < 9; i++) {
		struct Object* evaluated = testEval(tests[i].input);
		if(evaluated->type != OBJ_ERROR) {
			printf("No error object returned, got %s\n", objectTypeToStr(evaluated->type));
			FAIL();
		}

		if(strcmp(evaluated->value.error.msg, tests[i].expected) != 0) {
			printf("Wrong error message. Expected: %s, got %s\n", tests[i].expected, evaluated->value.error.msg);
			FAIL();
		}
	}
}

TEST(TestEval, TestEval_07_LetStatements) {
	struct TestInteger {
		char input[60];
		int64_t expected;
	} tests[]{
{"let a = 5; a;", 5},
{"let a = 5 * 5; a;", 25},
{"let a = 5; let b = a; b;", 5},
{"let a = 5; let b = a; let c = a + b + 5; c;", 15},
	};

	for (int i = 0; i < 4; i++) {
		printf("Start test %d\n",i);
		struct Object* evaluated = testEval(tests[i].input);
		printf("Here after eval: type = %d, value = %llu\n", evaluated->type, evaluated->value.integer);
		if (!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
		printf("End test %d\n", i);
	}
}

TEST(TestEval, TestEval_08_FunctionObject) {

	char input[] = "fn(x) { x + 2; };";
	struct Object* evaluated = testEval(input);
	if(evaluated->type != OBJ_FUNCTION) {
		printf("object is not a function, got %d\n", evaluated->type);
		FAIL();
	}

	FunctionObject func = evaluated->value.function;
	if(func.parameters.size != 1) {
		printf("function has wrong parameters length, expected 1, got %llu\n", func.parameters.size);
		FAIL();
	}

	if(strcmp(func.parameters.values[0].value, "x") != 0) {
		printf("Parameter is not 'x', got %s\n", func.parameters.values[0].value);
		FAIL();
	}

	char expectedBody[] = "(x + 2)";

	char* body = (char*) malloc(MAX_PROGRAM_LEN);
	if(!body) {
		perror("malloc (test function body) returned `NULL`\n");
		FAIL();
	}

	body[0] = '\0';
	blockStatementToStr(body, func.body);

	if(strcmp(body, expectedBody) != 0) {
		printf("Body is not: '%s', got '%s'\n", expectedBody, body);
		FAIL();
	}

	free(body);
}

TEST(TestEval, TestEval_09_FunctionApplication) {
	struct TestInteger {
		char input[55];
		int64_t expected;
	} tests[]{
		{"let identity = fn(x) { x; }; identity(5);", 5},
		{ "let identity = fn(x) { return x; }; identity(5);", 5 },
		{ "let double = fn(x) { x * 2; }; double(5);", 10 },
		{ "let add = fn(x, y) { x + y; }; add(5, 5);", 10 },
		{ "let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20 },
		{ "fn(x) { x; }(5)", 5 },
	};

	for (int i = 0; i < 1; i++) {
		printf("Starting test %d\n", i);
		printf("\t - input %s\n", tests[i].input);
		struct Object* evaluated = testEval(tests[i].input);
		if (!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
		printf("Ending test %d\n", i);
	}
}

TEST(TestEval, TestEval_10_Closures) {
	
	const char* input = "let newAdder = fn(x) {"
						"fn(y) { x + y };"
					 "};"

				     "let addTwo = newAdder(2);"
				     "addTwo(2);";

	struct Object* evaluated = testEval(input);
	if (!testIntegerObject(evaluated, 4)) {
		FAIL();
	}
	
}

TEST(TestEval, TestEval_10_Strings) {

	char input[] = "\"Hello World!\"";
	char expected[] = "Hello World!";

	struct Object* evaluated = testEval(input);
	if(evaluated->type != OBJ_STRING) {
		printf("Object is not a string, got %s\n", objectTypeToStr(evaluated->type));
		FAIL();
	}

	if(strcmp(evaluated->value.string, expected) != 0) {
		printf("String has wrong value, expected: %s, got %s\n", expected, evaluated->value.string);
		FAIL();
	}

}

TEST(TestEval, TestEval_11_StringConcat) {

	const char input[] = R"("Hello" + " " + "World!")";
	char expected[] = "Hello World!";

	struct Object* evaluated = testEval(input);
	if (evaluated->type != OBJ_STRING) {
		printf("Object is not a string, got %s\n", objectTypeToStr(evaluated->type));
		FAIL();
	}

	if (strcmp(evaluated->value.string, expected) != 0) {
		printf("String has wrong value, expected: %s, got %s\n", expected, evaluated->value.string);
		FAIL();
	}

}

enum ExpectedType {EXPECT_INT, EXPECT_STRING, EXPECT_NULL, EXPECT_ARRAY};

TEST(TestEval, TestEval_12_BuiltinFunctions) {

	struct TestInteger {
		char input[55];
		ExpectedVal expected;
		//0 for int, 1 for string
		ExpectedType type;

	} tests[]{
		//len
		{"len(\"\")", {.expectedInt = 0}, EXPECT_INT},
		{"len(\"four\")", {.expectedInt = 4}, EXPECT_INT},
		{"len(\"hello world\")", {.expectedInt = 11}, EXPECT_INT},
		{"len(1)", {.expectedString = "argument to `len` not supported, got INTEGER"}, EXPECT_STRING},
		{R"(len("one", "two"))", {.expectedString = "wrong number of arguments. got=2, want=1"}, EXPECT_STRING},
		{"len([])", {.expectedInt = 0}, EXPECT_INT},
		{"len([1,2,3])", {.expectedInt = 3}, EXPECT_INT},
		{"len([1,2, 3 + 1])", {.expectedInt = 3}, EXPECT_INT},
		//first
		{"first([])", {.ptr = nullptr}, EXPECT_NULL},
		{"first([1])", {.expectedInt = 1}, EXPECT_INT},
		{"first([2, 3, 1])", {.expectedInt = 2}, EXPECT_INT},
		{"first([2+2, 3, 1])", {.expectedInt = 4}, EXPECT_INT},
		{"first(1)", {.expectedString = "argument to `first` must be ARRAY, got INTEGER"}, EXPECT_STRING},
		{"first([1], [2])", {.expectedString = "wrong number of arguments. got=2, want=1"}, EXPECT_STRING},
		//last
		{"last([])", {.ptr = nullptr}, EXPECT_NULL},
		{"last([1])", {.expectedInt = 1}, EXPECT_INT},
		{"last([2, 3, 1])", {.expectedInt = 1}, EXPECT_INT},
		{"last([2+2, 3, 1*10])", {.expectedInt = 10}, EXPECT_INT},
		{"last(1)", {.expectedString = "argument to `last` must be ARRAY, got INTEGER"}, EXPECT_STRING},
		{"last([1], [2])", {.expectedString = "wrong number of arguments. got=2, want=1"}, EXPECT_STRING},
		//cdr
		{"cdr([])", { .ptr = nullptr }, EXPECT_NULL},
		{ "cdr([1])", {.expectedArray = {.expectedArrSize = 0, .expectedObjects = {}}}, EXPECT_ARRAY },
		{ "cdr([2, 3, 1])", {.expectedArray = {.expectedArrSize = 2,
			.expectedObjects = {
				{.type = OBJ_INT, .value = {.integer = 3}},
				{.type = OBJ_INT, .value = {.integer = 1}},
			}}}, EXPECT_ARRAY },
		{ "cdr([2+2, 3, 1*10])", {.expectedArray = {.expectedArrSize = 2,
			.expectedObjects = {
				{.type = OBJ_INT, .value = {.integer = 3}},
				{.type = OBJ_INT, .value = {.integer = 10}},
			}}}, EXPECT_ARRAY },
		{ "cdr(1)", {.expectedString = "argument to `cdr` must be ARRAY, got INTEGER"}, EXPECT_STRING },
		{ "cdr([1], [2])", {.expectedString = "wrong number of arguments. got=2, want=1"}, EXPECT_STRING },
		//push
		{"push([], 1)", {.expectedArray = {.expectedArrSize = 1,
			.expectedObjects = {
				{.type = OBJ_INT, .value = {.integer = 1}},
			}}}, EXPECT_ARRAY},
		{"push([1], 3)",  {.expectedArray = {.expectedArrSize = 2,
			.expectedObjects = {
				{.type = OBJ_INT, .value = {.integer = 1}},
				{.type = OBJ_INT, .value = {.integer = 3}},
			}}}, EXPECT_ARRAY},
		{"push([2, 3, 1], 5)", {.expectedArray = {.expectedArrSize = 4,
			.expectedObjects = {
				{.type = OBJ_INT, .value = {.integer = 2}},
				{.type = OBJ_INT, .value = {.integer = 3}},
				{.type = OBJ_INT, .value = {.integer = 1}},
				{.type = OBJ_INT, .value = {.integer = 5}},
			}}}, EXPECT_ARRAY},
		{"push([2+2, 3, 1*10], 20)",  {.expectedArray = {.expectedArrSize = 4,
			.expectedObjects = {
				{.type = OBJ_INT, .value = {.integer = 4}},
				{.type = OBJ_INT, .value = {.integer = 3}},
				{.type = OBJ_INT, .value = {.integer = 10}},
				{.type = OBJ_INT, .value = {.integer = 20}},
			}}}, EXPECT_ARRAY},
		{"push(1)", {.expectedString = "wrong number of arguments. got=1, want=2"}, EXPECT_STRING},
		{"push(1, 2)", {.expectedString = "argument to `push` must be ARRAY, got INTEGER"}, EXPECT_STRING},
			};


	for (int i = 0; i < 32; i++) {
		printf("Testing input: %s\n", tests[i].input);
		struct Object* evaluated = testEval(tests[i].input);
		printf("Evaluated: %s\n", inspectObject(evaluated));

		switch (tests[i].type) {
			case EXPECT_INT:
				if (!testIntegerObject(evaluated, tests[i].expected.expectedInt)) {
						FAIL();
					}
					break;

			case EXPECT_STRING:
				if(evaluated->type != OBJ_ERROR) {
					printf("Object is not an error, got %s", objectTypeToStr(evaluated->type));
					FAIL();
				}

				if(strcmp(evaluated->value.error.msg, tests[i].expected.expectedString) != 0) {
					printf("Wrong error msg, expected: %s, got: %s\n", tests[i].expected.expectedString, evaluated->value.error.msg);
					FAIL();
				}
				break;

			case EXPECT_NULL:
				if (!testNullObject(evaluated)) {
					FAIL();
				}
				break;

			case EXPECT_ARRAY:
				if (!testIntegerArrayObject(evaluated, tests[i].expected.expectedArray)) {
					FAIL();
				}
				break;
		}
	}
}

TEST(TestEval, TestEval_13_ArrayLiterals) {

	const char input[] = "[1, 2 * 2, 3 + 3]";

	struct Object* evaluated = testEval(input);
	printf("After eval, type: %s\n", objectTypeToStr(evaluated->type));
	if (evaluated->type != OBJ_ARRAY) {
		printf("Object is not an array, got %s\n", objectTypeToStr(evaluated->type));
		FAIL();
	}

	if(evaluated->value.arr.size != 3) {
		printf("array has wrong number of elements. got=%llu", evaluated->value.arr.size);
		FAIL();
	}

	if(!testIntegerObject(evaluated->value.arr.objects[0], 1)) {
		FAIL();
	}

	if (!testIntegerObject(evaluated->value.arr.objects[1], 4)) {
		FAIL();
	}

	if (!testIntegerObject(evaluated->value.arr.objects[2], 6)) {
		FAIL();
	}
}

TEST(TestEval, TestEval_14_ArrayIndexExpr) {
	struct TestIndexExpression {
		char input[65];
		union {
			int64_t expectedInt;
			void* ptr;
		};
		//0 for int, 1 for string
		ExpectedType type;

	} tests[]{
		{
		"[1, 2, 3][0]",
		{.expectedInt = 1},
			EXPECT_INT
		},
		{
		"[1, 2, 3][1]",
					{.expectedInt = 2},
			EXPECT_INT
		},
		{
		"[1, 2, 3][2]",
				{.expectedInt = 3},
			EXPECT_INT,
		},
		{
		"let i = 0; [1][i];",
				{.expectedInt = 1},
			EXPECT_INT,
		},
		{
		"[1, 2, 3][1 + 1];",
				{.expectedInt = 3},
			EXPECT_INT,
		},
		{
		"let myArray = [1, 2, 3]; myArray[2];",
				{.expectedInt = 3},
			EXPECT_INT,
		},
		{
		"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];",
				{.expectedInt = 6},
			EXPECT_INT,
		},
		{
		"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]",
				{.expectedInt = 2},
			EXPECT_INT,
		},
		{
		"[1, 2, 3][3]",
					{.ptr = nullptr},
			EXPECT_NULL
		},
		{
		"[1, 2, 3][-1]",
				{.ptr = nullptr},
			EXPECT_NULL
		},
	};

	for (int i = 0; i < 10; i++) {
		struct Object* evaluated = testEval(tests[i].input);
		printf("After eval: type = %s\n", objectTypeToStr(evaluated->type));
		printf("%s\n", inspectObject(evaluated));
		switch (tests[i].type) {

		case EXPECT_INT:
			if (!testIntegerObject(evaluated, tests[i].expectedInt)) {
				FAIL();
			}
			break;

		case EXPECT_NULL:
			if(!testNullObject(evaluated)) {
				FAIL();
			}
		}
	}
}
