#include "servicemain.h"
#include "snortnotify.h"

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

WSADATA winsockData;
SOCKET listenSocket, snortSocket;

namespace Configuration
{
	WORD Port = 507;
}

bool SnsvInitialize()
{
	int result = WSAStartup(MAKEWORD(2, 2), &winsockData);
	if (!result)
	{
		ReportNumericError(L"WSAStartup failed", result);
		return false;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		ReportWindowsError(L"Socket creation failed");
		return false;
	}

	sockaddr_in addr = { };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Configuration::Port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenSocket, (sockaddr*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		ReportWindowsError(L"Socket bind failed");
		return false;
	}

	return true;
}

bool running = true;

void SnsvRun()
{
	while (running)
	{

	}
}

void SnsvStop()
{
	running = false;
	// ... notify snsvrun socket (close)
}
