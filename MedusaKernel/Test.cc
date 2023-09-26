#include "Test.h"
#include "MemoryRW.h"
#include "Threads.h"
#include "KernelModules.h"


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

void TestReadKernelMemory()
{
	DbgBreakPoint();
	ULONG64 temp = 0;
	KernelSafeReadMemoryIPI((ULONG64)&TestReadKernelMemory, &temp, 8);
	int a = 1;
}

void TestGetKernel()
{
	KernelModules _KernelModules;
	DbgBreakPoint();
	_KernelModules.GetKernelModuleList4();
}