#include "common.h"
#include "logger.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <iostream>

void ReportWindowsEvent(const char** messages, int messageCount)
{
	HANDLE source = RegisterEventSourceA(NULL, SNUI_APP_NAME);

	if (source)
	{
		ReportEventA(
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

void LogMessage(const char* message)
{
	if (IsWindowVisible(GetConsoleWindow()))
	{
		std::cerr << message << std::endl;
	}
	else
	{
		const char* cmessage = message;
		ReportWindowsEvent(&cmessage, 1);
	}
}

void NumLogMessage(const char* message, int code)
{
	if (IsWindowVisible(GetConsoleWindow()))
	{
		std::cerr << message << " (" << code << ")" << std::endl;
	}
	else
	{
		char codeMessage[20];
		sprintf_s(codeMessage, "Error code: %d", code);
		
		const char* messages[] =
		{
			codeMessage,
			message
		};

		ReportWindowsEvent(messages, 2);
	}
}

void WinLogMessage(const char* message)
{
	NumLogMessage(message, GetLastError());
}
