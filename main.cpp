#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

#include "proc.h"
#include "cheat.h"

void main() 
{
	cheat::hook();
	cheat::main();
}