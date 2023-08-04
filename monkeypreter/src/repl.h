#pragma once

#include <string.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/object.h"
#include "interpreter/environment.h"

const char* MONKEY_FACE = "            __,__\n\
   .--.  .-\"     \"-.  .--.\n\
  / .. \\/  .-. .-.  \\/ .. \\\n\
 | |  '|  /   Y   \\  |'  | |\n\
 | \\   \\  \\ 0 | 0 /  /   / |\n\
  \\ '- ,\\.-\"\"\"\"\"\"\"-./, -' /\n\
   ''-' /_   ^ ^   _\\ '-''\n\
       |  \\._   _./  |\n\
       \\   \\ '~' /   /\n\
        '._ '-=-' _.'\n\
           '-----'\n\
";

inline void printParserErrors(Parser* parser) {
	printf("Oopsie daisy! We ran into some monkey business!\n");
	printf("Parser errors\n");
	for (size_t i = 0; i < parser->errorsLen; i++) {
		printf("\t - Parser error %llu: %s\n", i + 1, parser->errors[i]);
	}
}

inline void repl(void) {
	printf("%s\n", MONKEY_FACE);
	printf("Type 'exit' to exit REPL\n");
	struct ObjectEnvironment* env = newEnvironment();
	while (true) {
		char inputBuffer[100];
		printf(">> ");
		char* success = fgets(inputBuffer, sizeof(inputBuffer), stdin);
		Lexer lexer = createLexer(inputBuffer);
		Parser parser = createParser(&lexer);
		Program* program = parseProgram(&parser);
		if(parser.errorsLen != 0) {
			printParserErrors(&parser);
			continue;
		}

		struct Object evaluated = evalProgram(program, env);

		if (strncmp(inputBuffer, "exit", 4) == 0) {
			printf("Exiting REPL...");
			break;
		}

		if(evaluated.type != OBJ_NULL && evaluated.type != OBJ_FUNCTION && evaluated.type != OBJ_RETURN) {
			char* objStr = inspectObject(&evaluated);
			printf("%s\n\n", objStr);
			free(objStr);
		}
		

		//Clean up memory
		//freeProgram(program);
		freeParser(&parser);
	}
	deleteAllEnvironment(env);
}
