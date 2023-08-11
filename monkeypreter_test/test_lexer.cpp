#include "gtest/gtest.h"

extern "C" {
	#include "lexer/lexer.h"
	#include "lexer/lexer.c"
	#include "lexer/token.h"
	#include "lexer/token.c"
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
		"10 != 9;"
		"\"foobar\""
		"\"foo bar\""
		"[1, 2];"
		"{\"foo\": \"bar\"}"
	;

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
		{TokenTypeString, "foobar"},
		{TokenTypeString, "foo bar"},
		{TokenTypeLBracket, "["},
		{TokenTypeInt, "1"},
		{TokenTypeComma, ","},
		{TokenTypeInt, "2"},
		{TokenTypeRBracket, "]"},
		{TokenTypeSemicolon, ";"},
		{TokenTypeLSquirly, "{"},
		{TokenTypeString, "foo"},
		{TokenTypeColon, ":"},
		{TokenTypeString, "bar"},
		{TokenTypeRSquirly, "}"},

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
