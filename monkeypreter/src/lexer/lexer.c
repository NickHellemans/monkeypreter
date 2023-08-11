#include "lexer.h"
#include <ctype.h>
#include <string.h>

void readChar(Lexer* lexer);
void readIdentifier(Lexer* lexer, Token* token);
bool isLetter(char ch);
void readNumber(Lexer* lexer, Token* token);
char peekChar(const Lexer* lexer);
void readString(Lexer* lexer, char* str);

Lexer createLexer(const char* input)
{
	Lexer l = { input, strlen(input),-1, 0, input[0] };
	readChar(&l);
	return l;
}

void skipWhiteSpace(Lexer* lexer) {
	char ch = lexer->ch;
	while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
	{
		readChar(lexer);
		ch = lexer->ch;
	}
}

Token nextToken(Lexer* lexer) {
	Token token;
	token.type = TokenTypeIllegal;
	token.literal[0] = '\0';

	skipWhiteSpace(lexer);

	switch (lexer->ch)
	{
		case '=':
			if(peekChar(lexer) == '=') {
				token.type = TokenTypeEqual;
				token.literal[0] = lexer->ch;
				readChar(lexer);
				token.literal[1] = lexer->ch;
				token.literal[2] = '\0';
			}
			else {
				token.type = TokenTypeAssign;
				token.literal[0] = lexer->ch;
				token.literal[1] = '\0';
			}
			break;
		case '-':
			token.type = TokenTypeMinus;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '!':
			if(peekChar(lexer) == '=') {
				token.type = TokenTypeNotEqual;
				token.literal[0] = lexer->ch;
				readChar(lexer);
				token.literal[1] = lexer->ch;
				token.literal[2] = '\0';
			}
			else {
				token.type = TokenTypeBang;
				token.literal[0] = lexer->ch;
				token.literal[1] = '\0';
			}
			break;
		case '/':
			token.type = TokenTypeSlash;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '*':
			token.type = TokenTypeAsterisk;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '<':
			token.type = TokenTypeLT;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '>':
			token.type = TokenTypeGT;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case ';':
			token.type = TokenTypeSemicolon;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '(':
			token.type = TokenTypeLParen;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case ')':
			token.type = TokenTypeRParen;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case ',':
			token.type = TokenTypeComma;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '+':
			token.type = TokenTypePlus;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '{':
			token.type = TokenTypeLSquirly;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '}':
			token.type = TokenTypeRSquirly;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case '"':
			token.type = TokenTypeString;
			readString(lexer, token.literal);
			break;
		case '[':
			token.type = TokenTypeLBracket;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case ']':
			token.type = TokenTypeRBracket;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;

		case ':':
			token.type = TokenTypeColon;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
		case 0:
			token.type = TokenTypeEof;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;

		default:
			if(isLetter(lexer->ch))
			{
				readIdentifier(lexer, &token);
				return token;
			}

			if (isdigit(lexer->ch)) {
				readNumber(lexer, &token);
				return token;
			}

			token.type = TokenTypeIllegal;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;
	}

	readChar(lexer);
	return token;
}

void readChar(Lexer* lexer)
{
	if (lexer->readPosition >= lexer->inputLength)
	{
		lexer->ch = 0;
	}
	else
	{
		lexer->ch = lexer->input[lexer->readPosition];
	}

	lexer->position = (int) lexer->readPosition;
	lexer->readPosition++;
}

//Fill literal in token
void readIdentifier(Lexer* lexer, Token* token) {
	//Set literal
	size_t i = 0;
	while(isLetter(lexer->ch) && i < MAX_IDENT_LENGTH - 1)
	{
		token->literal[i] = lexer->ch;
		i++;
		readChar(lexer);
	}
	token->literal[i] = '\0';
	//Sets type
	getIdentType(token);
}

//Fill literal in token number
void readNumber(Lexer* lexer, Token* token) {
	//Set literal
	size_t i = 0;
	while (isdigit(lexer->ch) && i < MAX_IDENT_LENGTH - 1)
	{
		token->literal[i] = lexer->ch;
		i++;
		readChar(lexer);
	}
	token->literal[i] = '\0';

	//Sets type
	token->type = TokenTypeInt;
}

bool isLetter(const char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
}

char peekChar(const Lexer* lexer) {

	if(lexer->readPosition >= lexer->inputLength)
		return 0;

	return lexer->input[lexer->readPosition];
}

void readString(Lexer* lexer, char* str) {
	int index = 0;
	while(true) {
		readChar(lexer);
		if (lexer->ch == '"' || lexer->ch == 0)
			break;

		str[index] = lexer->ch;
		index++;
	}

	str[index] = '\0';
}