#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "common.h"
#include "configuration.h"
#include "logger.h"
#include "nui.h"

#include <Windows.h>
#include <shellapi.h>
#include <TlHelp32.h>

#include <cassert>
#include <iostream>
#include <string>

void PrintUsage(char* filename)
{
	std::cout
		<< "snort notify user interface (for windows)\n\n"
		<< "Usage: " << filename << " [-options]\n"
		<< "       " << filename << " install [-options]\n"
		<< "       " << filename << " uninstall\n"
		<< "Options:\n"
		<< "       -p        snort alert_fast path\n"
		<< "       -r [1000] refresh rate for alert checking\n"
		<< "       -h        hides console window\n"
		<< std::endl;
}

bool MakeAdvancedPath(
	char* path,
	int argc,
	char* argv[])
{
	if (!GetModuleFileNameA(NULL, path, MAX_PATH))
	{
		WinLogMessage("Failed to aquire path");
		return false;
	}

	size_t position = strlen(path);
	for (int i = 0; i < argc; ++i)
	{
		int result = snprintf(path + position, MAX_PATH - position, " %s", argv[i]);

		if (result < 0)
		{
			NumLogMessage("Failed to create path", result);
			return false;
		}

		position += result;
	}

	return true;
}

bool HandleInstall(int argc, char* argv[]);
bool HandleUninstall();

bool HandleArguments(int argc, char* argv[])
{
	if (argc == 1)
	{
		PrintUsage(argv[0]);
		return false;
	}

	if (_stricmp("install", argv[1]) == 0)
	{
		return HandleInstall(argc - 2, argv + 2);
	}

	if (_stricmp("uninstall", argv[1]) == 0)
	{
		if (argc > 2)
		{
			PrintUsage(argv[0]);
			LogMessage("Invalid number of arguments");
			return false;
		}

		return HandleUninstall();
	}

	Configuration::hideConsole = false;

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-')
		{
			LogMessage("Got invalid argument");
			return false;
		}

		switch (argv[i][1])
		{
		case 'p':
			if (++i != argc)
			{
				Configuration::alertPath = argv[i];
			}

			break;
		case 'r':
			if (++i != argc)
			{
				Configuration::refreshRate = std::stoi(argv[i]);
			}

			break;
		case 'h':
			Configuration::hideConsole = true;

			break;
		}

		if (i == argc)
		{
			LogMessage("Got invalid number of arguments");
			return false;
		}
	}

	if (Configuration::alertPath.size() == 0)
	{
		LogMessage("FastAlert path missing");
		return false;
	}

	if (Configuration::refreshRate <= 0)
	{
		Configuration::refreshRate = 1000;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (!HandleArguments(argc, argv))
	{
		return 0;
	}

	std::cout << "Started listening to " << Configuration::alertPath.c_str() << " at " << Configuration::refreshRate << std::endl;

	if (Configuration::hideConsole)
	{
		ShowWindow(::GetConsoleWindow(), SW_HIDE);
	}

	HANDLE alertFile = CreateFileA(
		Configuration::alertPath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (alertFile == INVALID_HANDLE_VALUE)
	{
		WinLogMessage("Failed to open alert file");
		return 1;
	}

	LARGE_INTEGER lastFileSize;
	if (!GetFileSizeEx(alertFile, &lastFileSize))
	{
		WinLogMessage("Failed to get initial filetime");
		return 1;
	}

	if (!nui::initialize())
	{
		return 1;
	}

	while (true)
	{
		Sleep(Configuration::refreshRate);

		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(alertFile, &fileSize))
		{
			WinLogMessage("Failed to get filetime");
			return 1;
		}

		if (fileSize.QuadPart != lastFileSize.QuadPart)
		{
			if (!nui::notify())
				return 1;

			lastFileSize = fileSize;
		}
	}
}

bool HandleInstall(int argc, char* argv[])
{
	char path[MAX_PATH];
	if (!MakeAdvancedPath(path, argc, argv))
	{
		return false;
	}

	errno_t catResult = strcat_s(path, " -h");
	if (catResult)
	{
		NumLogMessage("Failed to create path", catResult);
		return false;
	}

	HKEY autorunKey;
	LSTATUS result = RegCreateKeyExA(
		HKEY_CURRENT_USER,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, NULL, 0,
		KEY_ALL_ACCESS,
		NULL,
		&autorunKey,
		NULL);
	if (result != ERROR_SUCCESS)
	{
		NumLogMessage("Failed to create registry key", result);
		return false;
	}

	result = RegSetValueExA(
		autorunKey,
		SNUI_APP_NAME,
		0,
		REG_SZ,
		(BYTE*) path,
		(DWORD) strlen(path));

	RegCloseKey(autorunKey);

	if (result != ERROR_SUCCESS)
	{
		NumLogMessage("Failed to change registry key", result);
		return false;
	}

	std::cout << "Successfully installed" << std::endl;
	system(path);

	return true;
}

bool HandleUninstall()
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snapshot == INVALID_HANDLE_VALUE)
	{
		WinLogMessage("Failed to aquire process list");
		return false;
	}



	// find process

	// kill process

	HKEY autorunKey;
	LSTATUS result = RegOpenKeyA(
		HKEY_CURRENT_USER,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		&autorunKey);
	if (result != ERROR_SUCCESS)
	{
		NumLogMessage("Failed to open registry key", result);
		return false;
	}

	result = RegDeleteValueA(
		autorunKey,
		SNUI_APP_NAME);
	if (result != ERROR_SUCCESS)
	{
		NumLogMessage("Failed to delete registry value", result);
		return false;
	}

	std::cout << "Successfully uninstalled" << std::endl;

	return false;
}
