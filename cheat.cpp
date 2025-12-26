#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <thread>
#include <chrono>

#include "cheat.h"
#include "proc.h"
#include "overlay.h"

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
			std::cout << "[!] Hooking Failed! Open Assault Cube\n\n[+] ";
			system("pause");
			exit(EXIT_FAILURE);
		}
		return pID && baseModule;
	}

	void cheat::main()
	{
			HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, NULL, pID);

			ReadProcessMemory(handle, (LPCVOID)(baseModule + 0x17E0A8), &localPlayerPtr, sizeof(localPlayerPtr), nullptr);

			std::cout << "\n=============================================== Initializing Main Cheat ================================================\n";

			// start External Overlay

			std::thread overlayThread([]()
				{
					HINSTANCE instance = GetModuleHandleW(nullptr);
					RunOverlay(instance, SW_SHOW);
				});

			overlayThread.detach();

			std::cout << "[+] Health overwritten successfully" << std::endl;
			std::cout << "[+] Ammo overwritten successfully" << std::endl;

			while (true)
			{
				WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0xEC), &health, sizeof(health), nullptr); // health overwrite

				WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0x140), &ammo, sizeof(ammo), nullptr); // rifle ammo overwrite
				WriteProcessMemory(handle, (LPVOID)(localPlayerPtr + 0x12C), &ammo, sizeof(ammo), nullptr); // pistol ammo overwrite
			}
	}