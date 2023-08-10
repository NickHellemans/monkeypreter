#pragma once
#include <stdint.h>
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
    Token token;
    struct SExpression* left;
    enum OperatorType operatorType;
    struct SExpression* right;
};

struct IfExpression {
    Token token;
    struct SExpression* condition;
    struct BlockStatement* consequence;
    struct BlockStatement* alternative;
};

typedef struct SIdentifier {
	Token token;
	char value[MAX_IDENT_LENGTH];
} Identifier;

struct IdentifierList {
    Identifier* values;
    size_t size;
    size_t cap;
};

struct FunctionLiteral {
    Token token;
    struct IdentifierList parameters;
    struct BlockStatement* body;
};

struct ExpressionList {
    struct SExpression** values;
    size_t size;
    size_t cap;
};

struct CallExpression {
    Token token;
    struct SExpression* function;
    struct ExpressionList arguments;
};

struct ArrayLiteral {
    Token token;
    struct ExpressionList elements;
};

struct IndexExpression {
    Token token;
    struct SExpression* left;
    struct SExpression* index;
};

struct HashLiteral {
    Token token;
    struct HashMap* pairs;
};

typedef struct SExpression {
    enum ExpressionType type;
	Token token;
    union {
        int64_t integer;
        bool boolean;
        char string[MAX_IDENT_LENGTH];
        Identifier ident;
        struct PrefixExpression prefix;
        struct InfixExpression infix;
        struct IfExpression ifelse;
        struct FunctionLiteral function;
        struct CallExpression call;
        struct ArrayLiteral array;
        struct IndexExpression indexExpr;
    };
} Expression;

typedef struct SStatement {
    enum StatementType type;
	Token token;
	Identifier identifier;
    Expression* expr;
} Statement;

struct BlockStatement {
    Token token;
    size_t cap;
    size_t size;
    Statement* statements;
};

typedef struct SProgram {
	Statement* statements;
    size_t cap;
	size_t size;
} Program;


char* programToStr(const Program* program);
enum OperatorType parseOperator(TokenType tokenType);
void blockStatementToStr(char* str, const struct BlockStatement* bs);
//Free memory
void freeProgram(Program* program);
void freeBlockStatement(struct BlockStatement* bs);