// monkeypreter.c : This file contains the 'main' function. Program execution begins and ends there.
//

//Toggle memory tracker
#define TOGGLE_MEM_TRACK    
#define _CRTDBG_MAP_ALLOC

#include <stdio.h>
#include "src/repl.h"
#include <crtdbg.h>

int main(void)
{
    printf("Welcome to the Monkeypreter!\n");
    repl();

#ifdef TOGGLE_MEM_TRACK
    //Dump memory leaks if defined
    _CrtDumpMemoryLeaks();
#endif
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

