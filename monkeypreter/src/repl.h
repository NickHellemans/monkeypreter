#pragma once

#include <string.h>
#include "lexer/lexer.h"

inline void repl() {
	while (true) {
		char inputBuffer[100];
		printf("Type 'exit' to exit REPL\n");
		printf(">> ");
		char* succes = fgets(inputBuffer, sizeof(inputBuffer), stdin);
		Lexer lexer = createLexer(inputBuffer);
		Token token = nextToken(&lexer);

		if (strncmp(inputBuffer, "exit", 4) == 0) {
			printf("Exiting REPL...");
			break;
		}

		while(succes && token.type != TokenTypeEof) {
			printf("{Type: %s, Literal: %s}\n", tokenTypeToStr(token.type), token.literal);
			token = nextToken(&lexer);
		}
	}
}
