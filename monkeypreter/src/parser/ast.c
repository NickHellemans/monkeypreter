#include "ast.h"
#include <stdio.h>
#include <string.h>

#define MAX_PROGRAM_LEN 1000000

//Internal declarations
void freeStatements(Statement* stmts, size_t size);
void freeExpression(Expression* expr);
void statementToStr(char* str, const Statement* stmt);


void freeProgram(Program* program) {
	freeStatements(program->statements, program->size);
	free(program);
}

void freeStatements(Statement* stmts, size_t size) {
	for(size_t i = 0; i < size; i++) {
		freeExpression(stmts[i].expr);
	}
	free(stmts);
}

void freeExpression(Expression* expr) {

	if(!expr) {
		return;
	}

	switch (expr->type) {
		case EXPR_INFIX: 
			freeExpression(expr->infix.left);
			freeExpression(expr->infix.right);
			break;

		case EXPR_PREFIX: 
			freeExpression(expr->prefix.right);
			break;

		case EXPR_IF: 
			free(expr->ifelse.condition);
			freeBlockStatement(expr->ifelse.consequence);
			freeBlockStatement(expr->ifelse.alternative);
			break;

		case EXPR_FUNCTION:
			//Hand over ownership to object
			//GC will handle
			//free(expr->function.parameters.values);
			//freeBlockStatement(expr->function.body);
			break;

		case EXPR_CALL: 
			for(size_t i = 0; i < expr->call.arguments.size; i++) {
				freeExpression(expr->call.arguments.values[i]);
			}
			free(expr->call.arguments.values);
			freeExpression(expr->call.function);
			break;

		case EXPR_ARRAY: 
			for (size_t i = 0; i < expr->array.elements.size; i++) {
				freeExpression(expr->array.elements.values[i]);
			}
			free(expr->array.elements.values);
			break;

		case EXPR_INDEX: 
			freeExpression(expr->indexExpr.left);
			freeExpression(expr->indexExpr.index);
			break;

		case EXPR_INT:
		case EXPR_IDENT: 
		case EXPR_BOOL: 
		case EXPR_STRING:
			//Nothing to free for these expressions
			break;

	}
	
	free(expr);
}

void freeBlockStatement(struct BlockStatement* bs) {

	if(!bs) {
		return;
	}

	freeStatements(bs->statements, bs->size);
	free(bs);
}

void exprStatementToStr(char* str, Expression* expr);

char* programToStr(const Program* program) {
	char* str = (char*) malloc(MAX_PROGRAM_LEN);
	if (!str) {
		int success = fprintf(stderr, "Value of errno: %d\n", errno);
		perror("OUT OF MEMORY");
		return NULL;
	}

	//Init with NTC for strcat to look for when adding next string
	str[0] = '\0';

	for (size_t i = 0; i < program->size; i++) {
		statementToStr(str, &program->statements[i]);
		//strcat_s(str, MAX_PROGRAM_LEN, "\n");
	}
	return str;
}

void letStatementToStr(char* str, const Statement* stmt) {
	strcat_s(str, MAX_PROGRAM_LEN, stmt->token.literal);
	strcat_s(str, MAX_PROGRAM_LEN, " ");
	strcat_s(str, MAX_PROGRAM_LEN, stmt->identifier.value);
	strcat_s(str, MAX_PROGRAM_LEN, " = ");
	exprStatementToStr(str, stmt->expr);
	strcat_s(str, MAX_PROGRAM_LEN, ";");
}

void retStatementToStr(char* str, const Statement* stmt) {
	strcat_s(str, MAX_PROGRAM_LEN, stmt->token.literal);
	strcat_s(str, MAX_PROGRAM_LEN, " ");
	exprStatementToStr(str, stmt->expr);
	strcat_s(str, MAX_PROGRAM_LEN, ";");
}

void blockStatementToStr(char* str, const struct BlockStatement* bs) {
	for(size_t i = 0; i < bs->size; i++) {
		statementToStr(str, &bs->statements[i]);
	}
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
		case EXPR_BOOL:
		case EXPR_STRING:
			strcat_s(str, MAX_PROGRAM_LEN, expr->token.literal);
			break;

		case EXPR_IF:
			strcat_s(str, MAX_PROGRAM_LEN, "if");
			exprStatementToStr(str, expr->ifelse.condition);
			strcat_s(str, MAX_PROGRAM_LEN, " ");
			blockStatementToStr(str, expr->ifelse.consequence);
			if(expr->ifelse.alternative) {
				strcat_s(str, MAX_PROGRAM_LEN, "else");
				blockStatementToStr(str, expr->ifelse.alternative);
			}
			break;

		case EXPR_FUNCTION:
			strcat_s(str, MAX_PROGRAM_LEN, expr->function.token.literal);
			strcat_s(str, MAX_PROGRAM_LEN, "(");
			for(size_t i = 0; i < expr->function.parameters.size; i++) {
				if(i > 0) {
					strcat_s(str, MAX_PROGRAM_LEN, ", ");
				}

				strcat_s(str, MAX_PROGRAM_LEN, expr->function.parameters.values[i].value);
			}
			strcat_s(str, MAX_PROGRAM_LEN, ")");
			strcat_s(str, MAX_PROGRAM_LEN, "{\n");
			blockStatementToStr(str, expr->function.body);
			strcat_s(str, MAX_PROGRAM_LEN, "\n}");
			break;
			
		case EXPR_CALL:
			exprStatementToStr(str, expr->call.function);
			strcat_s(str, MAX_PROGRAM_LEN, "(");
			for (size_t i = 0; i < expr->call.arguments.size; i++) {
				if (i > 0) {
					strcat_s(str, MAX_PROGRAM_LEN, ", ");
				}
				exprStatementToStr(str, expr->call.arguments.values[i]);
			}
			strcat_s(str, MAX_PROGRAM_LEN, ")");
			break;

		case EXPR_ARRAY:
			strcat_s(str, MAX_PROGRAM_LEN, "[");
			for (size_t i = 0; i < expr->array.elements.size; i++) {
				if (i > 0) {
					strcat_s(str, MAX_PROGRAM_LEN, ", ");
				}
				exprStatementToStr(str, expr->array.elements.values[i]);
			}
			strcat_s(str, MAX_PROGRAM_LEN, "]");
			break;

		case EXPR_INDEX:
			strcat_s(str, MAX_PROGRAM_LEN, "(");
			exprStatementToStr(str, expr->indexExpr.left);
			strcat_s(str, MAX_PROGRAM_LEN, "[");
			exprStatementToStr(str, expr->indexExpr.index);
			strcat_s(str, MAX_PROGRAM_LEN, "]");
			strcat_s(str, MAX_PROGRAM_LEN, ")");
			break;


		default:
			strcat_s(str, MAX_PROGRAM_LEN, expr->token.literal);
	}
}

void statementToStr(char* str, const Statement* stmt) {

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