#pragma once

bool SnsvInitialize();
void SnsvRun();

namespace Configuration
{
	extern WORD Port;
}

void SnsvStop();
