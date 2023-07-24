#include "ast.h"
#include <stdio.h>
#include <string.h>

#define MAX_PROGRAM_LEN 1000000

char* programToStr(Program* program) {
	char* str = (char*) malloc(MAX_PROGRAM_LEN);
	if (!str) {
		int success = fprintf(stderr, "Value of errno: %d\n", errno);
		perror("OUT OF MEMORY");
	}

	//Init with NTC for strcat to look for when adding next string
	str[0] = '\0';

	for (size_t i = 0; i < program->size; i++) {
		statementToStr(str, &program->statements[i]);
		//strcat_s(str, MAX_PROGRAM_LEN, "\n");
	}
	return str;
}

void letStatementToStr(char* str, Statement* stmt) {
	strcat_s(str, MAX_PROGRAM_LEN, stmt->token.literal);
	strcat_s(str, MAX_PROGRAM_LEN, " ");
	strcat_s(str, MAX_PROGRAM_LEN, stmt->identifier.value);
	strcat_s(str, MAX_PROGRAM_LEN, " = ");
	strcat_s(str, MAX_PROGRAM_LEN, stmt->expr->token.literal);
	strcat_s(str, MAX_PROGRAM_LEN, ";");
}

void retStatementToStr(char* str, Statement* stmt) {
	strcat_s(str, MAX_PROGRAM_LEN, stmt->token.literal);
	strcat_s(str, MAX_PROGRAM_LEN, " ");
	strcat_s(str, MAX_PROGRAM_LEN, stmt->expr->token.literal);
	strcat_s(str, MAX_PROGRAM_LEN, ";");
}


void exprStatementToStr(char* str, Expression* expr) {
	switch (expr->type) {
		case EXPR_PREFIX:
			strcat_s(str, MAX_PROGRAM_LEN, "(");
			strcat_s(str, MAX_PROGRAM_LEN, expr->token.literal);
			exprStatementToStr(str, expr->prefix.right);
			strcat_s(str, MAX_PROGRAM_LEN, ")");
			break;

		case EXPR_INFIX:
			strcat_s(str, MAX_PROGRAM_LEN, "(");
			exprStatementToStr(str, expr->infix.left);
			strcat_s(str, MAX_PROGRAM_LEN, " ");
			strcat_s(str, MAX_PROGRAM_LEN, expr->token.literal);
			strcat_s(str, MAX_PROGRAM_LEN, " ");
			exprStatementToStr(str, expr->infix.right);
			strcat_s(str, MAX_PROGRAM_LEN, ")");
			break;

		case EXPR_IDENT:
			strcat_s(str, MAX_PROGRAM_LEN, expr->ident.value);
			break;

		case EXPR_INT:
			strcat_s(str, MAX_PROGRAM_LEN, expr->token.literal);
			break;

		default:
			strcat_s(str, MAX_PROGRAM_LEN, expr->token.literal);
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
	case TokenTypeBang: return OP_NEGATE; 
	case TokenTypeMinus: return OP_SUBTRACT; 
	case TokenTypePlus: return OP_ADD; 
	case TokenTypeAsterisk: return OP_MULTIPLY; 
	case TokenTypeGT: return OP_GT;
	case TokenTypeLT: return OP_LT; 
	case TokenTypeEqual: return OP_EQ; 
	case TokenTypeNotEqual: return OP_NOT_EQ; 
	case TokenTypeSlash: return OP_DIVIDE; 
	default:
		break;
	}

	return OP_UNKNOWN;
}