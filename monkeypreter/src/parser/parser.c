#include "parser.h"
#include <stdio.h>
#include <string.h>

//Internal declarations
void setParserNextToken(Parser* parser);
struct Expression* createExpression(enum ExpressionType type, Token token);
struct Statement parseStatement(Parser* parser);
struct Statement parseLetStatement(Parser* parser);
struct Statement parseRetStatement(Parser* parser);
struct Statement parseExprStatement(Parser* parser);
struct Expression* parseExpr(Parser* parser, enum Precedence precedence);
struct Expression* parseIdentExpr(Parser* parser);
struct Expression* parseIntegerLiteralExpr(Parser* parser);
struct Expression* parsePrefixExpr(Parser* parser);
struct Expression* parseInfixExpr(Parser* parser, struct Expression* left);
struct Expression* parseBoolExpr(Parser* parser);
struct Expression* parseGroupedExpr(Parser* parser);
struct Expression* parseIfExpression(Parser* parser);
struct BlockStatement* parseBlockStatement(Parser* parser);
struct Expression* parseFunctionLiteralExpr(Parser* parser);
struct IdentifierList parseFunctionParameters(Parser* parser);
struct Expression* parseCallExpression(Parser* parser, struct Expression* left);
struct ExpressionList parseExpressionList(Parser* parser, TokenType end);
struct Expression* parseStringLiteral(Parser* parser);
struct Expression* parseArrayLiteral(Parser* parser);
struct Expression* parseIndexExpression(Parser* parser, struct Expression* left);
void peekError(Parser* parser, TokenType type);
bool expectPeek(Parser* parser, TokenType tokenType);
bool curTokenIs(Parser* parser, TokenType tokenType);
bool peekTokenIs(Parser* parser, TokenType tokenType);

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

		case TokenTypeLParen:
			return CALL;

		case TokenTypeLBracket:
			return INDEX;

		default:
			return LOWEST;
	}
}

Parser createParser(Lexer* lexer) {
	Parser parser;
	parser.lexer = lexer;
	parser.errorsLen = 0;
	//Init with space for 5 strings
	parser.errorsCap = 5;
	parser.errors = (char**) malloc(parser.errorsCap * sizeof *parser.errors);

	if(!parser.errors) {
		perror("malloc (parser errors) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	//Read 2 tokens so cur & peek are set
	setParserNextToken(&parser);
	setParserNextToken(&parser);

	return parser;
}

void freeParser(Parser* parser) {

	if (parser->errorsLen <= 0)
		return;

	for (size_t i = 0; i < parser->errorsLen; i++) {
		free(parser->errors[i]);
	}

	free(parser->errors);
}

struct Expression* createExpression(enum ExpressionType type, Token token) {
	struct Expression* expr = (struct Expression*) malloc(sizeof *expr);
	if (!expr) {
		perror("malloc (create Expression) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	expr->type = type;
	expr->token = token;
	return expr;
}

struct BlockStatement* createBlockStatement(Token token) {
	struct BlockStatement* bs = (struct BlockStatement*) malloc(sizeof *bs);
	if (!bs) {
		perror("malloc (create BlockStatement) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}
	bs->token = token;
	bs->cap = 5;
	bs->size = 0;
	bs->statements = (struct Statement*) malloc(bs->cap * sizeof *bs->statements);

	if (!bs->statements) {
		perror("malloc (create BlockStatement statements) returned `NULL`\n");
		free(bs);
		exit(EXIT_FAILURE);
	}

	return bs;
}

void setParserNextToken(Parser* parser) {
	parser->curToken = parser->peekToken;
	parser->peekToken = nextToken(parser->lexer);
}

Program* parseProgram(Parser* parser) {
	Program* program = (Program*) malloc(sizeof *program);
	if(!program) {
		perror("malloc (parse Program) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	program->size = 0;
	program->cap = 100;
	program->statements = (struct Statement*) malloc(program->cap * sizeof *program->statements);

	if (!program->statements) {
		perror("malloc (Program statements) returned `NULL`\n");
		free(program);
		exit(EXIT_FAILURE);
	}

	while (!curTokenIs(parser, TokenTypeEof)) {
		struct Statement stmt = parseStatement(parser);
		if (stmt.type != STMT_ILLEGAL) {
			//Double size if needed
			if(program->size >= program->cap) {
				program->cap *= 2;
				struct Statement* tmp = (struct Statement*) realloc(program->statements, program->cap * sizeof * program->statements);
				if(!tmp) {
					perror("realloc (Program statements) returned `NULL`\n");
					freeProgram(program);
					exit(EXIT_FAILURE);
				}
				program->statements = tmp;
			}

			program->statements[program->size] = stmt;
			program->size++;
		}
		setParserNextToken(parser);
	}
	return program;
}

struct Statement parseStatement(Parser* parser) {
	switch (parser->curToken.type) {
		case TokenTypeLet : 
			return parseLetStatement(parser);
		case TokenTypeReturn:
			return parseRetStatement(parser);
		default:
			return parseExprStatement(parser);
	}
}

struct Statement parseLetStatement(Parser* parser) {
	struct Statement stmt;
	stmt.type = STMT_LET;
	stmt.token = parser->curToken;

	if(!expectPeek(parser, TokenTypeIdent)) {
		stmt.type = STMT_ILLEGAL;
		return stmt;
	}

	struct Identifier ident;
	ident.token = parser->curToken;
	strcpy_s(ident.value, MAX_IDENT_LENGTH, parser->curToken.literal);
	stmt.identifier = ident;

	if(!expectPeek(parser, TokenTypeAssign)) {
		stmt.type = STMT_ILLEGAL;
		return stmt;
	}

	setParserNextToken(parser);
	stmt.expr = parseExpr(parser, (enum Precedence)LOWEST);

	if(peekTokenIs(parser, TokenTypeSemicolon)) {
		setParserNextToken(parser);
	}

	return stmt;
}

struct Statement parseRetStatement(Parser* parser) {
	struct Statement stmt;
	stmt.type = STMT_RETURN;
	stmt.token = parser->curToken;

	setParserNextToken(parser);
	stmt.expr = parseExpr(parser, (enum Precedence) LOWEST);

	if(peekTokenIs(parser, TokenTypeSemicolon)) {
		setParserNextToken(parser);
	}

	return stmt;
}

struct Statement parseExprStatement(Parser* parser) {
	struct Statement stmt;
	stmt.type = STMT_EXPR;
	stmt.token = parser->curToken;
	stmt.expr = parseExpr(parser, (enum Precedence)LOWEST);

	if (peekTokenIs(parser, TokenTypeSemicolon))
		setParserNextToken(parser);

	return stmt;
}

struct Expression* parseExpr(Parser* parser, enum Precedence precedence) {
	struct Expression* leftExpr = NULL;
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

		case TokenTypeFunction:
			leftExpr = parseFunctionLiteralExpr(parser);
			break;

		case TokenTypeString:
			leftExpr = parseStringLiteral(parser);
			break;

		case TokenTypeLBracket:
			leftExpr = parseArrayLiteral(parser);
			break;

		default:
			printf("Parser no support for TokenType: %s\n", tokenTypeToStr(parser->curToken.type));
			exit(EXIT_FAILURE);
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

			case TokenTypeLParen:
				setParserNextToken(parser);
				leftExpr = parseCallExpression(parser, leftExpr);
				break;

			case TokenTypeLBracket:
				setParserNextToken(parser);
				leftExpr = parseIndexExpression(parser, leftExpr);
				break;

			default:
				return leftExpr;
			}
	}

	return leftExpr;
}

struct Expression* parseIdentExpr(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_IDENT, parser->curToken);
	expr->ident.token = expr->token;
	strcpy_s(expr->ident.value, MAX_IDENT_LENGTH, parser->curToken.literal);
	return expr;
}

struct Expression* parseIntegerLiteralExpr(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_INT, parser->curToken);
	expr->integer = 0;
	const char* s = expr->token.literal;
	//Cast to int from string
	for (int i = 0; s[i] != '\0'; i++) {
		expr->integer = expr->integer * 10 + (s[i] - 48);
	}
	return expr;
}

struct Expression* parsePrefixExpr(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_PREFIX, parser->curToken);
	expr->prefix.token = parser->curToken;
	expr->prefix.operatorType = parseOperator(expr->token.type);
	setParserNextToken(parser);
	expr->prefix.right = parseExpr(parser, (enum Precedence)PREFIX);
	return expr;
}

struct Expression* parseInfixExpr(Parser* parser, struct Expression* left) {
	struct Expression* expr = createExpression(EXPR_INFIX, parser->curToken);
	expr->infix.token = parser->curToken;
	expr->infix.operatorType = parseOperator(expr->token.type);
	expr->infix.left = left;

	enum Precedence precedence = getTokenPrecedence(parser->curToken);
	setParserNextToken(parser);
	expr->infix.right = parseExpr(parser, (enum Precedence)precedence);
	return expr;
}

struct Expression* parseBoolExpr(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_BOOL, parser->curToken);
	//Set boolean to true if token is TokenTypeTrue else it is TokenTypeFalse
	expr->boolean = curTokenIs(parser, TokenTypeTrue);
	return expr;
}

struct Expression* parseGroupedExpr(Parser* parser) {

	setParserNextToken(parser);

	struct Expression* expr = parseExpr(parser, (enum Precedence) LOWEST);

	if(!expectPeek(parser, TokenTypeRParen)) {
		free(expr);
		return NULL;
	}

	return expr;
}

struct Expression* parseIfExpression(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_IF, parser->curToken);
	expr->ifelse.token = parser->curToken;

	if(!expectPeek(parser, TokenTypeLParen)) {
		free(expr);
		return NULL;
	}

	setParserNextToken(parser);

	expr->ifelse.condition = parseExpr(parser, (enum Precedence) LOWEST);
	if(!expectPeek(parser, TokenTypeRParen)) {
		free(expr);
		return NULL;
	}

	if (!expectPeek(parser, TokenTypeLSquirly)) {
		free(expr);
		return NULL;
	}

	expr->ifelse.consequence = parseBlockStatement(parser);
	expr->ifelse.alternative = NULL;

	if(peekTokenIs(parser, TokenTypeElse)) {
		setParserNextToken(parser);

		if(!expectPeek(parser, TokenTypeLSquirly)) {
			free(expr);
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
		struct Statement stmt = parseStatement(parser);
		if(stmt.type != STMT_ILLEGAL) {

			if(bs->size >= bs->cap) {
				bs->cap *= 2;
				struct Statement* tmp = (struct Statement*)realloc(bs->statements, bs->cap * sizeof * bs->statements);
				if(!tmp) {
					perror("OUT OF MEMORY");
					freeBlockStatement(bs);
					abort();
				}
				bs->statements = tmp;
			}
			bs->statements[bs->size] = stmt;
			bs->size++;
		}
		setParserNextToken(parser);
	}
	return bs;
}

struct Expression* parseFunctionLiteralExpr(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_FUNCTION, parser->curToken);
	expr->function.token = parser->curToken;

	if(!expectPeek(parser, TokenTypeLParen)) {
		free(expr);
		return NULL;
	}

	expr->function.parameters = parseFunctionParameters(parser);

	if(!expectPeek(parser, TokenTypeLSquirly)) {
		free(expr);
		return NULL;
	}

	expr->function.body = parseBlockStatement(parser);

	return expr;
}

struct IdentifierList parseFunctionParameters(Parser* parser) {
	struct IdentifierList params = { NULL, 0, 1 };

	if(peekTokenIs(parser, TokenTypeRParen)) {
		setParserNextToken(parser);
		return params;
	}

	setParserNextToken(parser);

	struct Identifier ident;
	ident.token = parser->curToken;
	strcpy_s(ident.value, MAX_IDENT_LENGTH, parser->curToken.literal);

	params.values = (struct Identifier*) malloc(params.cap * sizeof *params.values);

	if (!params.values) {
		perror("malloc (function parameters) returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	params.values[params.size] = ident;
	params.size++;

	while(peekTokenIs(parser, TokenTypeComma)) {
		setParserNextToken(parser);
		setParserNextToken(parser);

		ident.token = parser->curToken;
		strcpy_s(ident.value, MAX_IDENT_LENGTH, parser->curToken.literal);

		if(params.size >= params.cap) {
			params.cap *= 2;
			struct Identifier* tmp = (struct Identifier*)realloc(params.values, params.cap * sizeof * params.values);
			if (!tmp) {
				perror("realloc (function parameters) returned `NULL`\n");
				exit(EXIT_FAILURE);
			}
			params.values = tmp;
		}

		params.values[params.size] = ident;
		params.size++;
	}

	if(!expectPeek(parser, TokenTypeRParen)) {
		perror("NOT VALID SYNTAX");
	}

	return params;
}

struct Expression* parseCallExpression(Parser* parser, struct Expression* left) {
	struct Expression* expr = createExpression(EXPR_CALL, parser->curToken);
	expr->call.token = parser->curToken;
	expr->call.function = left;
	expr->call.arguments = parseExpressionList(parser, TokenTypeRParen);
	return expr;
}

struct ExpressionList parseExpressionList(Parser* parser, TokenType end) {
	struct ExpressionList params = { NULL, 0, 1};

	if (peekTokenIs(parser, end)) {
		setParserNextToken(parser);
		return params;
	}

	setParserNextToken(parser);
	params.values = (struct Expression**) malloc(params.cap * sizeof(struct Expression*));

	if (!params.values) {
		perror("OUT OF MEMORY");
	}

	params.values[params.size] = parseExpr(parser, (enum Precedence) LOWEST);
	params.size++;

	while (peekTokenIs(parser, TokenTypeComma)) {
		setParserNextToken(parser);
		setParserNextToken(parser);

		if (params.size >= params.cap) {
			params.cap *= 2;
			struct Expression** tmp = (struct Expression**)realloc(params.values, params.cap * sizeof(struct Expression*));
			if (!tmp) {
				perror("OUT OF MEMORY");
				free(params.values);
				exit(EXIT_FAILURE);
			}
			params.values = tmp;
		}

		params.values[params.size] = parseExpr(parser, (enum Precedence)LOWEST);
		params.size++;
	}

	if (!expectPeek(parser, end)) {
		free(params.values);
		params.values = NULL;
		perror("NOT VALID SYNTAX");
	}

	return params;
}

struct Expression* parseStringLiteral(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_STRING, parser->curToken);
	strcpy_s(expr->string, MAX_IDENT_LENGTH, parser->curToken.literal);
	return expr;
}

struct Expression* parseArrayLiteral(Parser* parser) {
	struct Expression* expr = createExpression(EXPR_ARRAY, parser->curToken);
	expr->array.token = parser->curToken;
	expr->array.elements = parseExpressionList(parser, TokenTypeRBracket);
	return expr;
}

struct Expression* parseIndexExpression(Parser* parser, struct Expression* left) {
	struct Expression* expr = createExpression(EXPR_INDEX, parser->curToken);
	expr->indexExpr.token = parser->curToken;
	setParserNextToken(parser);
	expr->indexExpr.left = left;
	expr->indexExpr.index = parseExpr(parser, (enum Precedence)LOWEST);

	if(!expectPeek(parser, TokenTypeRBracket)) {
		return NULL;
	}
	return expr;
}

void peekError(Parser* parser, TokenType type) {
	char* msg = (char*) malloc(128 * sizeof(char));

	if (!msg) {
		perror("malloc (peek error returned `NULL`\n");
		exit(EXIT_FAILURE);
	}

	int success = sprintf_s(msg, 128 * sizeof(char), "expected next token to be: `%s`, got: `%s` instead", tokenTypeToStr(type), tokenTypeToStr(parser->peekToken.type));

	if(parser->errorsLen >= parser->errorsCap) {
		parser->errorsCap *= 2;
		char** tmp = (char**)realloc(parser->errors, parser->errorsCap * sizeof * parser->errors);
		if(!tmp) {
			free(msg);
			perror("realloc (peek error returned `NULL`\n");
			exit(EXIT_FAILURE);
		}
		parser->errors = tmp;
	}
	parser->errors[parser->errorsLen] = msg;
	parser->errorsLen++;
}

bool expectPeek(Parser* parser, TokenType tokenType) {

	if(peekTokenIs(parser, tokenType)) {
		setParserNextToken(parser);
		return true;
	}

	peekError(parser, tokenType);
	return false;
}

bool curTokenIs(Parser* parser, TokenType tokenType) {
	return parser->curToken.type == tokenType;
}

bool peekTokenIs(Parser* parser, TokenType tokenType) {
	return parser->peekToken.type == tokenType;
}
