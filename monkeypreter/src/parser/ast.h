#pragma once
#include "../lexer/lexer.h"

enum ExpressionType {
    EXPR_INFIX = 1,
    EXPR_PREFIX,
    EXPR_INT,
    EXPR_IDENT,
    EXPR_BOOL,
    EXPR_IF,
    EXPR_FUNCTION,
    EXPR_CALL,
    EXPR_STRING,
    EXPR_ARRAY,
    EXPR_INDEX,
    EXPR_FOR,
    EXPR_WHILE,
    EXPR_ASSIGN,
};

enum StatementType {
    STMT_LET = 1,
    STMT_RETURN,
    STMT_EXPR,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_ILLEGAL,
};

typedef struct SIdentifier {
	Token token;
	char value[MAX_IDENT_LENGTH];
} Identifier;

typedef struct SExpression {
    enum ExpressionType type;
	Token token;
} Expression;

typedef struct SStatement {
    enum StatementType type;
	Token token;
	Identifier identifier;
	Expression expression;
} Statement;

typedef struct SProgram {
	Statement* statements;
    size_t cap;
	size_t size;
} Program;


char* programToStr(Program* program);
void statementToStr(char* str, Statement* stmt); 