#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

#include "proc.h"

bool healthBool = false;
bool ammoBool = false;

void main() {
	DWORD pID, baseModule, localPlayerPtr;

	int health = 6969;
	int ammo = 1337;

	pID = GetProcessID(L"ac_client.exe");

	baseModule = GetModuleBaseAddress(pID, L"ac_client.exe");

	if (pID != 0 && baseModule != 0)
	{
		std::cout << "ProcessID: " << std::hex << pID << std::endl;
		std::cout << "BaseModuleAddress: " << std::hex << baseModule << std::endl;
	}
	else
	{
		std::cout << "Assault Cube was not found. Please Open Assault Cube \n" << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, NULL, pID);

	ReadProcessMemory(handle, (LPCVOID)(baseModule + 0x17E0A8), &localPlayerPtr, sizeof(localPlayerPtr), nullptr);

	while (true)
	{
		WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0xEC), &health, sizeof(health), nullptr); // health overwrite

		WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0x140), &ammo, sizeof(ammo), nullptr); // rifle ammo overwrite
		WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0x12C), &ammo, sizeof(ammo), nullptr); // pistol ammo overwrite
	}
}