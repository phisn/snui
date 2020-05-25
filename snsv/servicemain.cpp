#include "servicemain.h"
#include "snortnotify.h"

#include <cstdio>

void WINAPI ServiceControl(DWORD dwControl)
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		SnsvStop();
		
		break;
	case SERVICE_CONTROL_INTERROGATE:
		// send current state information

		break;
	default:
		const wchar_t* message = L"Got invalid control in servicecontrol";
		ReportMessages(&message, 1);

		break;
	}
}

bool EnforceServiceStatus(
	SERVICE_STATUS_HANDLE hServiceStatus,
	LPSERVICE_STATUS lpServiceStatus)
{
	if (!SetServiceStatus(hServiceStatus, lpServiceStatus))
	{
		ReportWindowsError(L"Failed to set service status");
		return false;
	}

	return true;
}

void WINAPI ServiceMain(
	DWORD argc,
	wchar_t** argv)
{
	SERVICE_STATUS_HANDLE serviceControl = RegisterServiceCtrlHandlerW(
		L"snsv",
		ServiceControl);

	if (!serviceControl)
	{
		ReportWindowsError(L"Failed to register service control handler");
		return;
	}

	SERVICE_STATUS status = { };
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwWin32ExitCode = NO_ERROR;
	status.dwWaitHint = 500;

	if (!EnforceServiceStatus(serviceControl, &status))
	{
		return;
	}

	if (!SnsvInitialize())
	{
		status.dwCurrentState = SERVICE_STOPPED;
		status.dwWin32ExitCode = GetLastError();
		EnforceServiceStatus(serviceControl, &status);
		return;
	}

	// pending
	status.dwCurrentState = SERVICE_RUNNING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWaitHint = 0;

	if (!EnforceServiceStatus(serviceControl, &status))
	{
		return;
	}

	SnsvRun();

	// when snsvrun stopped with an error
	// this will be reported here
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwWin32ExitCode = GetLastError();
	EnforceServiceStatus(serviceControl, &status);
}

void ReportWindowsError(const wchar_t* message)
{
	ReportNumericError(message, GetLastError());
}

void ReportNumericError(const wchar_t* message, int error)
{
	wchar_t buffer[20] = { };
	swprintf_s(buffer, L"Error: %d", error);

	LPCWSTR messages[2] =
	{
		buffer,
		message
	};

	ReportMessages(messages, 2);
}

void ReportMessages(const wchar_t** messages, int messageCount)
{
	HANDLE source = RegisterEventSourceW(NULL, L"snsv");
	
	if (source)
	{
		ReportEventW(
			source,
			EVENTLOG_ERROR_TYPE,
			0, 0, 
			NULL,
			messageCount,
			0,
			messages,
			NULL);

		DeregisterEventSource(source);
	}
}
