#pragma once

#include <string.h>
#include "lexer/lexer.h"
#include "parser/parser.h"

inline void printParserErrors(Parser* parser) {
	printf("Oopsie daisy! We ran into some monkey business!\n");
	printf("Parser errors\n");
	for (size_t i = 0; i < parser->errorsLen; i++) {
		printf("\t - Parser error %llu: %s\n", i + 1, parser->errors[i]);
	}
}

inline void repl(void) {
	printf("Type 'exit' to exit REPL\n");
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

		if (strncmp(inputBuffer, "exit", 4) == 0) {
			printf("Exiting REPL...");
			break;
		}

		char* progmanStr = programToStr(program);
		printf("%s\n", progmanStr);
		printf("\n");

		//Clean up memory
		free(progmanStr);
		freeProgram(program);
		freeParser(&parser);
	}
}
