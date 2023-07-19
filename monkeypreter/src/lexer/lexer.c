#include "lexer.h"

#include <ctype.h>
#include <string.h>

Lexer createLexer(const char* input)
{
	Lexer l = { input, strlen(input),-1, 0, input[0] };
	readChar(&l);
	return l;
}

Token nextToken(Lexer* lexer) {
	Token token;
	token.type = TokenTypeIllegal;
	token.literal[0] = '\0';

	switch (lexer->ch)
	{
		case '=':
			token.type = TokenTypeAssign;
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
		case 0:
			token.type = TokenTypeEof;
			token.literal[0] = lexer->ch;
			token.literal[1] = '\0';
			break;

		default:
			if(isalpha(lexer->ch))
			{
				readIdentifier(lexer, &token);
				return token;
			}
			else
			{
				token.type = TokenTypeIllegal;
				token.literal[0] = lexer->ch;
				token.literal[1] = '\0';
			}
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

	lexer->position = lexer->readPosition;
	lexer->readPosition++;
}

//Fill literal in token
void readIdentifier(Lexer* lexer, Token* token) {
	size_t i = 0;
	while(isLetter(lexer->ch))
	{
		token->literal[i] = lexer->ch;
		i++;
		readChar(lexer);
	}
	token->literal[i] = '\0';
	token->type = TokenTypeIdent;
}

bool isLetter(const char ch) {
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
}