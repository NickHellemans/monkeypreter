#include "gtest/gtest.h"

extern "C" {
	#include "parser/parser.h"
	//#include "parser/parser.c"
	#include "parser/ast.h"
	//#include "parser/ast.c"
}

bool testIntegerLiteral(Expression* expr, int64_t integerVal) {
	if (expr->type != EXPR_INT) {
		printf("Expression not an integer expr, got %d\n", expr->type);
		return false;
	}

	if (expr->integer != integerVal) {
		printf("Integer not %lld, got %lld\n", integerVal, expr->integer);
		return false;
	}

	char valAsStr[MAX_IDENT_LENGTH];
	int success = sprintf_s(valAsStr, MAX_IDENT_LENGTH, "%lld", integerVal);
	if (strcmp(expr->token.literal, valAsStr) != 0) {
		printf("Integer token literal not %lld, got %s", integerVal, expr->token.literal);
		return false;
	}

	return true;
}

bool testIdentifier(Expression* expr, const char* value) {
	if (expr->type != EXPR_IDENT) {
		printf("Expression not an ident expr, got %d\n", expr->type);
		return false;
	}

	if (strcmp(expr->ident.value, value) != 0) {
		printf("Ident value not %s, got %s\n", value, expr->ident.value);
		return false;
	}

	if (strcmp(expr->ident.token.literal, value) != 0) {
		printf("Ident token literal not %s, got %s", value, expr->ident.token.literal);
		return false;
	}

	return true;
}

bool testBoolean(Expression* expr, bool value) {
	if (expr->type != EXPR_BOOL) {
		printf("expression not a boolVal expr, got %d\n", expr->type);
		return false;
	}

	if (expr->boolean != value) {
		printf("Boolean value not %hhd, got %hhd\n", value, expr->boolean);
		return false;
	}

	const char* valAsStr = value ? "true" : "false";
	if (strcmp(expr->token.literal, valAsStr) != 0) {
		printf("Token literal not %s, got %s", valAsStr, expr->token.literal);
		return false;
	}

	return true;
}

typedef union {
	int64_t intValue;
	bool boolVal;
	char stringValue[MAX_IDENT_LENGTH];
} ExpectedValue;

bool testLiteralExpression(Expression* expr, ExpectedValue expected) {
	switch (expr->type) {
	case EXPR_INT: return testIntegerLiteral(expr, expected.intValue);
	case EXPR_IDENT: return testIdentifier(expr, expected.stringValue);
	case EXPR_BOOL: return testBoolean(expr, expected.boolVal);
	default:
		printf("Type of expr not handled, got %d", expr->type);
		return false;
	}
}

bool testInfixExpression(Expression* expr, ExpectedValue left, OperatorType op, ExpectedValue right) {
	if (expr->type != EXPR_INFIX) {
		printf("Expression not a infix expression, got %d\n", expr->type);
		return false;
	}

	if (!testLiteralExpression(expr->infix.left, left)) {
		return false;
	}

	if (expr->infix.operatorType != op) {
		printf("Operator is not %d, got %d\n", op, expr->infix.operatorType);
		return false;
	}

	if (!testLiteralExpression(expr->infix.right, right)) {
		return false;
	}

	return true;
}

bool testLetStatement(Statement stmt, const char* name) {
	if (strcmp(stmt.token.literal, "let") != 0) {
		printf("Token literal not 'let', got %s\n", stmt.token.literal);
		return false;
	}

	if (stmt.type != STMT_LET) {
		printf("Not a let statement, got %d", stmt.type);
		return false;
	}

	if (strcmp(stmt.identifier.value, name) != 0) {
		printf("Statement.Name.Value not '%s', got '%s'", name, stmt.identifier.value);
		return false;
	}

	if (strcmp(stmt.identifier.token.literal, name) != 0) {
		printf("Statement.Name.TokenLiteral not '%s', got '%s'", name, stmt.identifier.token.literal);
		return false;
	}

	return true;
}

void checkParserErrors(Parser* parser) {
	if (parser->errorsLen == 0)
		return;

	printf("Parser has %llu errors\n", parser->errorsLen);
	for (size_t i = 0; i < parser->errorsLen; i++) {
		printf("Parser error %llu: %s\n", i, parser->errors[i]);
	}
	FAIL();
}


TEST(TestParser, TestParser_01_let) {
	//const char* input = "let x = 5;"
	//	"let y = 10;"
	//	"let foobar = 838383;";

	struct LetTest {
		char input[16];
		char expectedIdentifier[MAX_IDENT_LENGTH];
		ExpectedValue expected;
	};

	LetTest letTests[]{
	{"let x = 5;", "x", {5}},
	{"let y = true;", "y", {.boolVal = true}},
	{"let foobar = y;", "foobar", {.stringValue = "y"}},
	};

	for (int i = 0; i < 3; i++) {

		printf("Starting test %d\n", i);
		Lexer lexer = createLexer(letTests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statements, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];

		if (!testLetStatement(stmt, letTests[i].expectedIdentifier)) {
			printf("Test failed on statement: %d\n", i);
			FAIL();
		}

		if (!testLiteralExpression(stmt.expr, letTests[i].expected)) {
			FAIL();
		}

	}
}

TEST(TestParser, TestParser_02_ret) {
	//const char* input = "return 5;"
	//	"return 10;"
	//	"return 993322;";

	struct RetTest {
		char input[16];
		ExpectedValue expected;
	};

	RetTest retTests[]{
	{"return 5;", {5}},
	{"return true;", {.boolVal = true}},
	{"return y;", {.stringValue = "y"}},
	};

	for (int i = 0; i < 3; i++) {

		Lexer lexer = createLexer(retTests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statement, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];
		if (stmt.type != STMT_RETURN) {
			printf("Stmt not a return statement, got %d", stmt.type);
			continue;
		}

		if (strcmp(stmt.token.literal, "return") != 0) {
			printf("Stmt token literal not 'return', got %s", stmt.token.literal);
			FAIL();
		}

		if (!testLiteralExpression(stmt.expr, retTests[i].expected)) {
			FAIL();
		}
	}


}

TEST(TestParser, TestParser_03_Ident) {
	const char* input = "foobar;";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	if (stmt.type != STMT_EXPR) {
		printf("Stmt not a expression statement, got %d", stmt.type);
		FAIL();
	}

	if (stmt.expr->type != EXPR_IDENT) {
		printf("Expression not a ident expression, got %d", stmt.type);
		FAIL();
	}

	if (strcmp(stmt.expr->ident.value, "foobar") != 0) {
		printf("Ident value not 'foobar', got %s", stmt.expr->ident.value);
		FAIL();
	}

	if (strcmp(stmt.expr->ident.token.literal, "foobar") != 0) {
		printf("Ident token literal not 'foobar', got %s", stmt.expr->ident.token.literal);
		FAIL();
	}
}

TEST(TestParser, TestParser_04_IntLiteral) {
	const char* input = "5;";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	if (stmt.type != STMT_EXPR) {
		printf("Stmt not a expression statement, got %d\n", stmt.type);
		FAIL();
	}

	if (stmt.expr->type != EXPR_INT) {
		printf("Expression not a integer expression, got %d\n", stmt.type);
		FAIL();
	}

	if (strcmp(stmt.expr->token.literal, "5") != 0) {
		printf("Ident token literal not '5', got %s\n", stmt.expr->ident.token.literal);
		FAIL();
	}

	if (stmt.expr->integer != 5) {
		printf("Expression value not '5', got %lld\n", stmt.expr->integer);
		FAIL();
	}
}


TEST(TestParser, TestParser_05_PrefixExpr) {
	struct prefixTest {
		char input[8];
		OperatorType operatorType;
		ExpectedValue expectedValue;
	};

	prefixTest prefixTests[]{
		{"!5;", OP_NEGATE, {5}},
		{"-15;", OP_SUBTRACT, {15}},
		{"!true;", OP_NEGATE, {true}},
		{"!false;", OP_NEGATE, {false}},
	};

	for (int i = 0; i < 4; i++) {
		Lexer lexer = createLexer(prefixTests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statement, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];
		if (stmt.type != STMT_EXPR) {
			printf("Stmt not a expression statement, got %d\n", stmt.type);
			FAIL();
		}

		if (stmt.expr->type != EXPR_PREFIX) {
			printf("Expression not a prefix expression, got %d\n", stmt.type);
			FAIL();
		}

		if (stmt.expr->prefix.operatorType != prefixTests[i].operatorType) {
			printf("Operator is not '%d', got %d\n", prefixTests[i].operatorType, stmt.expr->prefix.operatorType);
			FAIL();
		}

		if (!testLiteralExpression(stmt.expr->prefix.right, prefixTests[i].expectedValue)) {
			FAIL();
		}
	}
}

TEST(TestParser, TestParser_06_InfixExpr) {
	struct infixTest {
		char input[15];
		ExpectedValue leftValue;
		OperatorType operatorType;
		ExpectedValue rightValue;
	};

	infixTest infixTests[]{
		{"5 + 6;", {5}, OP_ADD, {6}},
		{"5 - 5;", {5}, OP_SUBTRACT, {5}},
		{"5 * 5;", {5}, OP_MULTIPLY, {5}},
		{"5 / 5;", {5}, OP_DIVIDE, {5}},
		{"5 > 5;", {5}, OP_GT, {5}},
		{"5 < 5;", {5}, OP_LT, {5}},
		{"5 == 5;", {5}, OP_EQ, {5}},
		{"5 != 5;", {5}, OP_NOT_EQ, {5}},
		//Add boolVal tests
		{"true == true", {true}, OP_EQ, {true}},
		{"true != false", {true}, OP_NOT_EQ, {false}},
		{"false == false", {false}, OP_EQ, {false}},
	};

	for (int i = 0; i < 11; i++) {
		Lexer lexer = createLexer(infixTests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statement, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];
		if (stmt.type != STMT_EXPR) {
			printf("Stmt not a expression statement, got %d\n", stmt.type);
			FAIL();
		}

		//if (stmt.expr->type != EXPR_INFIX) {
		//	printf("Expression not a infix expression, got %d\n", stmt.type);
		//	FAIL();
		//}

		//if (!testIntegerLiteral(stmt.expr->infix.left, infixTests[i].leftValue)) {
		//	FAIL();
		//}

		//if (stmt.expr->infix.operatorType != infixTests[i].operatorType) {
		//	printf("Operator is not '%d', got %d\n", infixTests[i].operatorType, stmt.expr->infix.operatorType);
		//	FAIL();
		//}


		//if (!testIntegerLiteral(stmt.expr->infix.right, infixTests[i].rightValue)) {
		//	FAIL();
		//}
		const ExpectedValue left = infixTests[i].leftValue;
		const ExpectedValue right = infixTests[i].rightValue;
		if (!testInfixExpression(stmt.expr, left, infixTests[i].operatorType, right)) {
			FAIL();
		}
	}
}


TEST(TestParser, TestParser_07_OperatorPrecedence) {
	struct precedenceTest {
		char input[50];
		char expected[50];
	};

	precedenceTest precedenceTests[]{

		{"-a * b", "((-a) * b)",},
		{
			"!-a",
			"(!(-a))",
		},
		{
			"a + b + c",
			"((a + b) + c)",
		},
		{
			"a + b - c",
			"((a + b) - c)",
		},
		{
			"a * b * c",
			"((a * b) * c)",
		},
		{
			"a * b / c",
			"((a * b) / c)",
		},
		{
			"a + b / c",
			"(a + (b / c))",
		},
		{
			"a + b * c + d / e - f",
			"(((a + (b * c)) + (d / e)) - f)",
		},
		{
			"3 + 4; -5 * 5",
			"(3 + 4)((-5) * 5)",
		},
		{
			"5 > 4 == 3 < 4",
			"((5 > 4) == (3 < 4))",
		},
		{
			"5 < 4 != 3 > 4",
			"((5 < 4) != (3 > 4))",
		},
		{
			"3 + 4 * 5 == 3 * 1 + 4 * 5",
			"((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
		},
		{
			"3 + 4 * 5 == 3 * 1 + 4 * 5",
			"((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
		},
		//Test with bools
		{
			"true",
			"true",
		},
		{
			"false",
			"false",
		},
		{
			"3 > 5 == false",
			"((3 > 5) == false)",
		},
		{
			"3 < 5 == true",
			"((3 < 5) == true)",
		},
		//Grouped expressions
		{
			"1 + (2 + 3) + 4",
			"((1 + (2 + 3)) + 4)",
		},
		{
			"(5 + 5) * 2",
			"((5 + 5) * 2)",
		},
		{
			"2 / (5 + 5)",
			"(2 / (5 + 5))",
		},
		{
			"-(5 + 5)",
			"(-(5 + 5))",
		},
		{
			"!(true == true)",
			"(!(true == true))",
		},
		//Call expressions
		{
		"a + add(b * c) + d",
		"((a + add((b * c))) + d)",
		},
		{
		"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
		"add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
		},
		{
		"add(a + b + c * d / f + g)",
		"add((((a + b) + ((c * d) / f)) + g))",
		},
		//Index expressions
		{
		"a * [1, 2, 3, 4][b * c] * d",
		"((a * ([1, 2, 3, 4][(b * c)])) * d)",
				},
		{
		"add(a * b[2], b[1], 2 * [1, 2][1])",
		"add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))",
		},
	};

	for (int i = 0; i < 27; i++) {
		printf("At %d iteration\n", i);
		printf("Input = %s\n", precedenceTests[i].input);
		printf("Expected = %s\n", precedenceTests[i].expected);
		Lexer lexer = createLexer(precedenceTests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}
		char* actual = programToStr(program);

		if (strcmp(actual, precedenceTests[i].expected) != 0) {
			printf("Expected %s, got %s", precedenceTests[i].expected, actual);
			FAIL();
		}
	}
}

TEST(TestParser, TestParser_08_Bool) {
	struct boolTest {
		char input[8];
		bool expected;
	};

	boolTest boolTests[]{
		{"true;", true},
		{"false;", false},
	};

	for (int i = 0; i < 2; i++) {
		Lexer lexer = createLexer(boolTests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statement, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];
		if (stmt.type != STMT_EXPR) {
			printf("Stmt not a expression statement, got %d", stmt.type);
			FAIL();
		}

		if (!testBoolean(stmt.expr, boolTests[i].expected)) {
			FAIL();
		}
	}
}


TEST(TestParser, TestParser_09_IfExpression) {

	const char* input = "if (x < y) { x }";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	if (stmt.type != STMT_EXPR) {
		printf("Stmt not a expression statement, got %d", stmt.type);
		FAIL();
	}

	if (stmt.expr->type != EXPR_IF) {
		printf("Expression not an IfExpression, got %d", stmt.expr->type);
		FAIL();
	}

	if (!testInfixExpression(stmt.expr->ifelse.condition, { .stringValue = {"x"} }, OP_LT, { .stringValue = {"y"} })) {
		FAIL();
	}

	BlockStatement* consequence = stmt.expr->ifelse.consequence;

	if (!consequence) {
		printf("Expected consequence block statement, got NULL\n");
		FAIL();
	}

	if (consequence->size != 1) {
		printf("Consequence does not contain 1 statement, got %llu\n", consequence->size);
		FAIL();
	}

	if (consequence->statements[0].type != STMT_EXPR) {
		printf("Statements[0] is not an expression statement, got %d", consequence->statements[0].type);
		FAIL();
	}

	if (!testIdentifier(consequence->statements[0].expr, "x")) {
		FAIL();
	}

	if (stmt.expr->ifelse.alternative) {
		printf("Alternative statement is not NULL, expected NULL");
		FAIL();
	}
}

TEST(TestParser, TestParser_10_IfElseExpression) {

	const char* input = "if (x < y) { x } else { y }";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	if (stmt.type != STMT_EXPR) {
		printf("Stmt not a expression statement, got %d", stmt.type);
		FAIL();
	}

	if (stmt.expr->type != EXPR_IF) {
		printf("Expression not an IfExpression, got %d", stmt.expr->type);
		FAIL();
	}

	if (!testInfixExpression(stmt.expr->ifelse.condition, { .stringValue = {"x"} }, OP_LT, { .stringValue = {"y"} })) {
		FAIL();
	}

	BlockStatement* consequence = stmt.expr->ifelse.consequence;

	if (!consequence) {
		printf("Expected consequence block statement, got NULL\n");
		FAIL();
	}

	if (consequence->size != 1) {
		printf("Consequence does not contain 1 statement, got %llu\n", consequence->size);
		FAIL();
	}

	if (consequence->statements[0].type != STMT_EXPR) {
		printf("Statements[0] is not an expression statement, got %d", consequence->statements[0].type);
		FAIL();
	}

	if (!testIdentifier(consequence->statements[0].expr, "x")) {
		FAIL();
	}

	BlockStatement* alternative = stmt.expr->ifelse.alternative;
	if (!alternative) {
		printf("Expected alternative block statement, got NULL");
		FAIL();
	}

	if (alternative->size != 1) {
		printf("Alternative does not contain 1 statement, got %llu\n", alternative->size);
		FAIL();
	}

	if (alternative->statements[0].type != STMT_EXPR) {
		printf("Statements[0] is not an expression statement, got %d", alternative->statements[0].type);
		FAIL();
	}

	if (!testIdentifier(alternative->statements[0].expr, "y")) {
		FAIL();
	}
}

TEST(TestParser, TestParser_11_FunctionLiteral) {

	const char* input = "fn(x, y) { x + y; }";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	if (stmt.type != STMT_EXPR) {
		printf("Stmt not a expression statement, got %d", stmt.type);
		FAIL();
	}

	if (stmt.expr->type != EXPR_FUNCTION) {
		printf("Expression not a function literal expression, got %d", stmt.expr->type);
		FAIL();
	}

	FunctionLiteral fn = stmt.expr->function;

	if (fn.parameters.size != 2) {
		printf("Function literal parameters wrong. want 2, got=%llu", fn.parameters.size);
		FAIL();
	}

	if (strcmp(fn.parameters.values[0].value, "x") != 0) {
		printf("Invalid parameter[0]: expected 'x', got %s", fn.parameters.values[0].value);
		FAIL();
	}

	if (strcmp(fn.parameters.values[1].value, "y") != 0) {
		printf("Invalid parameter[0]: expected 'y', got %s", fn.parameters.values[0].value);
		FAIL();
	}

	if (fn.body->size != 1) {
		printf("Function body statements not equal to 1, got %lld", fn.body->size);
		FAIL();
	}

	if (fn.body->statements[0].type != STMT_EXPR) {
		printf("function body statement is not an Expression statement, got %d", fn.body->statements[0].type);
		FAIL();
	}

	if (!testInfixExpression(fn.body->statements[0].expr, { .stringValue = {"x"} }, OP_ADD, { .stringValue = {"y"} })) {
		FAIL();
	}
}

TEST(TestParser, TestParser_12_FunctionParameters) {

	struct Test {
		char input[16];
		const char* expectedParams[3];
		size_t expectedSize;
	};

	const Test tests[] = {
		{"fn() {};", {}, 0},
		{"fn(x) {};", {"x"}, 1},
		{"fn(x, y, z) {};", {"x", "y", "z"}, 3},
	};

	for (int i = 0; i < 3; i++) {
		printf("Starting test: %d\n", i);
		Lexer lexer = createLexer(tests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statement, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];
		FunctionLiteral fn = stmt.expr->function;

		if (fn.parameters.size != tests[i].expectedSize) {
			printf("Length function literal parameters wrong. want %llu, got=%llu\n", tests[i].expectedSize, fn.parameters.size);
			FAIL();
		}

		for (size_t j = 0; j < tests[i].expectedSize; j++) {
			if (strcmp(fn.parameters.values[j].value, tests[i].expectedParams[j]) != 0) {
				printf("Invalid parameter: expected '%s', got %s\n", tests[i].expectedParams[j], fn.parameters.values[j].value);
				FAIL();
			}
		}
	}
}

TEST(TestParser, TestParser_13_CallExpression) {

	const char* input = "add(1, 2 * 3, 4 + 5);";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	if (stmt.type != STMT_EXPR) {
		printf("Stmt not a expression statement, got %d", stmt.type);
		FAIL();
	}

	if (stmt.expr->type != EXPR_CALL) {
		printf("Expression not a call expression, got %d", stmt.expr->type);
		FAIL();
	}

	CallExpression call = stmt.expr->call;

	if (!testIdentifier(call.function, "add")) {
		FAIL();
	}

	if (call.arguments.size != 3) {
		printf("Call expression parameters wrong: want 3, got=%llu", call.arguments.size);
		FAIL();
	}

	if (!testLiteralExpression(call.arguments.values[0], { 1 })) {
		FAIL();
	}

	if (!testInfixExpression(call.arguments.values[1], { 2 }, OP_MULTIPLY, { 3 })) {
		FAIL();
	}

	if (!testInfixExpression(call.arguments.values[2], { 4 }, OP_ADD, { 5 })) {
		FAIL();
	}

}


TEST(TestParser, TestParser_14_CallParameters) {

	struct Test {
		char input[16];
		const char* expectedParams[3];
		size_t expectedSize;
	};

	const Test tests[] = {
		{"empty();", {}, 0},
		{"add(x);", {"x"}, 1},
		{"max(x, y, z);", {"x", "y", "z"}, 3},
	};

	for (int i = 0; i < 3; i++) {
		printf("Starting test: %d\n", i);
		Lexer lexer = createLexer(tests[i].input);
		Parser parser = createParser(&lexer);

		Program* program = parseProgram(&parser);
		checkParserErrors(&parser);

		if (!program) {
			printf("Parser returned NULL\n");
			FAIL();
		}

		if (program->size != 1) {
			printf("Program does not contain 1 statement, got %llu\n", program->size);
			FAIL();
		}

		Statement stmt = program->statements[0];
		CallExpression call = stmt.expr->call;

		if (call.arguments.size != tests[i].expectedSize) {
			printf("Length call expression parameters wrong. want %llu, got=%llu\n", tests[i].expectedSize, call.arguments.size);
			FAIL();
		}

		for (size_t j = 0; j < call.arguments.size; j++) {
			if (strcmp(call.arguments.values[j]->ident.value, tests[i].expectedParams[j]) != 0) {
				printf("Invalid parameter: expected '%s', got %s\n", tests[i].expectedParams[j], call.arguments.values[j]->ident.value);
				FAIL();
			}
		}
	}
}

TEST(TestParser, TestParser_15_StringLiteral) {

	char input[] = "\"Hello World!\"";
	char expected[] = "Hello World!";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	Expression* literal = stmt.expr;

	if (literal->type != EXPR_STRING) {
		printf("Expr not a string literal, got %d\n", literal->type);
		FAIL();
	}

	if(strcmp(literal->string, expected) != 0) {
		printf("Literal value not %s, got %s", expected, literal->string);
		FAIL();
	}

}

TEST(TestParser, TestParser_16_ArrayLiteral) {

	char input[] = "[1, 2 * 2, 3 + 3]";
	char expected[] = "Hello World!";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	Expression* array = stmt.expr;

	if (array->type != EXPR_ARRAY) {
		printf("Expr not a array literal, got %d\n", array->type);
		FAIL();
	}

	if(array->array.elements.size != 3) {
		printf("Array size not = 3, got %llu\n", array->array.elements.size);
		FAIL();
	}

	if(!testIntegerLiteral(array->array.elements.values[0], 1)) {
		FAIL();
	}

	if (!testInfixExpression(array->array.elements.values[1], { 2 }, OP_MULTIPLY, {2})) {
		FAIL();
	}

	if (!testInfixExpression(array->array.elements.values[2], { 3 }, OP_ADD, { 3 })) {
		FAIL();
	}
}

TEST(TestParser, TestParser_17_IndexExpressions) {

	char input[] = "myArray[1 + 1]";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 1) {
		printf("Program does not contain 1 statement, got %llu\n", program->size);
		FAIL();
	}

	Statement stmt = program->statements[0];
	Expression* indexExpr = stmt.expr;

	if (indexExpr->type != EXPR_INDEX) {
		printf("Expr not a index expression, got %d\n", indexExpr->type);
		FAIL();
	}

	if (!testIdentifier(indexExpr->indexExpr.left, "myArray")) {
		FAIL();
	}

	if (!testInfixExpression(indexExpr->indexExpr.index, { 1 }, OP_ADD, { 1 })) {
		FAIL();
	}

}