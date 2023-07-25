#include "parser.h"

#include <stdio.h>
#include <string.h>

enum Precedence {
	LOWEST = 1,
	EQUALS,         // ==
	LESSGREATER,    // < or >
	SUM,            // - and +
	PRODUCT,        // * and /
	PREFIX,         // - or !x
	CALL,           // fn()
};


enum Precedence getTokenPrecedence(Token token) {
	switch (token.type) {

		case TokenTypeEqual:
		case TokenTypeNotEqual:
			return EQUALS;

		case TokenTypeLT:
		case TokenTypeGT:
			return LESSGREATER;

		case TokenTypePlus:
		case TokenTypeMinus:
			return SUM;

		case TokenTypeSlash:
		case TokenTypeAsterisk:
			return PRODUCT;

		default:
			return LOWEST;
	}
}

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
		return NULL;
	}
	expr->type = type;
	expr->token = token;
	return expr;
}

struct BlockStatement* createBlockStatement(Token token) {
	struct BlockStatement* bs = (struct BlockStatement*) malloc(sizeof *bs);
	if (!bs) {
		perror("OUT OF MEMORY");
		return NULL;
	}
	bs->token = token;
	bs->cap = 0;
	bs->size = 0;
	bs->statements = (Statement*) malloc(1000000);
	if (!bs->statements) {
		perror("OUT OF MEMORY");
		return NULL;
	}
	return bs;
}

void setParserNextToken(Parser* parser) {
	parser->curToken = parser->peekToken;
	parser->peekToken = nextToken(parser->lexer);
}

Program* parseProgram(Parser* parser) {
	Program* program = (Program*) malloc(sizeof(Statement*) + 2 * sizeof(size_t));
	if(!program) {
		return NULL;
	}
	program->statements = (Statement*) malloc(1000000);

	if (!program->statements) {
		return NULL;
	}
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
	stmt.expr = parseExpr(parser, (enum Precedence)LOWEST);

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

		case TokenTypeFalse:
		case TokenTypeTrue:
			leftExpr = parseBoolExpr(parser);
			break;

		case TokenTypeLParen:
			leftExpr = parseGroupedExpr(parser);
			break;

		case TokenTypeIf:
			leftExpr = parseIfExpression(parser);
			break;

		default:
			//Error msg here?
			return NULL;
	}

	while(!peekTokenIs(parser, TokenTypeSemicolon) && precedence < getTokenPrecedence(parser->peekToken)) {
		switch (parser->peekToken.type) {
			case TokenTypePlus:
			case TokenTypeMinus:
			case TokenTypeAsterisk:
			case TokenTypeSlash:
			case TokenTypeEqual:
			case TokenTypeNotEqual:
			case TokenTypeLT:
			case TokenTypeGT:
				setParserNextToken(parser);
				leftExpr = parseInfixExpr(parser, leftExpr);
				break;

			default:
				return leftExpr;
			}
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
	expr->prefix.right = parseExpr(parser, (enum Precedence)PREFIX);
	return expr;
}

Expression* parseInfixExpr(Parser* parser, Expression* left) {
	Expression* expr = createExpression(EXPR_INFIX, parser->curToken);
	expr->infix.operatorType = parseOperator(expr->token.type);
	expr->infix.left = left;

	enum Precedence precedence = getTokenPrecedence(parser->curToken);
	setParserNextToken(parser);
	expr->infix.right = parseExpr(parser, (enum Precedence)precedence);
	return expr;
}

Expression* parseBoolExpr(Parser* parser) {
	Expression* expr = createExpression(EXPR_BOOL, parser->curToken);
	//Set boolean to true if token is TokenTypeTrue else it is TokenTypeFalse
	expr->boolean = curTokenIs(parser, TokenTypeTrue);
	return expr;
}

Expression* parseGroupedExpr(Parser* parser) {

	setParserNextToken(parser);

	Expression* expr = parseExpr(parser, (enum Precedence) LOWEST);

	if(!expectPeek(parser, TokenTypeRParen)) {
		free(expr);
		return NULL;
	}

	return expr;
}

Expression* parseIfExpression(Parser* parser) {
	Expression* expr = createExpression(EXPR_IF, parser->curToken);

	if(!expectPeek(parser, TokenTypeLParen)) {
		return NULL;
	}

	setParserNextToken(parser);

	expr->ifelse.condition = parseExpr(parser, (enum Precedence) LOWEST);
	if(!expectPeek(parser, TokenTypeRParen)) {
		return NULL;
	}

	if (!expectPeek(parser, TokenTypeLSquirly)) {
		return NULL;
	}

	expr->ifelse.consequence = parseBlockStatement(parser);
	expr->ifelse.alternative = NULL;

	if(peekTokenIs(parser, TokenTypeElse)) {
		setParserNextToken(parser);

		if(!expectPeek(parser, TokenTypeLSquirly)) {
			return NULL;
		}

		expr->ifelse.alternative = parseBlockStatement(parser);

	}
	return expr;
}

struct BlockStatement* parseBlockStatement(Parser* parser) {
	struct BlockStatement* bs = createBlockStatement(parser->curToken);
	setParserNextToken(parser);
	while(!curTokenIs(parser, TokenTypeRSquirly) && !curTokenIs(parser, TokenTypeEof)) {
		Statement stmt = parseStatement(parser);
		if(stmt.type != STMT_ILLEGAL) {
			bs->statements[bs->size] = stmt;
			bs->size++;
		}
		setParserNextToken(parser);
	}
	return bs;
}

void peekError(Parser* parser, TokenType type) {
	char* msg = (char*) malloc(128 * sizeof(char));
	if (!msg) {
		return;
	}

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

