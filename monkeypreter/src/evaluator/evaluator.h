#pragma once
#include "../parser/ast.h"
#include "environment.h"

struct Object* evalProgram(Program* program, struct ObjectEnvironment* env);
struct Object* newEvalError(struct MonkeyGC* gc, const char* format, ...);