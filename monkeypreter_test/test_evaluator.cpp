#include "gtest/gtest.h"


extern "C" {
	#include "parser/parser.h"
	#include "parser/parser.c"
	#include "parser/ast.h"
	#include "parser/ast.c"
	#include "interpreter/object.h"
	#include "interpreter/object.c"
}

struct Object testEval(char* input) {
	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);
	Program* program = parseProgram(&parser);
	const struct Object obj = evalProgram(program);
	freeProgram(program);
	freeParser(&parser);
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
		printf("Starting test %d\n", i);
		struct Object evaluated = testEval(tests[i].input);
		printf("Evaluated: %s\n", inspectObject(&evaluated));
		if (!testBooleanObject(&evaluated, tests[i].expected)) {
			FAIL();
		}
		printf("Ended test %d\n\n", i);
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
	};

	for (int i = 0; i < 5; i++) {
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