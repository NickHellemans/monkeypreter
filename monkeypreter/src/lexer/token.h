#pragma once

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
	TokenTypeLBracket,
	TokenTypeRBracket,
	TokenTypeColon,
} TokenType;

typedef struct Token
{
	TokenType type;
	char literal[MAX_IDENT_LENGTH];
} Token;

const char* tokenTypeToStr(TokenType type);
void getIdentType(Token* t);