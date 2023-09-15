#include "KernelModules.h"

void KernelModules::GetKernelModuleListALL(PDRIVER_OBJECT  pdriver)
{
	_KernelModuleList.clear();
	GetKernelModuleList1();
	GetKernelModuleList2(pdriver);
	UNICODE_STRING	directory = RTL_CONSTANT_STRING(L"\\driver");
	UNICODE_STRING	FileSystem = RTL_CONSTANT_STRING(L"\\FileSystem");
	GetKernelModuleList3(&directory);
	GetKernelModuleList3(&FileSystem);
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
		temp_list.Check = 0;
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
			if (pentry->FullDllName.Buffer != NULL)
			{
				RtlCopyMemory(temp_list.Path, pentry->FullDllName.Buffer, pentry->FullDllName.MaximumLength);
			}
			temp_vector.push_back(temp_list);

			bool found = false;
			for (auto y : _KernelModuleList)
			{
				if (temp_list.Addr == y.Addr)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				temp_list.Check = 1;
				_KernelModuleList.push_back(temp_list);
			}
			pentry = (PLDR_DATA_TABLE_ENTRY)pentry->InLoadOrderLinks.Blink;
		}
	} while ((ULONGLONG)pentry->InLoadOrderLinks.Blink != (ULONGLONG)first);

	return temp_vector;
}



std::vector<KernelModulesVector> KernelModules::GetKernelModuleList3(UNICODE_STRING* Directory)
{
	std::vector<KernelModulesVector> temp_vector;

	NTSTATUS			ntStatus = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES	ObjAttr = { 0 };
	HANDLE				FileHandle = 0;
	IO_STATUS_BLOCK		IoStatusBlock = { 0 };
	PVOID				FileInformation = 0;
	ULONG				Length = 4096; // 这个数设置的太小会导致ZwQueryDirectoryFile蓝屏。  
	UNICODE_STRING		driverName = RTL_CONSTANT_STRING(L"Driver");
	BOOLEAN         RestartScan;
	ULONG           Context = 0;
	ULONG           ReturnedLength = 0;

	InitializeObjectAttributes(&ObjAttr, Directory, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
	ntStatus = ZwOpenDirectoryObject(&FileHandle, GENERIC_READ | SYNCHRONIZE, &ObjAttr);
	if (!NT_SUCCESS(ntStatus))
	{
		return temp_vector;
	}
	FileInformation = ExAllocatePool(NonPagedPool, Length);
	if (NULL == FileInformation)
	{
		ZwClose(FileHandle);
		return temp_vector;
	}
	RtlZeroMemory(FileInformation, Length);

	do
	{
		UNICODE_STRING FileName = { 0 };
		POBJECT_DIRECTORY_INFORMATION podi = 0;
		UNICODE_STRING FullName = { 0 };

		RestartScan = FALSE; // 为TRUE会导致死循环;
		ntStatus = ZwQueryDirectoryObject(FileHandle, FileInformation, Length, TRUE, RestartScan, &Context, &ReturnedLength);
		if (STATUS_NO_MORE_FILES != ntStatus && STATUS_SUCCESS != ntStatus)
		{
			break;
		}

		podi = (POBJECT_DIRECTORY_INFORMATION)FileInformation;
		if (RtlCompareUnicodeString(&podi->TypeName, &driverName, TRUE) != 0)
		{
			continue;
		}

		// 申请要显示的内存，另一思路是格式化。
		FullName.MaximumLength = (USHORT)Length + Directory->MaximumLength;
		FullName.Buffer = (PWCH)ExAllocatePool(NonPagedPool, FullName.MaximumLength);
		if (NULL == FullName.Buffer) {
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlZeroMemory(FullName.Buffer, FullName.MaximumLength);
		RtlCopyUnicodeString(&FullName, Directory);
		ntStatus = RtlAppendUnicodeToString(&FullName, L"\\");
		if (!NT_SUCCESS(ntStatus)) {
			ExFreePool(FullName.Buffer);
			break;
		}
		ntStatus = RtlAppendUnicodeStringToString(&FullName, &podi->Name);
		if (!NT_SUCCESS(ntStatus)) {
			ExFreePool(FullName.Buffer);
			break;
		}
		PDRIVER_OBJECT DriverObject = nullptr;
		ntStatus = ObReferenceObjectByName(
			&FullName,
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
			NULL, FILE_ANY_ACCESS,
			*IoDriverObjectType,
			KernelMode,
			NULL,
			(PVOID*)&DriverObject);
		if (NT_SUCCESS(ntStatus))
		{
			KernelModulesVector temp_list;
			RtlZeroMemory(&temp_list, sizeof(KernelModulesVector));
			temp_list.Addr = (ULONG64)DriverObject->DriverStart;
			temp_list.Size = DriverObject->DriverSize;
			if (DriverObject->DriverName.Buffer != NULL)
			{
				RtlCopyMemory(temp_list.Name, DriverObject->DriverName.Buffer, DriverObject->DriverName.MaximumLength);
			}
			temp_vector.push_back(temp_list);
			
			bool found = false;
			for (auto y : _KernelModuleList)
			{
				if (temp_list.Addr == y.Addr)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				temp_list.Check = 2;
				_KernelModuleList.push_back(temp_list);
			}
		}
		ExFreePool(FullName.Buffer);
	} while (STATUS_NO_MORE_FILES != ntStatus);
	if (FileInformation)
	{
		ExFreePool(FileInformation);
		FileInformation = NULL;
	}
	ZwClose(FileHandle);
	return temp_vector;
}


bool KernelModules::IsAddressInDriversList(ULONG64 Address)
{
	for (auto x : _KernelModuleList)
	{
		if (Address >= x.Addr && Address < x.Addr + x.Size)
			return true;
	}
	return false;
}