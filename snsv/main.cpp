#define _CRT_SECURE_NO_WARNINGS

#include "Common.h"
#include "servicemain.h"

#include <cstring>
#include <iostream>

void printUsage(wchar_t* filename)
{
	std::wcout
		<< L"snort notify user interface (for windows)\n\n"
		<< L"Usage: " << filename << " install [-options]\n"
		<< L"       " << filename << " change  [-options]\n"
		<< L"       " << filename << " remove\n"
		<< L"       " << filename << " status\n"
		<< L"Options:\n"
		<< L"       -p [507] snort port\n"
		<< std::endl;
}

int handleInstall(int argc, wchar_t* argv[]);
int handleChange(int argc, wchar_t* argv[]);
int handleRemove();
int handleStatus();

int handleArguments(int argc, wchar_t* argv[])
{
	if (argc == 1)
	{
		printUsage(argv[0]);
		return 0;
	}

	if (_wcsicmp(argv[1], L"install") == 0)
	{
		handleInstall(argc - 2, argv + 2);
		return 0;
	}

	if (_wcsicmp(argv[1], L"change") == 0)
	{
		handleChange(argc - 2, argv + 2);
		return 0;
	}

	if (argc > 2)
	{
		printUsage(argv[0]);
		std::wcout << L"Got invalid arguments" << std::endl;
		return -3;
	}

	if (_wcsicmp(argv[1], L"remove") == 0)
	{
		handleRemove();
		return 0;
	}

	if (_wcsicmp(argv[1], L"status") == 0)
	{
		handleStatus();
		return 0;
	}

	printUsage(argv[0]);
	std::wcout << L"Got invalid option" << std::endl;
	return -2;
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc > 1)
	{
		return handleArguments(argc, argv);
	}

	SERVICE_TABLE_ENTRYW serviceTable;
	
	serviceTable.lpServiceName = (LPWSTR) SNUI_SERVICE_NAME;
	serviceTable.lpServiceProc = ServiceMain;

	if (!StartServiceCtrlDispatcherW(&serviceTable))
	{
		int result = GetLastError();
		if (result != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			ReportWindowsError(L"Failed to start service control dispatcher");
			return result;
		}

		printUsage(argv[0]);
		return -1;
	}

	return 0;
}

int makeAdvancedPath(
	wchar_t* path, 
	int argc, 
	wchar_t* argv[])
{
	if (!GetModuleFileNameW(NULL, path, MAX_PATH))
	{
		std::wcout << L"Failed to aquire path (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	int position = wcslen(path);
	for (int i = 0; i < argc; ++i)
	{
		int result = _snwprintf(path + position, MAX_PATH - position, L" %s", argv[i]);

		if (result < 0)
		{
			std::wcout << L"Failed to create path (" << result << L")" << std::endl;
			return -3;
		}

		position += result;
	}

	return 0;
}

int handleInstall(int argc, wchar_t* argv[])
{
	wchar_t path[MAX_PATH] = { };

	int result = makeAdvancedPath(path, argc, argv);
	if (result != ERROR_SUCCESS)
	{
		return result;
	}

	SC_HANDLE scManager = OpenSCManagerW(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (!scManager)
	{
		std::wcout << L"Failed to open scmanager (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	SC_HANDLE newService = CreateServiceW(
		scManager,                 // SCM database 
		SNUI_SERVICE_NAME,         // name of service 
		SNUI_SERVICE_DISPLAY,      // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		path,                      // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 
	int temp_error = GetLastError();
		
	CloseServiceHandle(scManager);

	if (!newService)
	{
		switch (temp_error)
		{
		case ERROR_SERVICE_MARKED_FOR_DELETE:
			std::wcout << L"Please close the service manager before continuing" << std::endl;

			break;
		case ERROR_SERVICE_EXISTS:
			std::wcout << L"Service already exists" << std::endl;

			break;
		default:
			std::wcout << L"failed to install service (" << temp_error << L")" << std::endl;

			break;
		}

		return temp_error;
	}

	std::wcout << L"service installed" << std::endl;
	CloseServiceHandle(newService);

	return 0;
}

int handleChange(int argc, wchar_t* argv[])
{
	wchar_t path[MAX_PATH] = { };

	int result = makeAdvancedPath(path, argc, argv);
	if (result != ERROR_SUCCESS)
	{
		return result;
	}

	SC_HANDLE scManager = OpenSCManagerW(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (!scManager)
	{
		std::wcout << L"Failed to open scmanager (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	SC_HANDLE service = OpenServiceW(
		scManager,
		SNUI_SERVICE_NAME,
		SERVICE_CHANGE_CONFIG);

	if (!service)
	{
		CloseServiceHandle(scManager);
		std::wcout << L"Failed to open service (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	result = ChangeServiceConfigW(
		service,                   // SCM database 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		path,                      // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL,                      // no password
		SNUI_SERVICE_DISPLAY);
	int temp_error = GetLastError();

	CloseServiceHandle(service);
	CloseServiceHandle(scManager);

	if (!result)     // service name to display 
	{
		std::wcout << L"Failed to change service config (" << temp_error << L")" << std::endl;
		return temp_error;
	}

	std::wcout << L"Service config changed" << std::endl;

	return 0;
}

int handleRemove()
{
	SC_HANDLE scManager = OpenSCManagerW(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (!scManager)
	{
		std::wcout << L"Failed to open scmanager (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	SC_HANDLE service = OpenServiceW(
		scManager,
		SNUI_SERVICE_NAME,
		DELETE);

	if (!service)
	{
		CloseServiceHandle(scManager);
		std::wcout << L"Failed to open service (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	bool result = DeleteService(service);
	int temp_error = GetLastError();

	CloseServiceHandle(service);
	CloseServiceHandle(scManager);

	if (!result)
	{
		std::wcout << L"Failed to remove service (" << temp_error << L")" << std::endl;
		return temp_error;
	}

	std::wcout << L"Service removed" << std::endl;

	return 0;
}

int handleStatus()
{

	SC_HANDLE scManager = OpenSCManagerW(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (!scManager)
	{
		std::wcout << L"Failed to open scmanager (" << GetLastError() << L")" << std::endl;
		return GetLastError();
	}

	return 0;
}
