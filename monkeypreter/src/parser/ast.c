#include "ast.h"
#include <stdio.h>
#include <string.h>

char* programToStr(Program* program) {
	char* str = (char*) malloc(1000000);
	if (!str) {
		int success = fprintf(stderr, "Value of errno: %d\n", errno);
		perror("OUT OF MEMORY");
	}

	//Init with NTC for strcat to look for
	str[0] = '\0';

	for (size_t i = 0; i < program->size; i++) {
		statementToStr(str, &program->statements[i]);
		strcat_s(str, 1000000, "\n");
	}
	return str;
}

void letStatementToStr(char* str, Statement* stmt) {
	strcat_s(str, 1000000, stmt->token.literal);
	strcat_s(str, 1000000, " ");
	strcat_s(str, 1000000, stmt->identifier.value);
	strcat_s(str, 1000000, " = ");
	strcat_s(str, 1000000, stmt->expr->token.literal);
	strcat_s(str, 1000000, ";");
}

void retStatementToStr(char* str, Statement* stmt) {
	strcat_s(str, 1000000, stmt->token.literal);
	strcat_s(str, 1000000, " ");
	strcat_s(str, 1000000, stmt->expr->token.literal);
	strcat_s(str, 1000000, ";");
}


void exprStatementToStr(char* str, Expression* expr) {
	switch (expr->type) {
		case EXPR_PREFIX:
			strcat_s(str, 1000000, "(");
			strcat_s(str, 1000000, expr->token.literal);
			exprStatementToStr(str, expr->prefix.right);
			strcat_s(str, 1000000, ")");
			break;

		default:
			strcat_s(str, 1000000, expr->token.literal);
	}
}

void statementToStr(char* str, Statement* stmt) {

	switch (stmt->type) {

		case STMT_LET:
			letStatementToStr(str, stmt);
			break;
		case STMT_RETURN:
			retStatementToStr(str, stmt);
			break;
		case STMT_EXPR:
			exprStatementToStr(str, stmt->expr);
			break;
		default: break;
	}
}

enum OperatorType parseOperator(TokenType tokenType) {
	switch (tokenType) {
	case TokenTypeBang: return OP_NEGATE; break;
	case TokenTypeMinus: return OP_SUBTRACT; break;
	case TokenTypePlus: return OP_ADD; break;
	case TokenTypeAsterisk: return OP_MULTIPLY; break;
	case TokenTypeGT: return OP_GT; break;
	case TokenTypeLT: return OP_LT; break;
	case TokenTypeEqual: return OP_EQ; break;
	case TokenTypeNotEqual: return OP_NOT_EQ; break;
	case TokenTypeSlash: return OP_DIVIDE; break;
	default:
		break;
	}

	return OP_UNKNOWN;
}