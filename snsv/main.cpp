#include "servicemain.h"

#include <iostream>

void printUsage(wchar_t* filename)
{
	std::wcout << L"Usage: " << filename << " [-x]...\n"
		<< L" as process:\n"
		<< L"   -i install service\n"
		<< L"   -u uninstall service\n"
		<< L" as service:\n"
		<< L"   -p snort port\n"
		//			<< L"\n" 
		<< std::endl;
}

int handleArguments(int argc, wchar_t* argv[])
{

}

int wmain(int argc, wchar_t* argv[])
{
	if (argc > 1)
	{
		return handleArguments(argc, argv);
	}

	SERVICE_TABLE_ENTRYW serviceTable;
	
	serviceTable.lpServiceName = (LPWSTR) L"snsv";
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
