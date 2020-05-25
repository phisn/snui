#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

void WINAPI ServiceMain(
	DWORD argc,
	wchar_t** argv);

void ReportWindowsError(const wchar_t* message);
void ReportNumericError(
	const wchar_t* message,
	int error);
void ReportMessages(
	const wchar_t** messages, 
	int messageCount);
