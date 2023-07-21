#pragma once
#include "../lexer/lexer.h"
#include "ast.h"

typedef struct SParser {
	Lexer* lexer;
	Token curToken;
	Token peekToken;

	//Parsing error handling
	size_t errorsLen;
	char** errors;

} Parser;

Parser createParser(Lexer* lexer);
void setParserNextToken(Parser* parser);

//Constructs AST returns head
Program* parseProgram(Parser* parser);
Statement parseStatement(Parser* parser);
Statement parseLetStatement(Parser* parser);
void peekError(Parser* parser, TokenType type);
bool expectPeek(Parser* parser, TokenType tokenType);
bool curTokenIs(Parser* parser, TokenType tokenType);
bool peekTokenIs(Parser* parser, TokenType tokenType);