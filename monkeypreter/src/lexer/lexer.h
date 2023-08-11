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
void readChar(Lexer* lexer);
void readIdentifier(Lexer* lexer, Token* token);
bool isLetter(char ch);
void readNumber(Lexer* lexer, Token* token);
char peekChar(const Lexer* lexer);
void readString(Lexer* lexer, char* str);	