#include "Test.h"
#include "MemoryRW.h"
#include "Threads.h"
#include "KernelModules.h"
#include "EmunProcess.h"
#include "Modules.h"

#include <string>


void TestWalkStack()
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestWalkStack start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");

	Threads _Threads;
	_Threads.InitWin32StartAddressOffset();

	EmunProcess _EmunProcess;
	_EmunProcess.EmunProcessALL();
	for (auto x : _EmunProcess._Process_List)
	{
		std::vector<ThreadList> temp_vector = _Threads.GetThreadListR0(x.PID);
		for (auto y : temp_vector)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TID==%d\n", y.TID);
			std::vector<ULONG64> temp_walk_vector = _Threads.StackWalkThread(y.TID);
			for (auto n : temp_walk_vector)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", n);
			}
		}
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}

void TestReadKernelMemory()
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestReadKernelMemory start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");

	ULONG64 temp = 0;
	KernelSafeReadMemoryIPI((ULONG64)&TestReadKernelMemory, &temp, 8);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", temp);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}

void TestGetKernel(PDRIVER_OBJECT drive_object)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestGetKernel start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
	KernelModules _KernelModules;
	_KernelModules.GetKernelModuleList4Quick();
	_KernelModules.GetKernelModuleListALL(drive_object);
	_KernelModules.GetUnLoadKernelModuleList(drive_object);
	for (auto x : _KernelModules._KernelModuleList)
	{
		char* temp = new char[x.Size];
		ULONG64 ret = _KernelModules.DumpDriver(x.Addr, temp);
		delete temp;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", ret);
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}


void TestProcess()
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestProcess start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");

	EmunProcess _EmunProcess;
	_EmunProcess.EmunProcessALL();
	for (auto x: _EmunProcess._Process_List)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", x.PID);
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}

void TestModules()
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestModules start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");

	EmunProcess _EmunProcess;
	_EmunProcess.EmunProcessALL();
	for (auto x : _EmunProcess._Process_List)
	{
		Modules _Modules;
		_Modules.GetWin32MoudleList(x.PID);
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}

void TestALL(PDRIVER_OBJECT drive_object)
{
	DbgBreakPoint();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestALL start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");



	TestWalkStack();
	TestReadKernelMemory();
	TestGetKernel(drive_object);
	TestProcess();
	TestModules();


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}