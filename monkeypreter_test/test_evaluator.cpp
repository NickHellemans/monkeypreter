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
		printf("Object is not an integer, expected %s, got %s", objectTypeToStr(OBJ_INT), objectTypeToStr(obj.type));
		return false;
	}
	if(obj.value.integer != expected) {
		printf("Object has wrong value, expected %lld, got %lld", expected, obj.value.integer);
		return false;
	}

	return true;
}

TEST(TestEval, TestEval_01_IntegerExpr) {
	struct TestInteger {
		char input[10];
		int64_t expected;
	} tests[2]{
		{"5", 5},
		{"10", 10},
	};

	for(int i = 0; i < 2; i++) {
		const struct Object evaluated = testEval(tests[i].input);
		if(!testIntegerObject(evaluated, tests[i].expected)) {
			FAIL();
		}
	}

}