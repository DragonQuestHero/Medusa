#include "KernelModules.h"

#include "ntdll.h"



bool KernelModules::GetKernelModuleListR3()
{
	_KernelModuleListR3.clear();
	NTSTATUS    status = STATUS_SUCCESS;
	RTL_PROCESS_MODULES info = { 0 };
	ULONG retPro = 0;

	status = ZwQuerySystemInformation(SystemModuleInformation, &info, sizeof(info), &retPro);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		return false;
	}

	ULONG len = retPro + sizeof(RTL_PROCESS_MODULES);
	PRTL_PROCESS_MODULES mem = (PRTL_PROCESS_MODULES)new char[len];
	if (!mem)
	{
		return false;
	}
	RtlZeroMemory(mem, len);
	status = ZwQuerySystemInformation(SystemModuleInformation, mem, len, &retPro);
	if (!NT_SUCCESS(status))
	{
		delete mem;
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
		_KernelModuleListR3.push_back(temp_list);
	}
	if (mem)
	{
		delete mem;
		mem = NULL;
	}
	return true;
}