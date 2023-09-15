#include "Test.h"



void TestWalkStack()
{
	DbgBreakPoint();

	Threads _Threads;
	_Threads.InitWin32StartAddressOffset();
	
	PETHREAD tempthd;
	NTSTATUS status;
	for (int i = 1000; i < 65535; i = i + 4)
	{
		status = PsLookupThreadByThreadId((HANDLE)i, &tempthd);
		if (NT_SUCCESS(status))
		{
			_Threads.StackWalkThread(i);
		}
	}
}