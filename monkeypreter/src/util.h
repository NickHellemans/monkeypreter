#pragma once
#include <cstdio>
#include <cstdlib>

static void errorAndDie(const char* msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}
