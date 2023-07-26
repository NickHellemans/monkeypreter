#pragma once
#include "../lexer/lexer.h"
#include "ast.h"

typedef struct SParser {
	Lexer* lexer;
	Token curToken;
	Token peekToken;

	//Parsing error handling
	size_t errorsLen;
	size_t errorsCap;
	char** errors;

} Parser;

Parser createParser(Lexer* lexer);
//Constructs AST, returns head
Program* parseProgram(Parser* parser);
void freeParser(Parser* parser);