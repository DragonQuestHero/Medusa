#include "KernelModules.h"

void KernelModules::GetKernelModuleListALL(PDRIVER_OBJECT  pdriver)
{
	GetKernelModuleList1();
	GetKernelModuleList2(pdriver);
}

bool KernelModules::GetKernelModuleList1()
{
	NTSTATUS    status = STATUS_SUCCESS;
	RTL_PROCESS_MODULES info = { 0 };
	ULONG retPro = 0;

	status = ZwQuerySystemInformation(SystemModuleInformation, &info, sizeof(info), &retPro);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		return false;
	}

	ULONG len = retPro + sizeof(RTL_PROCESS_MODULES);
	PRTL_PROCESS_MODULES mem = (PRTL_PROCESS_MODULES)ExAllocatePool(PagedPool, len);
	if (!mem)
	{
		return false;
	}
	RtlZeroMemory(mem, len);
	status = ZwQuerySystemInformation(SystemModuleInformation, mem, len, &retPro);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(mem);
		return false;
	}

	for (DWORD i = 0; i < mem->NumberOfModules; i++)
	{
		PRTL_PROCESS_MODULE_INFORMATION processModule = &mem->Modules[i];
		KernelModulesVector temp_list;
		RtlZeroMemory(&temp_list, sizeof(KernelModulesVector));
		temp_list.Check = false;
		temp_list.Addr = (ULONG64)processModule->ImageBase;
		temp_list.Size = processModule->ImageSize;
		RtlCopyMemory(temp_list.Path, processModule->FullPathName, 256);
		RtlCopyMemory(temp_list.Name, processModule->FullPathName + processModule->OffsetToFileName, 256 - processModule->OffsetToFileName);
		_KernelModuleList.push_back(temp_list);
	}
	if (mem)
	{
		ExFreePool(mem);
		mem = NULL;
	}
	return true;
}



std::vector<KernelModulesVector> KernelModules::GetKernelModuleList2(PDRIVER_OBJECT  pdriver)
{
	if (_KernelModuleList.size() == 0)
	{
		return;
	}

	std::vector<KernelModulesVector> temp_vector;
	PLDR_DATA_TABLE_ENTRY		pentry = (PLDR_DATA_TABLE_ENTRY)pdriver->DriverSection;
	PLDR_DATA_TABLE_ENTRY		first = pentry;
	do
	{
		if (pentry->DllBase)
		{
			KernelModulesVector temp_list;
			RtlZeroMemory(&temp_list, sizeof(KernelModulesVector));
			temp_list.Addr = (ULONG64)pentry->DllBase;
			temp_list.Size = pentry->SizeOfImage;
			if (pentry->BaseDllName.Buffer != NULL)
			{
				RtlCopyMemory(temp_list.Name, pentry->BaseDllName.Buffer, pentry->BaseDllName.MaximumLength);
			}
			temp_vector.push_back(temp_list);

			bool found = false;
			for (auto y : _KernelModuleList)
			{
				if (temp_list.Addr == y.Addr)
				{
					break;
				}
			}
			if (!found)
			{
				temp_list.Check = true;
				_KernelModuleList.push_back(temp_list);
			}
			pentry = (PLDR_DATA_TABLE_ENTRY)pentry->InLoadOrderLinks.Blink;
		}
	} while ((ULONGLONG)pentry->InLoadOrderLinks.Blink != (ULONGLONG)first);

	return temp_vector;
}