#pragma once

#include <stdlib.h>
#include <stdbool.h>

#define MAX_IDENT_LENGTH 64
#define MAX_STRING_LENGTH 1024

typedef enum
{
	TokenTypeIllegal,
	TokenTypeEof,
	TokenTypeIdent,
	TokenTypeInt,
	TokenTypeFunction,
	TokenTypeLet,
	TokenTypeTrue,
	TokenTypeFalse,
	TokenTypeIf,
	TokenTypeElse,
	TokenTypeReturn,
	TokenTypeAssign,
	TokenTypePlus,
	TokenTypeMinus,
	TokenTypeBang,
	TokenTypeAsterisk,
	TokenTypeSlash,
	TokenTypeLT,
	TokenTypeGT,
	TokenTypeEqual,
	TokenTypeNotEqual,
	TokenTypeComma,
	TokenTypeSemicolon,
	TokenTypeLParen,
	TokenTypeRParen,
	TokenTypeLSquirly,
	TokenTypeRSquirly,
	TokenTypeString,
} TokenType;

typedef struct Token
{
	TokenType type;
	char literal[MAX_IDENT_LENGTH];
} Token;

//Token* tokenCreate(TokenType type, char* literal);
//void tokenCleanUp(Token** token);

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
static void getIdentType(Token* t);
void readNumber(Lexer* lexer, Token* token);
char peekChar(const Lexer* lexer);
const char* tokenTypeToStr(TokenType type);
void readString(Lexer* lexer, char* str);	