#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

#include "cheat.h"
#include "proc.h"

// declerations

bool healthBool = false;
bool ammoBool = false;

int health = 9999;
int ammo = 1337;

DWORD pID, baseModule, localPlayerPtr;

	int cheat::hook()
	{
		std::cout << "\n=================================================== Initializing Hook =================================================\n" << std::endl;

		std::cout << "[+] Hooking into Assault Cube" << std::endl;

		pID = GetProcessID(L"ac_client.exe");

		baseModule = GetModuleBaseAddress(pID, L"ac_client.exe");

		if (pID != 0 && baseModule != 0)
		{
			std::cout << "[+] Hooked into game successfully\n" << std::endl;
			std::cout << "[+] ProcessID: " << std::hex << pID << std::endl;
			std::cout << "[+] BaseModuleAddress: " << std::hex << baseModule << std::endl;
		}
		else
		{
			std::cout << "[+] Hooking Failed! Please open Assault Cube\n" << std::endl;
			system("pause");
			exit(EXIT_FAILURE);
		}
	}

	void cheat::main()
	{
		HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, NULL, pID);

		ReadProcessMemory(handle, (LPCVOID)(baseModule + 0x17E0A8), &localPlayerPtr, sizeof(localPlayerPtr), nullptr);

		std::cout << "\n=============================================== Initializing Main Cheat ================================================\n" << std::endl;

		std::cout << "[+] Health overwritten successfully" << std::endl;
		std::cout << "[+] Ammo overwritten successfully" << std::endl;

		while (true)
		{
			WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0xEC), &health, sizeof(health), nullptr); // health overwrite

			WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0x140), &ammo, sizeof(ammo), nullptr); // rifle ammo overwrite
			WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0x12C), &ammo, sizeof(ammo), nullptr); // pistol ammo overwrite
		}
	}