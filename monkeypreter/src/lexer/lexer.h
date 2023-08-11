#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "token.h"

typedef struct Lexer {
	const char* input;
	size_t inputLength;
	int position; //current position (points to current char)
	size_t readPosition; //current reading position 
	char ch; //current char
} Lexer;

Lexer createLexer(const char* input);
Token nextToken(Lexer* lexer);