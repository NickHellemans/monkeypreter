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
    STMT_ILLEGAL
};

enum OperatorType {
	OP_UNKNOWN,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_GT,
    OP_LT,
    OP_EQ,
    OP_NOT_EQ,
    OP_NEGATE,
};

struct PrefixExpression {
    Token token;
    enum OperatorType operatorType;
    struct Expression* right;
};

struct InfixExpression {
    Token token;
    struct Expression* left;
    enum OperatorType operatorType;
    struct Expression* right;
};

struct IfExpression {
    Token token;
    struct Expression* condition;
    struct BlockStatement* consequence;
    struct BlockStatement* alternative;
};

struct Identifier {
	Token token;
	char value[MAX_IDENT_LENGTH];
};

struct IdentifierList {
    struct Identifier* values;
    size_t size;
    size_t cap;
};

struct FunctionLiteral {
    Token token;
    struct IdentifierList parameters;
    struct BlockStatement* body;
};

struct ExpressionList {
    struct Expression** values;
    size_t size;
    size_t cap;
};

struct CallExpression {
    Token token;
    struct Expression* function;
    struct ExpressionList arguments;
};

struct ArrayLiteral {
    Token token;
    struct ExpressionList elements;
};

struct IndexExpression {
    Token token;
    struct Expression* left;
    struct Expression* index;
};

struct HashLiteral {
    Token token;
    struct HashMap* pairs;
};

struct Expression {
    enum ExpressionType type;
	Token token;
    union {
        int64_t integer;
        bool boolean;
        char string[MAX_IDENT_LENGTH];
        struct Identifier ident;
        struct PrefixExpression prefix;
        struct InfixExpression infix;
        struct IfExpression ifelse;
        struct FunctionLiteral function;
        struct CallExpression call;
        struct ArrayLiteral array;
        struct IndexExpression indexExpr;
    };
};

struct Statement {
    enum StatementType type;
	Token token;
	struct Identifier identifier;
    struct Expression* expr;
};

struct BlockStatement {
    Token token;
    size_t cap;
    size_t size;
    struct Statement* statements;
};

typedef struct Program {
	struct Statement* statements;
    size_t cap;
	size_t size;
} Program;


char* programToStr(const Program* program);
enum OperatorType parseOperator(TokenType tokenType);
void blockStatementToStr(char* str, const struct BlockStatement* bs);
//Free memory
void freeProgram(Program* program);
void freeBlockStatement(struct BlockStatement* bs);