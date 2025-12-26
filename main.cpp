#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <dwmapi.h>
#include <d3d11.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "proc.h"
#include "cheat.h"
#include "offsets.h"

int main()
{
	cheat::hook();
	cheat::main();
}