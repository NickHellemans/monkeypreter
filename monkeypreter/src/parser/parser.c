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
Expression* createExpression(enum ExpressionType type, Token token) {
	Expression* expr = (Expression*) malloc(sizeof(Expression));
	if (!expr) {
		perror("OUT OF MEMORY");
	}
	expr->type = type;
	expr->token = token;
	return expr;
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
	switch (parser->curToken.type) {
		case TokenTypeLet : 
			return parseLetStatement(parser);
		case TokenTypeReturn:
			return parseRetStatement(parser);
		default:
			return parseExprStatement(parser);
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
	stmt.expr = createExpression(EXPR_INT, parser->curToken);

	//Skip expr
	while(!curTokenIs(parser, TokenTypeSemicolon)) {
		setParserNextToken(parser);
	}
	return stmt;
}

Statement parseExprStatement(Parser* parser) {
	Statement stmt;
	stmt.type = STMT_EXPR;
	stmt.token = parser->curToken;
	stmt.expr = parseExpr(parser, LOWEST);

	if (peekTokenIs(parser, TokenTypeSemicolon))
		setParserNextToken(parser);

	return stmt;
}

Expression* parseExpr(Parser* parser, enum Precedence precedence) {
	Expression* leftExpr = NULL;
	switch (parser->curToken.type) {
		case TokenTypeIdent:
			leftExpr = parseIdentExpr(parser);
			break;
		case TokenTypeInt:
			leftExpr = parseIntegerLiteralExpr(parser);
			break;

		case TokenTypeBang:
		case TokenTypeMinus:
			leftExpr = parsePrefixExpr(parser);
			break;
	}

	return leftExpr;
}

Expression* parseIdentExpr(Parser* parser) {
	Expression* expr = createExpression(EXPR_IDENT, parser->curToken);
	expr->ident.token = expr->token;
	strcpy_s(expr->ident.value, MAX_IDENT_LENGTH, parser->curToken.literal);
	return expr;
}

Expression* parseIntegerLiteralExpr(Parser* parser) {
	Expression* expr = createExpression(EXPR_INT, parser->curToken);
	expr->integer = 0;
	const char* s = expr->token.literal;
	for (int i = 0; s[i] != '\0'; i++) {
		expr->integer = expr->integer * 10 + (s[i] - 48);
	}
	return expr;
}

Expression* parsePrefixExpr(Parser* parser) {
	Expression* expr = createExpression(EXPR_PREFIX, parser->curToken);
	expr->prefix.operatorType = parseOperator(expr->token.type);
	setParserNextToken(parser);
	printf("In parsePrefixExpr, token after: %s is %s\n", expr->token.literal, parser->curToken.literal);
	printf("Type = %d", parser->curToken.type);
	expr->prefix.right = parseExpr(parser, PREFIX);
	return expr;
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

