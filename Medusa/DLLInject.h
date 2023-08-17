#pragma once
#include <windows.h>


class DLLInject
{
public:
	DLLInject() = default;
	~DLLInject() = default;
public:
	bool R3CreateThread(ULONG64 PID);
	bool R3APCInject(ULONG64 PID);
private:

};

