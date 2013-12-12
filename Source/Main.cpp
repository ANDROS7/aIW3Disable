// Hedgehogscience.com

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

DWORD GetPID(const char *Windowname)
{
	HWND Window = NULL;
	DWORD PID = 0;

	if (!(Window = FindWindowA(Windowname, NULL)))
	if (!(Window = FindWindowA(NULL, Windowname)))
		printf("Could not find aIW.");
	else
		GetWindowThreadProcessId(Window, &PID);
	else
		GetWindowThreadProcessId(Window, &PID);

	return PID;
}

DWORD GetBaseAddress(DWORD PID, const char *ModuleName)
{
	HANDLE Snapshot = INVALID_HANDLE_VALUE;
	DWORD Result = NULL;

	Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
	if (Snapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32 = { 0 };
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(Snapshot, &ModuleEntry32))
		{
			do
			{
				if (strcmp(ModuleName, ModuleEntry32.szModule) == 0)
				{
					Result = (DWORD)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(Snapshot, &ModuleEntry32));
		}
		CloseHandle(Snapshot);
	}

	if (Result == 0)
		printf("Could not find module :%s\n", ModuleName);

	return Result;
}

DWORD GetValueAtAddress(HANDLE Process, DWORD Address, size_t Size)
{
	DWORD Result = 0x00000000;

	ReadProcessMemory(Process, (void *)Address, &Result, Size, NULL);
	return Result;
}

int main(int argc, char *argv[])
{
	DWORD LibNPAddress = NULL;
	DWORD NetRTAddress = NULL;
	DWORD Iw4m2Address = NULL;
	DWORD ProcessID = NULL;
	HANDLE Process = INVALID_HANDLE_VALUE;

	ProcessID = GetPID("iw4");
	Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
	LibNPAddress = GetBaseAddress(ProcessID, "libnp.dll");
	NetRTAddress = GetBaseAddress(ProcessID, "NetRT.4");
	Iw4m2Address = GetBaseAddress(ProcessID, "iw4m.dll");

	if (ProcessID == NULL)
		goto EXIT;

	// Check aCI
	if ((bool)GetValueAtAddress(Process, LibNPAddress + 0x10B380c, 1))
	{
		printf("Patching aCI..\n");
		// GameRT.dat + 0x3016 -> 0x7C
		// or
		// GameRT.dat + 0xCDC8 -> "troll 5001"
		// or
		// LibNP + 0x3019 -> 68 51 C3 90
		// or
		// LibNP + 0x29D7F -> NOP 5
		WriteProcessMemory(Process, (void *)(LibNPAddress + 0x3019), "\x68\x51\xC3\x90", 4, NULL);
	}
	else
		printf("aCI is not enabled.\n");

	// Namecheck fix
	if (GetValueAtAddress(Process, NetRTAddress + 0x272c, 1) == 0x74)
	{
		printf("Patching namecheck..\n");
		WriteProcessMemory(Process, (void *)(NetRTAddress + 0x272c), "\xEB", 1, NULL);
	}
	else
		printf("Namecheck already patched.\n");

	// Set name
	if (argc > 1)
	{
		printf("Setting name to %s..\n", argv[1]);
		WriteProcessMemory(Process, (void *)(Iw4m2Address + 0x10EE68), argv[1], strlen(argv[1]), NULL);
		WriteProcessMemory(Process, (void *)(Iw4m2Address + 0x10EE68 + strlen(argv[1])), "\x0", 1, NULL);
	}

EXIT:
	Sleep(10000);
	return 0;
}