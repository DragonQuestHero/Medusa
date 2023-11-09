#include "Modules.h"
#include "DLLInjectShellCode.h"



std::vector<UserModule> Modules::GetWin32MoudleList(ULONG64 PID)
{
	std::vector<UserModule> temp_vector;
	PEPROCESS   tempep = NULL;
	KAPC_STATE   kapc = { 0 };
	KIRQL        tempirql = 0;
	PPEB      peb = NULL;
	PPEB_LDR_DATA	pPebLdrData = NULL;
	PLIST_ENTRY		pListEntryStart = NULL;
	PLIST_ENTRY		pListEntryEnd = NULL;
	PLDR_DATA_TABLE_ENTRYB pLdrDataEntry = NULL;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)PID, &tempep);
	if (!NT_SUCCESS(status))
	{
		return temp_vector;
	}
	ObDereferenceObject(tempep);
	KeStackAttachProcess(tempep, &kapc);
	peb = (PPEB)PsGetProcessPeb(tempep);
	if (!peb || !MmIsAddressValid(peb))
	{
		KeUnstackDetachProcess(&kapc);
		return temp_vector;
	}
	if (!MmIsAddressValid(&peb->Ldr))
	{
		return temp_vector;
	}
	pPebLdrData = (PPEB_LDR_DATA)peb->Ldr;
	if (!pPebLdrData)
	{
		return temp_vector;
	}
	pListEntryStart = pListEntryEnd = pPebLdrData->InMemoryOrderModuleList.Blink;
	do
	{
		pLdrDataEntry = (PLDR_DATA_TABLE_ENTRYB)CONTAINING_RECORD(pListEntryStart, LDR_DATA_TABLE_ENTRYB, InMemoryOrderLinks);
		UserModule temp_list;
		RtlZeroMemory(&temp_list, sizeof(UserModule));
		temp_list.Addr = (ULONG64)pLdrDataEntry->DllBase;
		temp_list.Size = pLdrDataEntry->SizeOfImage;
		if (pLdrDataEntry->BaseDllName.Buffer != NULL)
		{
			RtlCopyMemory(temp_list.Name, pLdrDataEntry->BaseDllName.Buffer, pLdrDataEntry->BaseDllName.MaximumLength);
		}
		if (pLdrDataEntry->FullDllName.Buffer != NULL)
		{
			RtlCopyMemory(temp_list.Path, pLdrDataEntry->FullDllName.Buffer, pLdrDataEntry->FullDllName.MaximumLength);
		}
		pListEntryStart = pListEntryStart->Blink;
		if (pListEntryStart != pListEntryEnd)
		{
			temp_vector.push_back(temp_list);
		}
	} while (pListEntryStart != pListEntryEnd);

	KeUnstackDetachProcess(&kapc);
	return temp_vector;
}

bool Modules::R0MapInject(ULONG64 PID, ULONG64 Size, void* DLLImage)
{
	PEPROCESS tempep;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)PID, &tempep);
	if (NT_SUCCESS(status))
	{
		ObDereferenceObject(tempep);
		KAPC_STATE kapc;
		KeStackAttachProcess(tempep, &kapc);
		void* buffer = nullptr;
		void* shellcode = nullptr;
		ULONG64 shellcode_size = sizeof(MemLoadShellcode_x64);
		ULONG64 buffer_size = Size;
		status = ZwAllocateVirtualMemory(ZwCurrentProcess(), &shellcode, 0, &shellcode_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!NT_SUCCESS(status))
		{
			KeUnstackDetachProcess(&kapc);
			return false;
		}
		status = ZwAllocateVirtualMemory(ZwCurrentProcess(), &buffer, 0, &buffer_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!NT_SUCCESS(status))
		{
			KeUnstackDetachProcess(&kapc);
			return false;
		}
		RtlCopyMemory(shellcode, MemLoadShellcode_x64, sizeof(MemLoadShellcode_x64));
		RtlCopyMemory(buffer, DLLImage, Size);
		HANDLE thread_handle = 0;
		status = RtlCreateUserThread(ZwCurrentProcess(), 0, 0, 0, 0, 0, shellcode, buffer, &thread_handle, 0);
		if (!NT_SUCCESS(status))
		{
			ZwFreeVirtualMemory(ZwCurrentProcess(), &shellcode, &shellcode_size, MEM_RELEASE);
			ZwFreeVirtualMemory(ZwCurrentProcess(), &buffer, &Size, MEM_RELEASE);
			KeUnstackDetachProcess(&kapc);
			return false;
		}
		KeUnstackDetachProcess(&kapc);
		return true;
	}
	return false;
}