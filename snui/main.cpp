#define WIN32_LEAN_AND_MEAN

#include "Common.h"
#include "FastAlert.h"

#include <Windows.h>

#include <cassert>
#include <iostream>

namespace Configuration
{
	std::string alertPath;
	int refreshRate;
}

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
		<< std::endl;
}

int MakeAdvancedPath(
	char* path,
	int argc,
	char* argv[])
{
	if (!GetModuleFileNameA(NULL, path, MAX_PATH))
	{
		std::cout << "Failed to aquire path (" << GetLastError() << ")" << std::endl;
		return GetLastError();
	}

	int position = strlen(path);
	for (int i = 0; i < argc; ++i)
	{
		int result = snprintf(path + position, MAX_PATH - position, " %s", argv[i]);

		if (result < 0)
		{
			std::cout << "Failed to create path (" << result << ")" << std::endl;
			return -3;
		}

		position += result;
	}

	return 0;
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
			std::cout << "Invalid number of arguments" << std::endl;
			return false;
		}

		return HandleUninstall();
	}

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-')
		{
			std::cout << "Got invalid argument" << std::endl;
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
		}

		if (i == argc)
		{
			std::cout << "Got invalid number of arguments" << std::endl;
			return false;
		}
	}

	if (Configuration::alertPath.size() == 0)
	{
		std::cout << "FastAlert path missing" << std::endl;
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
		std::cout << "Failed to open alert file (" << GetLastError() << ")" << std::endl;
		return -3;
	}

	LARGE_INTEGER lastFileSize;
	if (!GetFileSizeEx(alertFile, &lastFileSize))
	{
		std::cout << "Failed to get initial filetime (" << GetLastError() << ")" << std::endl;
		return -4;
	}

	while (true)
	{
		Sleep(Configuration::refreshRate);

		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(alertFile, &fileSize))
		{
			std::cout << "Failed to get filetime (" << GetLastError() << ")" << std::endl;
			return -6;
		}

		if (fileSize.QuadPart != lastFileSize.QuadPart)
		{


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
		std::cout << "Failed to create registry key (" << result << ")" << std::endl;
		return false;
	}

	result = RegSetValueExA(
		autorunKey,
		SNUI_APP_NAME,
		0,
		REG_SZ,
		(BYTE*)path,
		strlen(path));

	RegCloseKey(autorunKey);

	if (result != ERROR_SUCCESS)
	{
		std::cout << "Failed to change registry key (" << result << ")" << std::endl;
		return false;
	}

	std::cout << "Successfully installed" << std::endl;
	Configuration::alertPath = argv[2];

	return true;
}

bool HandleUninstall()
{
	// find process
	// kill process

	HKEY autorunKey;
	LSTATUS result = RegOpenKeyA(
		HKEY_CURRENT_USER,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		&autorunKey);
	if (result != ERROR_SUCCESS)
	{
		std::cout << "Failed to open registry key (" << result << ")" << std::endl;
		return false;
	}

	result = RegDeleteValueA(
		autorunKey,
		SNUI_APP_NAME);
	if (result != ERROR_SUCCESS)
	{
		std::cout << "Failed to delete registry value (" << result << ")" << std::endl;
		return false;
	}

	std::cout << "Successfully uninstalled" << std::endl;

	return false;
}
