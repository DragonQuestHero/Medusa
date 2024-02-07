#include "Modules.h"
#include "DLLInjectShellCode.h"



std::vector<UserModule> Modules::GetWin32MoudleList(ULONG64 PID)
{
	std::vector<UserModule> temp_vector;
	PEPROCESS   tempep = NULL;
	KAPC_STATE   kapc = { 0 };
	KIRQL        tempirql = 0;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)PID, &tempep);
	if (!NT_SUCCESS(status))
	{
		return temp_vector;
	}
	ObDereferenceObject(tempep);
	KeStackAttachProcess(tempep, &kapc);
	
	PPEB32 pPeb32 = (PPEB32)PsGetProcessWow64Process(tempep);
	if (pPeb32 && MmIsAddressValid(pPeb32))
	{
		if (!MmIsAddressValid(&pPeb32->Ldr))
		{
			KeUnstackDetachProcess(&kapc);
			return temp_vector;
		}
		PPEB_LDR_DATA32 pPebLdrData = (PPEB_LDR_DATA32)pPeb32->Ldr;
		if (!pPebLdrData)
		{
			KeUnstackDetachProcess(&kapc);
			return temp_vector;
		}
		for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList.Flink;
			pListEntry != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList;
			pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
		{
			PLDR_DATA_TABLE_ENTRY32 pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

			UserModule temp_list;
			RtlZeroMemory(&temp_list, sizeof(UserModule));
			temp_list.Addr = (ULONG64)pEntry->DllBase;
			temp_list.Size = pEntry->SizeOfImage;
			if (pEntry->BaseDllName.Buffer != NULL)
			{
				RtlCopyMemory(temp_list.Name, (PWCH)pEntry->BaseDllName.Buffer, pEntry->BaseDllName.MaximumLength);
			}
			if (pEntry->FullDllName.Buffer != NULL)
			{
				RtlCopyMemory(temp_list.Path, (PWCH)pEntry->FullDllName.Buffer, pEntry->FullDllName.MaximumLength);
			}
			temp_vector.push_back(temp_list);
		}
	}
	{
		PPEB      peb = NULL;
		PPEB_LDR_DATA	pPebLdrData = NULL;
		PLIST_ENTRY		pListEntryStart = NULL;
		PLIST_ENTRY		pListEntryEnd = NULL;
		PLDR_DATA_TABLE_ENTRYB pLdrDataEntry = NULL;

		peb = (PPEB)PsGetProcessPeb(tempep);
		if (!peb || !MmIsAddressValid(peb))
		{
			KeUnstackDetachProcess(&kapc);
			return temp_vector;
		}
		if (!MmIsAddressValid(&peb->Ldr))
		{
			KeUnstackDetachProcess(&kapc);
			return temp_vector;
		}
		pPebLdrData = (PPEB_LDR_DATA)peb->Ldr;
		if (!pPebLdrData)
		{
			KeUnstackDetachProcess(&kapc);
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
	}
	

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
		unsigned char* temp_load_shellcode = nullptr;
		if (PsGetProcessWow64Process(tempep) != NULL)
		{
			temp_load_shellcode = MemLoadShellcode_x86;
		}
		else
		{
			temp_load_shellcode = MemLoadShellcode_x64;
		}
		ULONG64 shellcode_size = sizeof(temp_load_shellcode);
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
		RtlCopyMemory(shellcode, temp_load_shellcode, shellcode_size);
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