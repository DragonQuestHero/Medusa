#include "Test.h"
#include "MemoryRW.h"
#include "Threads.h"
#include "KernelModules.h"
#include "EmunProcess.h"
#include "Modules.h"
#include "CallBackScanner.h"
#include "SSDT.h"

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
	_KernelModules.GetKernelModuleListALL();
	_KernelModules.GetUnLoadKernelModuleList();
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

void TestCopyMemory(PDRIVER_OBJECT drive_object)
{
	ULONG64 addr = (ULONG64)drive_object->DriverStart + (ULONG64)drive_object->DriverSize - PAGE_SIZE;
	void* temp_buffer = new char[PAGE_SIZE * 10];
	RtlZeroMemory(temp_buffer, PAGE_SIZE * 10);
	SIZE_T NumberOfBytesTransferred = 0;
	MM_COPY_ADDRESS SourceAddress;
	SourceAddress.VirtualAddress = (PVOID)addr;
	NTSTATUS status = MmCopyMemory(temp_buffer, SourceAddress, PAGE_SIZE * 10, MM_COPY_MEMORY_VIRTUAL, &NumberOfBytesTransferred);
	if (NT_SUCCESS(status))
	{

	}
	else
	{

	}
	delete temp_buffer;
}

void TestCallBack()//²»Ö§³Öwin7
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestModules start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
	CallBackScanner _CallBackScanner;
	_CallBackScanner.GetALLCallBackList();

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", _CallBackScanner._ObjectCallBackList.size());
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}

void TestSSDT()
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestSSDT start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
	SSDT _SSDT;
	_SSDT.GetAllSSDT();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "_SSDT._SSDTALL.size():%llx\n", _SSDT._SSDTALL.size());
	_SSDT.GetAllShadowSSDT();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "_SSDT._SSSDTALL.size():%llx\n", _SSDT._SSSDTALL.size());

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}


void TestALL(PDRIVER_OBJECT drive_object)
{
	DbgBreakPoint();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TestALL start\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");



	/*TestWalkStack();
	TestReadKernelMemory();
	TestGetKernel(drive_object);
	TestProcess();
	TestModules();
	TestCallBack();*/
	TestSSDT();

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "=====================================\n");
}

