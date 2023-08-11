#include "token.h"
#include <string.h>

const char* tokenTypeToStr(TokenType type) {
	static const char* tokenNames[] = {
		"ILLEGAL", "EOF", "IDENT", "INT", "FUNCTION", "LET", "TRUE",
		"FALSE",   "IF",  "ELSE", "RETURN",	"=",  "+",
		"-", "!", "*",  "/",
		"<",  ">",    "==",   "!=", ",",
		";", "(", ")",  "{", "}", "STRING", "[", "]", ":",

	};
	return tokenNames[type];
}

//Get type based on literal
void getIdentType(Token* t) {

	if (strcmp(t->literal, "let") == 0) {
		t->type = TokenTypeLet;
	}
	else if (strcmp(t->literal, "fn") == 0) {
		t->type = TokenTypeFunction;
	}
	else if (strcmp(t->literal, "true") == 0) {
		t->type = TokenTypeTrue;
	}
	else if (strcmp(t->literal, "false") == 0) {
		t->type = TokenTypeFalse;
	}
	else if (strcmp(t->literal, "if") == 0) {
		t->type = TokenTypeIf;
	}
	else if (strcmp(t->literal, "else") == 0) {
		t->type = TokenTypeElse;
	}
	else if (strcmp(t->literal, "return") == 0) {
		t->type = TokenTypeReturn;
	}
	else
	{
		t->type = TokenTypeIdent;
	}
}