#pragma once
#include "../parser/ast.h"
#include "environment.h"

struct Object* evalProgram(Program* program, struct ObjectEnvironment* env);
struct Object* evalStatement(struct Statement* stmt, struct ObjectEnvironment* env);
struct Object* evalExpression(struct Expression* expr, struct ObjectEnvironment* env);
struct Object* evalPrefixExpression(enum OperatorType op, struct Object* right, struct MonkeyGC* gc);
struct Object* evalBangOperatorExpression(struct Object* right);
struct Object* evalMinusPrefixExpression(struct Object* right, struct MonkeyGC* gc);
struct Object* evalInfixExpression(enum OperatorType op, struct Object* left, struct Object* right, struct MonkeyGC* gc);
struct Object* evalIntegerInfixExpression(enum OperatorType op, struct Object* left, struct Object* right, struct MonkeyGC* gc);
struct Object* evalIfExpression(struct IfExpression expr, struct ObjectEnvironment* env);
struct Object* evalBlockStatement(struct BlockStatement* bs, struct ObjectEnvironment* env);
struct Object* newEvalError(struct MonkeyGC* gc, const char* format, ...);
struct Object* evalIdentifier(struct Expression* expr, struct ObjectEnvironment* env);
struct ObjectList evalExpressions(struct ExpressionList expressions, struct ObjectEnvironment* env);
struct Object* applyFunction(struct Object* fn, struct ObjectList args, struct MonkeyGC* gc);
struct ObjectEnvironment* extendFunctionEnv(struct Object* fn, struct ObjectList args);
struct Object* unwrapReturnValue(struct Object* obj);
struct Object* createFunctionObject(struct Expression* expr, struct ObjectEnvironment* env);
struct Object* evalStringInfixExpression(enum OperatorType op, struct Object* left, struct Object* right, struct MonkeyGC* gc);

struct Object* evalIndexExpression(struct Object* left, struct Object* index, struct MonkeyGC* gc);
struct Object* evalArrayIndexExpression(struct Object* arr, struct Object* index);