#pragma once
#include <stdint.h>

#include "../lexer/lexer.h"

enum Precedence {
    LOWEST = 1,
    EQUALS,         // ==
    LESSGREATER,    // < or >
    SUM,            // - and +
    PRODUCT,        // * and /
    PREFIX,         // - or !x
    CALL,           // fn()
};

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

enum OperatorType {
	OP_UNKNOWN,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE,
    OP_EQ,
    OP_NOT_EQ,
    OP_NEGATE,
    OP_AND,
    OP_OR,
    OP_MODULO,
};

struct PrefixExpression {
    Token token;
    enum OperatorType operatorType;
    struct SExpression* right;
};

struct InfixExpression {
    enum OperatorType operatorType;
    struct SExpression* left;
    struct SExpression* right;
};

struct IfExpression {
    struct SExpression* condition;
    struct blockStatement* consequence;
    struct blockStatement* alternative;
};

typedef struct SIdentifier {
	Token token;
	char value[MAX_IDENT_LENGTH];
} Identifier;

typedef struct SExpression {
    enum ExpressionType type;
	Token token;
    union {
        int64_t integer;
        bool boolean;
        char* string;
        Identifier ident;
        struct PrefixExpression prefix;
        struct InfixExpression infix;
        struct IfExpression ifelse;
    };
} Expression;

typedef struct SStatement {
    enum StatementType type;
	Token token;
	Identifier identifier;
    Expression* expr;
} Statement;

typedef struct SProgram {
	Statement* statements;
    size_t cap;
	size_t size;
} Program;


char* programToStr(Program* program);
void statementToStr(char* str, Statement* stmt);
enum OperatorType parseOperator(TokenType tokenType);