#include "gtest/gtest.h"

extern "C" {
	#include "lexer/lexer.h"
	#include "lexer/lexer.c"
	#include "parser/parser.h"
	#include "parser/parser.c"
	#include "parser/ast.h"
	#include "parser/ast.c"
}

TEST(TestLexer, TestNextToken_01)
{
	const char* input = "=+(){},;";
	Lexer lexer = createLexer(input);

	constexpr Token expectedTokens[]{
		{TokenTypeAssign, "="},
		{TokenTypePlus, "+"},
		{TokenTypeLParen, "("},
		{TokenTypeRParen, ")"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeRSquirly, "}"},
		{TokenTypeComma,","},
		{TokenTypeSemicolon, ";"},
		{TokenTypeEof,  ""},
	};

	constexpr int expectedLength = 9;

	for (int i = 0; i < expectedLength; i++) {
		Token token = nextToken(&lexer);
		ASSERT_EQ(token.type, expectedTokens[i].type);
		ASSERT_STREQ(token.literal, expectedTokens[i].literal);
	}
}

TEST(TestLexer, TestNextToken_02)
{
	const char* input = "let five = 5;"
						"let ten = 10;"
						"let add = fn(x, y) { x + y; };"
						"let result = add(five, ten);";

	Lexer lexer = createLexer(input);

	constexpr Token expectedTokens[]{
		{TokenTypeLet, "let"},
		{TokenTypeIdent,  "five" },
		{TokenTypeAssign,  "=" },
		{TokenTypeInt,  "5" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeLet,  "let" },
		{TokenTypeIdent,  "ten" },
		{TokenTypeAssign,  "=" },
		{TokenTypeInt,  "10" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeLet,  "let" },
		{TokenTypeIdent,  "add" },
		{TokenTypeAssign,  "=" },
		{TokenTypeFunction,  "fn" },
		{TokenTypeLParen,  "(" },
		{TokenTypeIdent,  "x" },
		{TokenTypeComma,  "," },
		{TokenTypeIdent,  "y" },
		{TokenTypeRParen,  ")" },
		{TokenTypeLSquirly,  "{" },
		{TokenTypeIdent,  "x" },
		{TokenTypePlus,  "+" },
		{TokenTypeIdent,  "y" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeRSquirly,  "}" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeLet,  "let" },
		{TokenTypeIdent,  "result" },
		{TokenTypeAssign,  "=" },
		{TokenTypeIdent,  "add" },
		{TokenTypeLParen,  "(" },
		{TokenTypeIdent,  "five" },
		{TokenTypeComma,  "," },
		{TokenTypeIdent,  "ten" },
		{TokenTypeRParen,  ")" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeEof, "" },
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

TEST(TestLexer, TestNextToken_03)
{
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
		{TokenTypeIdent,  "five" },
		{TokenTypeAssign,  "=" },
		{TokenTypeInt,  "5" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeLet,  "let" },
		{TokenTypeIdent,  "ten" },
		{TokenTypeAssign,  "=" },
		{TokenTypeInt,  "10" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeLet,  "let" },
		{TokenTypeIdent,  "add" },
		{TokenTypeAssign,  "=" },
		{TokenTypeFunction,  "fn" },
		{TokenTypeLParen,  "(" },
		{TokenTypeIdent,  "x" },
		{TokenTypeComma,  "," },
		{TokenTypeIdent,  "y" },
		{TokenTypeRParen,  ")" },
		{TokenTypeLSquirly,  "{" },
		{TokenTypeIdent,  "x" },
		{TokenTypePlus,  "+" },
		{TokenTypeIdent,  "y" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeRSquirly,  "}" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeLet,  "let" },
		{TokenTypeIdent,  "result" },
		{TokenTypeAssign,  "=" },
		{TokenTypeIdent,  "add" },
		{TokenTypeLParen,  "(" },
		{TokenTypeIdent,  "five" },
		{TokenTypeComma,  "," },
		{TokenTypeIdent,  "ten" },
		{TokenTypeRParen,  ")" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeBang,  "!" },
		{TokenTypeMinus,  "-" },
		{TokenTypeSlash,  "/" },
		{TokenTypeAsterisk,  "*" },
		{TokenTypeInt,  "5" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeInt,  "5" },
		{TokenTypeLT,  "<" },
		{TokenTypeInt,  "10" },
		{TokenTypeGT,  ">" },
		{TokenTypeInt,  "5" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeIf,  "if" },
		{TokenTypeLParen,  "(" },
		{TokenTypeInt,  "5" },
		{TokenTypeLT,  "<" },
		{TokenTypeInt,  "10" },
		{TokenTypeRParen,  ")" },
		{TokenTypeLSquirly,  "{" },
		{TokenTypeReturn,  "return" },
		{TokenTypeTrue,  "true" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeRSquirly,  "}" },
		{TokenTypeElse,  "else" },
		{TokenTypeLSquirly,  "{" },
		{TokenTypeReturn,  "return" },
		{TokenTypeFalse,  "false" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeRSquirly,  "}" },
		{TokenTypeInt,  "10" },
		{TokenTypeEqual,  "==" },
		{TokenTypeInt,  "10" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeInt,  "10" },
		{TokenTypeNotEqual,  "!=" },
		{TokenTypeInt,  "9" },
		{TokenTypeSemicolon,  ";" },
		{TokenTypeEof, "" },
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

bool testLetStatement(Statement stmt, const char* name) {

	if(strcmp(stmt.token.literal, "let") != 0) {
		printf("Token literal not 'let', got %s\n", stmt.token.literal);
		return false;
	}

	if(stmt.type != STMT_LET) {
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

TEST(TestParser, TestParser_01_let)
{
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

	std::string expectedIdents[] = { "x", "y", "foobar" };
	for (int i = 0; i < 3; i++) {
		Statement stmt = program->statements[i];
		if (!testLetStatement(stmt, expectedIdents[i].c_str())) {
			printf("Test failed on statement: %d\n", i);
			FAIL();
		}
	}
}

TEST(TestParser, TestParser_02_ret)
{
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
		if(stmt.type != STMT_RETURN) {
			printf("Stmt not a return statement, got %d", stmt.type);
			continue;
		}

		if(strcmp(stmt.token.literal, "return") != 0 ) {
			printf("Stmt token literal not 'return', got %s", stmt.token.literal);
			FAIL();
		}

	}
}

TEST(TestParser, TestParser_03_Ident)
{
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

	if (stmt.expr.type != EXPR_IDENT) {
		printf("Expression not a ident expression, got %d", stmt.type);
		FAIL();
	}

	if (strcmp(stmt.expr.ident.value, "foobar") != 0) {
		printf("Ident value not 'foobar', got %s", stmt.expr.ident.value);
		FAIL();
	}

	if (strcmp(stmt.expr.ident.token.literal, "foobar") != 0) {
		printf("Ident token literal not 'foobar', got %s", stmt.expr.ident.token.literal);
		FAIL();
	}
}

TEST(TestParser, TestParser_04_IntLiteral)
{
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

	if (stmt.expr.type != EXPR_INT) {
		printf("Expression not a integer expression, got %d\n", stmt.type);
		FAIL();
	}

	if (strcmp(stmt.expr.token.literal, "5") != 0) {
		printf("Ident token literal not '5', got %s\n", stmt.expr.ident.token.literal);
		FAIL();
	}

	if (stmt.expr.integer != 5) {
		printf("Expression value not '5', got %lld\n", stmt.expr.integer);
		FAIL();
	}
}