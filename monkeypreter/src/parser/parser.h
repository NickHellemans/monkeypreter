#pragma once
#include "../lexer/lexer.h"
#include "ast.h"

typedef struct SParser {
	Lexer* lexer;
	Token curToken;
	Token peekToken;

	//Parsing error handling
	size_t errorsLen;
	char** errors;

} Parser;

Parser createParser(Lexer* lexer);
void setParserNextToken(Parser* parser);
Expression* createExpression(enum ExpressionType type, Token token);
//Constructs AST returns head
Program* parseProgram(Parser* parser);
Statement parseStatement(Parser* parser);
Statement parseLetStatement(Parser* parser);
Statement parseRetStatement(Parser* parser);
Statement parseExprStatement(Parser* parser);
Expression* parseExpr(Parser* parser, enum Precedence precedence);
Expression* parseIdentExpr(Parser* parser);
Expression* parseIntegerLiteralExpr(Parser* parser);
Expression* parsePrefixExpr(Parser* parser);
Expression* parseInfixExpr(Parser* parser, Expression* left);
Expression* parseBoolExpr(Parser* parser);
Expression* parseGroupedExpr(Parser* parser);
Expression* parseIfExpression(Parser* parser);
struct BlockStatement* parseBlockStatement(Parser* parser);
Expression* parseFunctionLiteralExpr(Parser* parser);
struct IdentifierList parseFunctionParameters(Parser* parser);
Expression* parseCallExpression(Parser* parser, Expression* left);
struct ExpressionList parseCallExprParameters(Parser* parser);
void peekError(Parser* parser, TokenType type);
bool expectPeek(Parser* parser, TokenType tokenType);
bool curTokenIs(Parser* parser, TokenType tokenType);
bool peekTokenIs(Parser* parser, TokenType tokenType);