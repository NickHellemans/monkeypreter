#include "parser.h"

#include <stdio.h>
#include <string.h>

Parser createParser(Lexer* lexer) {
	Parser parser;
	parser.lexer = lexer;
	parser.errorsLen = 0;
	parser.errors = (char**) malloc(1000000);

	//Read 2 tokens so cur & peek are set
	setParserNextToken(&parser);
	setParserNextToken(&parser);

	return parser;
}

void setParserNextToken(Parser* parser) {
	parser->curToken = parser->peekToken;
	parser->peekToken = nextToken(parser->lexer);
}

Program* parseProgram(Parser* parser) {
	Program* program = (Program*) malloc(sizeof(Statement*) + 2 * sizeof(size_t));
	program->statements = (Statement*) malloc(1000000);
	program->size = 0;
	program->cap = 0;

	while (!curTokenIs(parser, TokenTypeEof)) {
		Statement stmt = parseStatement(parser);
		if (stmt.type != STMT_ILLEGAL) {
			program->statements[program->size] = stmt;
			program->size++;
		}
		setParserNextToken(parser);
	}
	return program;
}

Statement parseStatement(Parser* parser) {
	Statement stmt;
	switch (parser->curToken.type) {

		case TokenTypeLet : 
			return parseLetStatement(parser);
		case TokenTypeReturn:
			return parseRetStatement(parser);
		default:
			stmt.type = STMT_ILLEGAL;
			return stmt;
	}
}

Statement parseLetStatement(Parser* parser) {
	Statement stmt;
	stmt.type = STMT_LET;
	stmt.token = parser->curToken;

	if(!expectPeek(parser, TokenTypeIdent)) {
		stmt.type = STMT_ILLEGAL;
		return stmt;
	}

	Identifier ident;
	ident.token = parser->curToken;
	strcpy_s(ident.value, MAX_IDENT_LENGTH, parser->curToken.literal);
	stmt.identifier = ident;

	if(!expectPeek(parser, TokenTypeAssign)) {
		stmt.type = STMT_ILLEGAL;
		return stmt;
	}

	//Skip expression until ; 
	while(!curTokenIs(parser, TokenTypeSemicolon)) {
		setParserNextToken(parser);
	}

	return stmt;
}

Statement parseRetStatement(Parser* parser) {
	Statement stmt;
	stmt.type = STMT_RETURN;
	stmt.token = parser->curToken;

	setParserNextToken(parser);

	//Skip expr
	while(!curTokenIs(parser, TokenTypeSemicolon)) {
		setParserNextToken(parser);
	}
	return stmt;
}
void peekError(Parser* parser, TokenType type) {
	char* msg = (char*) malloc(128 * sizeof(char));
	int success = sprintf_s(msg, 128 * sizeof(char), "expected next token to be: %d, got: %d instead", type, parser->peekToken.type);

	parser->errors[parser->errorsLen] = msg;
	parser->errorsLen++;
}

bool expectPeek(Parser* parser, TokenType tokenType) {

	if(peekTokenIs(parser, tokenType)) {
		setParserNextToken(parser);
		return true;
	} else {
		peekError(parser, tokenType);
		return false;
	}
	
}
bool curTokenIs(Parser* parser, TokenType tokenType) {
	return parser->curToken.type == tokenType;
}
bool peekTokenIs(Parser* parser, TokenType tokenType) {
	return parser->peekToken.type == tokenType;
}