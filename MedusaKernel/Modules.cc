#include "Modules.h"




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
	if (!peb)
	{
		KeUnstackDetachProcess(&kapc);
		return temp_vector;
	}
	pPebLdrData = (PPEB_LDR_DATA)peb->Ldr;
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