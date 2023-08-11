#pragma once
#include "../lexer/lexer.h"
#include "ast.h"

enum Precedence {
	LOWEST = 1,
	EQUALS,         // ==
	LESSGREATER,    // < or >
	SUM,            // - and +
	PRODUCT,        // * and /
	PREFIX,         // - or !x
	CALL,           // fn()
	INDEX,			//arr[0]
};

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
//Constructs AST, returns head for eval phase 
Program* parseProgram(Parser* parser);
void freeParser(Parser* parser);