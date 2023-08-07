#include "gtest/gtest.h"

extern "C" {
	#include "parser/parser.h"
	#include "parser/parser.c"
	#include "parser/ast.h"
	#include "parser/ast.c"
	#include "interpreter/object.h"
	#include "interpreter/object.c"
	#include "interpreter/environment.h"
	#include "interpreter/environment.c"
	#include "interpreter/hash_map.h"
	#include "interpreter/hash_map.c"
}

struct Object testEval(const char* input) {
	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);
	Program* program = parseProgram(&parser);
	struct ObjectEnvironment* env = newEnvironment();
	const struct Object obj = evalProgram(program, env);
	//freeProgram(program);
	freeParser(&parser);
	deleteEnvironment(env);
	return obj;
}

bool testIntegerObject(struct Object obj, int64_t expected) {

	if(obj.type != OBJ_INT) {
		printf("Object is not an integer, expected %s, got %s\n", objectTypeToStr(OBJ_INT), objectTypeToStr(obj.type));
		return false;
	}
	if(obj.value.integer != expected) {
		printf("Object has wrong value, expected %lld, got %lld\n", expected, obj.value.integer);
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

bool testNullObject(const struct Object obj) {

	if(obj.type != OBJ_NULL) {
		printf("Object not NULL, got %s", objectTypeToStr(obj.type));
		return false;
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
		const struct Object evaluated = testEval(tests[i].input);
		if(!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
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
		const struct Object evaluated = testEval(tests[i].input);
		if (!testBooleanObject(&evaluated, tests[i].expected)) {
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
		struct Object evaluated = testEval(tests[i].input);
		if (!testBooleanObject(&evaluated, tests[i].expected)) {
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
		struct Object evaluated = testEval(tests[i].input);

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
		struct Object evaluated = testEval(tests[i].input);
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
		struct Object evaluated = testEval(tests[i].input);
		if(evaluated.type != OBJ_ERROR) {
			printf("No error object returned, got %s\n", objectTypeToStr(evaluated.type));
			FAIL();
		}

		if(strcmp(evaluated.value.error.msg, tests[i].expected) != 0) {
			printf("Wrong error message. Expected: %s, got %s\n", tests[i].expected, evaluated.value.error.msg);
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
		struct Object evaluated = testEval(tests[i].input);
		printf("Here after eval: type = %d, value = %llu\n", evaluated.type, evaluated.value.integer);
		if (!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
		printf("End test %d\n", i);
	}
}

TEST(TestEval, TestEval_08_FunctionObject) {

	char input[] = "fn(x) { x + 2; };";
	struct Object evaluated = testEval(input);
	if(evaluated.type != OBJ_FUNCTION) {
		printf("object is not a function, got %d\n", evaluated.type);
		FAIL();
	}

	FunctionObject func = evaluated.value.function;
	if(func.parameters.size != 1) {
		printf("function has wrong parameters length, expected 1, got %llu\n", func.parameters.size);
		FAIL();
	}

	if(strcmp(func.parameters.values[0].value, "x") != 0) {
		printf("Parameter is not 'x', got %s\n", func.parameters.values[0].value);
		FAIL();
	}

	char expectedBody[] = "(x + 2)";

	char* body = (char*)malloc(MAX_PROGRAM_LEN);
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

	for (int i = 0; i < 6; i++) {
		struct Object evaluated = testEval(tests[i].input);
		if (!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
	}
}

TEST(TestEval, TestEval_10_Closures) {
	
	const char* input = "let newAdder = fn(x) {"
						"fn(y) { x + y };"
					 "};"

				     "let addTwo = newAdder(2);"
				     "addTwo(2);";

	struct Object evaluated = testEval(input);
	if (!testIntegerObject(evaluated, 4)) {
		FAIL();
	}
	
}

TEST(TestEval, TestEval_10_Strings) {

	char input[] = "\"Hello World!\"";
	char expected[] = "Hello World!";

	struct Object evaluated = testEval(input);
	if(evaluated.type != OBJ_STRING) {
		printf("Object is not a string, got %s\n", objectTypeToStr(evaluated.type));
		FAIL();
	}

	if(strcmp(evaluated.value.string, expected) != 0) {
		printf("String has wrong value, expected: %s, got %s\n", expected, evaluated.value.string);
		FAIL();
	}

}

TEST(TestEval, TestEval_11_StringConcat) {

	const char input[] = R"("Hello" + " " + "World!")";
	char expected[] = "Hello World!";

	struct Object evaluated = testEval(input);
	if (evaluated.type != OBJ_STRING) {
		printf("Object is not a string, got %s\n", objectTypeToStr(evaluated.type));
		FAIL();
	}

	if (strcmp(evaluated.value.string, expected) != 0) {
		printf("String has wrong value, expected: %s, got %s\n", expected, evaluated.value.string);
		FAIL();
	}

}

enum ExpectedType {EXPECT_INT, EXPECT_STRING};

TEST(TestEval, TestEval_12_BuiltinFunctions) {
	struct TestInteger {
		char input[55];
		union {
			int64_t expectedInt;
			char expectedString[128];
		};
		//0 for int, 1 for string
		ExpectedType type;

	} tests[]{
		{"len(\"\")", {.expectedInt = 0}, EXPECT_INT},
		{"len(\"four\")", {.expectedInt = 4}, EXPECT_INT},
		{"len(\"hello world\")", {.expectedInt = 11}, EXPECT_INT},
		{"len(1)", {.expectedString = "argument to `len` not supported, got INTEGER"}, EXPECT_STRING},
		{R"(len("one", "two"))", {.expectedString = "wrong number of arguments. got=2, want=1"}, EXPECT_STRING},
	};

	for (int i = 0; i < 5; i++) {
		struct Object evaluated = testEval(tests[i].input);
		switch (tests[i].type) {
			case EXPECT_INT:
				if (!testIntegerObject(evaluated, tests[i].expectedInt)) {
						FAIL();
					}
					break;

			case EXPECT_STRING:
				if(evaluated.type != OBJ_ERROR) {
					printf("Object is not an error, got %s", objectTypeToStr(evaluated.type));
					FAIL();
				}

				if(strcmp(evaluated.value.error.msg, tests[i].expectedString) != 0) {
					printf("Wrong error msg, expected: %s, got: %s\n", tests[i].expectedString, evaluated.value.error.msg);
					FAIL();
				}
				break;
		}
	}
}