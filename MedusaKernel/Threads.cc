#include "Threads.h"

#include "Modules.h"

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


bool Threads::StackWalkThread(ULONG64 TID)
{
	PETHREAD tempthd = nullptr;
	NTSTATUS status = PsLookupThreadByThreadId((HANDLE)TID, &tempthd);
	if (!NT_SUCCESS(status))
	{
		return false;
	}
	ObDereferenceObject(tempthd);

	
	PEPROCESS tempeps;
	ULONG64 PID = (ULONG64)PsGetThreadProcessId(tempthd);
	status = PsLookupProcessByProcessId((HANDLE)PID, &tempeps);
	if (!NT_SUCCESS(status))
	{
		return false;
	}
	ObDereferenceObject(tempeps);

	if (PsIsThreadTerminating(tempthd))
		return false;


	status = PsSuspendProcess(tempeps);
	if (!NT_SUCCESS(status))
	{
		return false;
	}


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
		PsResumeProcess(tempeps);
		return false;
	}
	
	KAPC_STATE ApcState;
	KeStackAttachProcess(tempeps, &ApcState);
	auto CurrentStackSize = StackBase - CurrentStackLocation;
	if (CurrentStackLocation > StackLimit && CurrentStackLocation < StackBase)
	{
		if (MmIsAddressValid((PVOID)CurrentStackLocation) && MmIsAddressValid((PVOID)(CurrentStackLocation + CurrentStackSize-0x8)))
		{
			temp_stack = ExAllocatePool(NonPagedPool, CurrentStackSize);
			RtlCopyMemory(temp_stack, (PVOID)CurrentStackLocation, CurrentStackSize-0x8);
		}
	}
	KeUnstackDetachProcess(&ApcState);

	if (temp_stack)
	{
		ExFreePool(temp_stack);
	}
	PsResumeProcess(tempeps);

	//todo KeResumeThread
	
}