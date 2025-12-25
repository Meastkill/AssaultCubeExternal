#pragma once

#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

DWORD GetProcessID(const wchar_t* procName);

DWORD GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
