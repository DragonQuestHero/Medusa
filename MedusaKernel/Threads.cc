#include "Threads.h"

#include "Modules.h"
#include "CRT/NtSysAPI_Func.hpp"


bool Threads::InitWin32StartAddressOffset()
{
	bool first = false;
	HANDLE thread_handle;
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, 0, 0, 0, 0);
	CLIENT_ID cid = { 0, PsGetCurrentThreadId() };
	if (NT_SUCCESS(ZwOpenThread(&thread_handle, 0x0040, &ObjectAttributes, &cid)))
	{
		void* startaddr = nullptr;
		NTSTATUS status = ZwQueryInformationThread(thread_handle,
			ThreadQuerySetWin32StartAddress, &startaddr, sizeof(startaddr), NULL);
		if (NT_SUCCESS(status))
		{
			PETHREAD temp_thread = PsGetCurrentThread();
			for (int i = 0; i < 0x800; i = i + 8)
			{
				ULONG64 temp_win32start = *(ULONG64*)((char*)temp_thread + i);
				if (temp_win32start == (ULONG64)startaddr)
				{
					if (first)
					{
						StartAddressOffset = i;
					}
					else
					{
						Win32StartAddressOffset = i;
					}
					/*ZwClose(thread_handle);//故意不跳出去
					return true;*/
				}
			}
		}
		ZwClose(thread_handle);
	}
	return false;
}

std::vector<ThreadList> Threads::GetThreadListR0(ULONG64 PID)
{
	std::vector<ThreadList> temp_vector;
	PETHREAD tempthd;
	NTSTATUS status;
	for (int i = 8; i < 65535; i = i + 4)
	{
		status = PsLookupThreadByThreadId((HANDLE)i, &tempthd);
		if (NT_SUCCESS(status))
		{
			ObDereferenceObject(tempthd);
			ULONG64 PID2 = (ULONG64)PsGetThreadProcessId(tempthd);
			if (PID2 == PID)
			{
				ThreadList temp_list;
				RtlZeroMemory(&temp_list, sizeof(ThreadList));
				temp_list.TID = i;
				temp_list.ETHREAD = (ULONG64)tempthd;
				temp_list.StartAddr = *(ULONG64*)((char*)tempthd + Win32StartAddressOffset);
				Modules _Modules;
				std::vector<UserModule> temp_module_vector = _Modules.GetWin32MoudleList(PID);
				if (temp_module_vector.size() != 0)
				{
					for (auto x : temp_module_vector)
					{
						if ((ULONG64)temp_list.StartAddr >= (ULONG64)x.Addr &&
							(ULONG64)temp_list.StartAddr <= (ULONG64)x.Addr + x.Size)
						{
							RtlCopyMemory(temp_list.Name, x.Name, 260);
							break;
						}
					}
				}
				temp_vector.push_back(temp_list);
			}
		}
	}

	return temp_vector;
}


struct __declspec(align(8)) FunctionEntryForStackStruct
{
	ULONG* ExpectionTableEntry;
	PVOID ModuleBase;
	ULONG ModuleSize;
	ULONG UnknownTableIndex;
	PVOID Unknown_1;
};

std::vector<ULONG64> Threads::StackWalkThreadUser(ULONG64 TID)
{
	std::vector<ULONG64> temp_walk_vector;

	PETHREAD tempthd = nullptr;
	NTSTATUS status = PsLookupThreadByThreadId((HANDLE)TID, &tempthd);
	if (!NT_SUCCESS(status))
	{
		return temp_walk_vector;
	}
	ObDereferenceObject(tempthd);

	
	PEPROCESS tempeps;
	ULONG64 PID = (ULONG64)PsGetThreadProcessId(tempthd);
	if (PID == 0 || PID == 4)
	{
		return temp_walk_vector;
	}
	status = PsLookupProcessByProcessId((HANDLE)PID, &tempeps);
	if (!NT_SUCCESS(status))
	{
		return temp_walk_vector;
	}
	ObDereferenceObject(tempeps);

	if (PsIsThreadTerminating(tempthd))
		return temp_walk_vector;

	status = PsSuspendProcess(tempeps);
	if (!NT_SUCCESS(status))
	{
		return temp_walk_vector;
	}

	KAPC_STATE ApcState;
	KeStackAttachProcess(tempeps, &ApcState);


	CONTEXT ThreadContext = { 0 };

	auto CurrentThread = (ULONG64)PsGetCurrentThread();
	void* temp_stack = nullptr;

	auto CurrentStack = (ULONG64)PsGetCurrentThreadStackBase();
	ULONG ThreadStackBaseOffset = NULL;
	while (*(ULONG64*)(CurrentThread + ThreadStackBaseOffset) != CurrentStack)
		ThreadStackBaseOffset += 8;

	CurrentStack = (ULONG64)PsGetCurrentThreadStackLimit();
	ULONG ThreadStackLimitOffset = NULL;
	while (*(uint64_t*)(CurrentThread + ThreadStackLimitOffset) != CurrentStack)
		ThreadStackLimitOffset += 8;


	CurrentStack = (ULONG64)IoGetInitialStack();
	ULONG InitialThreadStackOffset = NULL;
	while (*(uint64_t*)(CurrentThread + InitialThreadStackOffset) != CurrentStack)
		InitialThreadStackOffset += 8;

	auto StackBase = *(uint64_t*)((uint64_t)tempthd + ThreadStackBaseOffset);
	auto StackLimit = *(uint64_t*)((uint64_t)tempthd + ThreadStackLimitOffset);
	auto InitialStack = *(uint64_t*)((uint64_t)tempthd + InitialThreadStackOffset);

	ULONG CurrentStackLocationOffset = 0;
	while (CurrentStackLocationOffset < 0x2F8)
	{
		if (CurrentStackLocationOffset != InitialThreadStackOffset &&
			*(uint64_t*)((uint64_t)tempthd + CurrentStackLocationOffset) < StackBase &&
			*(uint64_t*)((uint64_t)tempthd + CurrentStackLocationOffset) > StackLimit)
		{
			break;
		}
		CurrentStackLocationOffset += 8;
	}

	auto CurrentStackLocation = *(uint64_t*)((uint64_t)tempthd + CurrentStackLocationOffset);
	if (!StackBase || !StackLimit || !InitialStack || !CurrentStackLocation)
	{
		KeUnstackDetachProcess(&ApcState);
		PsResumeProcess(tempeps);
		return temp_walk_vector;
	}

	auto CurrentStackSize = StackBase - CurrentStackLocation;
	if (CurrentStackLocation > StackLimit && CurrentStackLocation < StackBase)
	{
		if (MmIsAddressValid((PVOID)CurrentStackLocation) && MmIsAddressValid((PVOID)(CurrentStackLocation + CurrentStackSize - 0x8)))
		{
			temp_stack = ExAllocatePool(NonPagedPool, CurrentStackSize);
			RtlZeroMemory(temp_stack, CurrentStackSize);
			SIZE_T NumberOfBytesTransferred;
			MM_COPY_ADDRESS SourceAddress;
			SourceAddress.VirtualAddress = (PVOID)CurrentStackLocation;
			status = MmCopyMemory(temp_stack, SourceAddress, CurrentStackSize - 0x8, MM_COPY_MEMORY_VIRTUAL, &NumberOfBytesTransferred);
			if (!NT_SUCCESS(status))
			{
				if (temp_stack)
				{
					ExFreePool(temp_stack);
				}
				KeUnstackDetachProcess(&ApcState);
				PsResumeProcess(tempeps);
				return temp_walk_vector;
			}
			//RtlCopyMemory(temp_stack, (PVOID)CurrentStackLocation, CurrentStackSize - 0x8);



			ThreadContext.Rip = *(uint64_t*)((char*)temp_stack + 0x38);
			ThreadContext.Rsp = (uint64_t)((char*)temp_stack + 0x40);

			PVOID HandlerData;
			DWORD64 EstablisherFrame;
			FunctionEntryForStackStruct ImageBase2 = { 0 };
			PRUNTIME_FUNCTION FunctionEntry = nullptr;
			int Index = 0;
			while ((Index < 256) && (ThreadContext.Rip != 0))
			{
				ULONG64 ImageBase = 0;
				FunctionEntry = RtlLookupFunctionEntry(ThreadContext.Rip, &ImageBase, NULL);
				if (FunctionEntry == NULL)
				{
					using func = PRUNTIME_FUNCTION(*)(ULONG64, FunctionEntryForStackStruct*);
					func temp_func = (func)0xfffff80128aadbe0;//RtlpLookupFunctionEntryForStackWalks
					FunctionEntry = temp_func(ThreadContext.Rip,
						&ImageBase2);
					ImageBase = (ULONG64)ImageBase2.ModuleBase;
				}
				if (FunctionEntry != NULL)
				{
					RtlVirtualUnwind(0,
						ImageBase,
						ThreadContext.Rip,
						FunctionEntry,
						&ThreadContext,
						&HandlerData,
						&EstablisherFrame,
						NULL);
					if (ThreadContext.Rip)
					{
						temp_walk_vector.push_back(ThreadContext.Rip);
					}
					Index++;
					//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", *(ULONG64*)((ULONG64)ThreadContext.Rsp - 8));
				}
				else
				{
					break;
				}
			}
		}
	}


	if (temp_stack)
	{
		ExFreePool(temp_stack);
	}
	KeUnstackDetachProcess(&ApcState);
	PsResumeProcess(tempeps);
	return temp_walk_vector;

	//todo KeResumeThread
	
}



//函数取rip 只能取r3
//CONTEXT * BaseAddress = 0;
 //   SIZE_T Size = sizeof(CONTEXT);
 //   status = ZwAllocateVirtualMemory(NtCurrentProcess(), (void**)&BaseAddress, 0, &Size, MEM_COMMIT, PAGE_READWRITE);
 //   if (0 <= status)
 //   {
 //       BaseAddress->ContextFlags = CONTEXT_ALL;
 //       if (0 <= (status = PsGetContextThread(tempthd, BaseAddress, UserMode)))
 //       {
 //           memcpy(&ThreadContext, BaseAddress, sizeof(CONTEXT));
 //       }
 //       ZwFreeVirtualMemory(NtCurrentProcess(), (void**)&BaseAddress, &Size, MEM_RELEASE);
 //   }
	//////status = PsGetContextThread(tempthd, &ThreadContext, KernelMode);
	//if (!NT_SUCCESS(status))
	//{
	//	KeUnstackDetachProcess(&ApcState);
	//	PsResumeProcess(tempeps);
	//	return temp_walk_vector;
	//}


	//PVOID HandlerData;
	//DWORD64 EstablisherFrame;
	//FunctionEntryForStackStruct ImageBase2 = { 0 };
	//PRUNTIME_FUNCTION FunctionEntry = nullptr;
	//int Index = 0;
	//while ((Index < 256) && (ThreadContext.Rip != 0))
	//{
	//	using func = PRUNTIME_FUNCTION(*)(ULONG64, FunctionEntryForStackStruct*);
	//	func temp_func = (func)0xfffff80128aadbe0;//RtlpLookupFunctionEntryForStackWalks
	//	FunctionEntry = temp_func(ThreadContext.Rip,
	//		&ImageBase2);
	//	/*ULONG64 ImageBase = 0;
	//	FunctionEntry = RtlLookupFunctionEntry(ThreadContext.Rip, &ImageBase, NULL);*/
	//	if ((FunctionEntry != NULL))
	//	{
	//		RtlVirtualUnwind(0,
	//			(ULONG64)ImageBase2.ModuleBase,
	//			ThreadContext.Rip,
	//			FunctionEntry,
	//			&ThreadContext,
	//			&HandlerData,
	//			&EstablisherFrame,
	//			NULL);
	//		if (MmIsAddressValid((char*)ThreadContext.Rsp - 8))
	//		{
	//			if (*(ULONG64*)((ULONG64)ThreadContext.Rsp - 8) != 0)
	//			{
	//				temp_walk_vector.push_back(*(ULONG64*)((ULONG64)ThreadContext.Rsp - 8));
	//			}
	//		}
	//		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%llx\n", *(ULONG64*)((ULONG64)ThreadContext.Rsp - 8));
	//	}
	//	else 
	//	{
	//		break;
	//	}
	//}
	//PsResumeProcess(tempeps);
	//KeUnstackDetachProcess(&ApcState);
	//return temp_walk_vector;