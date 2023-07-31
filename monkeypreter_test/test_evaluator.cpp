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

TEST(TestEval, TestEval_01_IntegerExpr) {
	struct TestInteger {
		char input[10];
		int64_t expected;
	} tests[]{
		{"5", 5},
		{"10", 10},
		{"-5", -5},
		{"-10", -10},
	};

	for(int i = 0; i < 4; i++) {
		const struct Object evaluated = testEval(tests[i].input);
		if(!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
	}

}

TEST(TestEval, TestEval_02_BoolExpr) {
	struct TestInteger {
		char input[10];
		int64_t expected;
	} tests[2]{
		{"true", true},
		{"false", false},
	};

	for (int i = 0; i < 2; i++) {
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