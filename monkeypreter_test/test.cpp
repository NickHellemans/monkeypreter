#include "gtest/gtest.h"

extern "C" {
#include "lexer/lexer.h"
#include "lexer/lexer.c"
#include "parser/parser.h"
#include "parser/parser.c"
#include "parser/ast.h"
#include "parser/ast.c"
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
		printf("expression not a boolean expr, got %d\n", expr->type);
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
	int64_t integer;
	bool boolean;
	char* string;
	Identifier ident;
	struct PrefixExpression prefix;
	struct InfixExpression infix;
	struct IfExpression ifelse;
} ExpectedValue;

bool testLiteralExpression(Expression* expr, ExpectedValue expected) {
	switch (expr->type) {
	case EXPR_INT: return testIntegerLiteral(expr, expected.integer);
	case EXPR_IDENT: return testIdentifier(expr, expected.ident.value);
	case EXPR_BOOL: return testBoolean(expr, expected.boolean);
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

TEST(TestLexer, TestNextToken_01) {
	const char* input = "=+(){},;";
	Lexer lexer = createLexer(input);

	constexpr Token expectedTokens[]{
		{TokenTypeAssign, "="},
		{TokenTypePlus, "+"},
		{TokenTypeLParen, "("},
		{TokenTypeRParen, ")"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeRSquirly, "}"},
		{TokenTypeComma, ","},
		{TokenTypeSemicolon, ";"},
		{TokenTypeEof, ""},
	};

	constexpr int expectedLength = 9;

	for (int i = 0; i < expectedLength; i++) {
		Token token = nextToken(&lexer);
		ASSERT_EQ(token.type, expectedTokens[i].type);
		ASSERT_STREQ(token.literal, expectedTokens[i].literal);
	}
}

TEST(TestLexer, TestNextToken_02) {
	const char* input = "let five = 5;"
		"let ten = 10;"
		"let add = fn(x, y) { x + y; };"
		"let result = add(five, ten);";

	Lexer lexer = createLexer(input);

	constexpr Token expectedTokens[]{
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "five"},
		{TokenTypeAssign, "="},
		{TokenTypeInt, "5"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "ten"},
		{TokenTypeAssign, "="},
		{TokenTypeInt, "10"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "add"},
		{TokenTypeAssign, "="},
		{TokenTypeFunction, "fn"},
		{TokenTypeLParen, "("},
		{TokenTypeIdent, "x"},
		{TokenTypeComma, ","},
		{TokenTypeIdent, "y"},
		{TokenTypeRParen, ")"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeIdent, "x"},
		{TokenTypePlus, "+"},
		{TokenTypeIdent, "y"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeRSquirly, "}"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "result"},
		{TokenTypeAssign, "="},
		{TokenTypeIdent, "add"},
		{TokenTypeLParen, "("},
		{TokenTypeIdent, "five"},
		{TokenTypeComma, ","},
		{TokenTypeIdent, "ten"},
		{TokenTypeRParen, ")"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeEof, ""},
	};

	constexpr int expectedLength = sizeof(expectedTokens) / sizeof(expectedTokens[0]);

	for (int i = 0; i < expectedLength; i++) {
		Token token = nextToken(&lexer);
		printf("Token: \n");
		printf("\tliteral: %s\n", token.literal);
		printf("\ttype: %d\n", token.type);
		printf("Expected Token: \n");
		printf("\tliteral: %s\n", expectedTokens[i].literal);
		printf("\ttype: %d\n", expectedTokens[i].type);
		printf("------------------------- \n");

		ASSERT_EQ(token.type, expectedTokens[i].type);
		ASSERT_STREQ(token.literal, expectedTokens[i].literal);
	}
}

TEST(TestLexer, TestNextToken_03) {
	const char* input = "let five = 5;"
		"let ten = 10;"
		"let add = fn(x, y) { x + y; };"
		"let result = add(five, ten);"
		"!-/*5;"
		"5 < 10 > 5;"
		"if (5 < 10) {"
		"return true;"
		"}"
		"else {"
		"return false;"
		"}"
		"10 == 10;"
		"10 != 9;";

	Lexer lexer = createLexer(input);

	constexpr Token expectedTokens[]{
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "five"},
		{TokenTypeAssign, "="},
		{TokenTypeInt, "5"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "ten"},
		{TokenTypeAssign, "="},
		{TokenTypeInt, "10"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "add"},
		{TokenTypeAssign, "="},
		{TokenTypeFunction, "fn"},
		{TokenTypeLParen, "("},
		{TokenTypeIdent, "x"},
		{TokenTypeComma, ","},
		{TokenTypeIdent, "y"},
		{TokenTypeRParen, ")"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeIdent, "x"},
		{TokenTypePlus, "+"},
		{TokenTypeIdent, "y"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeRSquirly, "}"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLet, "let"},
		{TokenTypeIdent, "result"},
		{TokenTypeAssign, "="},
		{TokenTypeIdent, "add"},
		{TokenTypeLParen, "("},
		{TokenTypeIdent, "five"},
		{TokenTypeComma, ","},
		{TokenTypeIdent, "ten"},
		{TokenTypeRParen, ")"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeBang, "!"},
		{TokenTypeMinus, "-"},
		{TokenTypeSlash, "/"},
		{TokenTypeAsterisk, "*"},
		{TokenTypeInt, "5"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeInt, "5"},
		{TokenTypeLT, "<"},
		{TokenTypeInt, "10"},
		{TokenTypeGT, ">"},
		{TokenTypeInt, "5"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeIf, "if"},
		{TokenTypeLParen, "("},
		{TokenTypeInt, "5"},
		{TokenTypeLT, "<"},
		{TokenTypeInt, "10"},
		{TokenTypeRParen, ")"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeReturn, "return"},
		{TokenTypeTrue, "true"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeRSquirly, "}"},
		{TokenTypeElse, "else"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeReturn, "return"},
		{TokenTypeFalse, "false"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeRSquirly, "}"},
		{TokenTypeInt, "10"},
		{TokenTypeEqual, "=="},
		{TokenTypeInt, "10"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeInt, "10"},
		{TokenTypeNotEqual, "!="},
		{TokenTypeInt, "9"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeEof, ""},
	};


	constexpr int expectedLength = sizeof(expectedTokens) / sizeof(expectedTokens[0]);

	for (int i = 0; i < expectedLength; i++) {
		Token token = nextToken(&lexer);
		printf("Token: \n");
		printf("\tliteral: %s\n", token.literal);
		printf("\ttype: %d\n", token.type);
		printf("Expected Token: \n");
		printf("\tliteral: %s\n", expectedTokens[i].literal);
		printf("\ttype: %d\n", expectedTokens[i].type);
		printf("------------------------- \n");

		ASSERT_EQ(token.type, expectedTokens[i].type);
		ASSERT_STREQ(token.literal, expectedTokens[i].literal);
	}
}


TEST(TestParser, TestParser_01_let) {
	const char* input = "let x = 5;"
		"let y = 10;"
		"let foobar = 838383;";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 3) {
		printf("Program does not contain 3 statements, got %llu\n", program->size);
		FAIL();
	}

	std::string expectedIdents[] = {"x", "y", "foobar"};
	for (int i = 0; i < 3; i++) {
		Statement stmt = program->statements[i];
		if (!testLetStatement(stmt, expectedIdents[i].c_str())) {
			printf("Test failed on statement: %d\n", i);
			FAIL();
		}
	}
}

TEST(TestParser, TestParser_02_ret) {
	const char* input = "return 5;"
		"return 10;"
		"return 993322;";

	Lexer lexer = createLexer(input);
	Parser parser = createParser(&lexer);

	Program* program = parseProgram(&parser);
	checkParserErrors(&parser);

	if (!program) {
		printf("Parser returned NULL\n");
		FAIL();
	}

	if (program->size != 3) {
		printf("Program does not contain 3 statements, got %llu\n", program->size);
		FAIL();
	}

	char* programStr = programToStr(program);
	printf("%s", programStr);
	free(programStr);

	for (int i = 0; i < 3; i++) {
		Statement stmt = program->statements[i];
		if (stmt.type != STMT_RETURN) {
			printf("Stmt not a return statement, got %d", stmt.type);
			continue;
		}

		if (strcmp(stmt.token.literal, "return") != 0) {
			printf("Stmt token literal not 'return', got %s", stmt.token.literal);
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
		//Add boolean tests
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
	};

	for (int i = 0; i < 22; i++) {
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

	ExpectedValue left;
	strcpy_s(left.ident.value, "x");
	ExpectedValue right;
	strcpy_s(right.ident.value, "y");

	if(!testInfixExpression(stmt.expr->ifelse.condition, left, OP_LT, right)) {
		FAIL();
	}

	BlockStatement* consequence = stmt.expr->ifelse.consequence;

	if(!consequence) {
		printf("Expected consequence block statement, got NULL\n");
		FAIL();
	}

	if(consequence->size != 1) {
		printf("Consequence does not contain 1 statement, got %llu\n", consequence->size);
		FAIL();
	}

	if(consequence->statements[0].type != STMT_EXPR) {
		printf("Statements[0] is not an expression statement, got %d", consequence->statements[0].type);
		FAIL();
	}

	if (!testIdentifier(consequence->statements[0].expr, "x")) {
		FAIL();
	}

	if(stmt.expr->ifelse.alternative) {
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

	ExpectedValue left;
	strcpy_s(left.ident.value, "x");
	ExpectedValue right;
	strcpy_s(right.ident.value, "y");

	if (!testInfixExpression(stmt.expr->ifelse.condition, left, OP_LT, right)) {
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