#include "KernelModules.h"
#include "cleaner.h"

#include "MemoryRW.h"
#include "MedusaPDBInfo.h"


void KernelModules::GetKernelModuleListALL(PDRIVER_OBJECT  pdriver)
{
	_KernelModuleList.clear();
	GetKernelModuleList1();
	GetKernelModuleList2(pdriver);
	UNICODE_STRING	directory = RTL_CONSTANT_STRING(L"\\driver");
	UNICODE_STRING	FileSystem = RTL_CONSTANT_STRING(L"\\FileSystem");
	GetKernelModuleList3(&directory);
	GetKernelModuleList3(&FileSystem);
	GetKernelModuleList4();
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
	if (!pdriver)
	{
		return temp_vector;
	}
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
				if (DriverObject->DriverSize==0 && 
					(ULONG64)DriverObject->DriverStart==0 &&
					IsAddressInDriversList((ULONG64)DriverObject->DriverInit))
				{
				}
				else
				{
					temp_list.Check = 2;
					_KernelModuleList.push_back(temp_list);
				}
			}
			ObDereferenceObject(DriverObject);
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

std::vector<KernelModulesVector> KernelModules::GetKernelModuleList4Quick()
{
	std::vector<KernelModulesVector> temp_vector;

	PSYSTEM_BIGPOOL_INFORMATION pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePool(NonPagedPool, sizeof(SYSTEM_BIGPOOL_INFORMATION));
	ULONG ReturnLength = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemBigPoolInformation, pBigPoolInfo, sizeof(SYSTEM_BIGPOOL_INFORMATION), &ReturnLength);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pBigPoolInfo);
		ReturnLength = ReturnLength + PAGE_SIZE;
		PSYSTEM_BIGPOOL_INFORMATION pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePool(NonPagedPool, ReturnLength);
		if (!pBigPoolInfo)
		{
			return temp_vector;
		}
		status = ZwQuerySystemInformation(SystemBigPoolInformation, pBigPoolInfo, ReturnLength, &ReturnLength);
		if (NT_SUCCESS(status))
		{
			for (auto j = 0; j < pBigPoolInfo->Count; j++)
			{
				SYSTEM_BIGPOOL_ENTRY poolEntry = pBigPoolInfo->AllocatedInfo[j];
				void* base_addr = poolEntry.VirtualAddress;
				void* memory_p = new char[PAGE_SIZE];
				if (KernelSafeReadMemoryIPI((ULONG64)base_addr, memory_p, PAGE_SIZE))
				{
					for (int i = 0; i < PAGE_SIZE - 2; i++)
					{
						PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)((char*)memory_p + i);
						if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE && MmIsAddressValid(&DosHeader->e_lfanew))
						{
							PIMAGE_NT_HEADERS pNt = PIMAGE_NT_HEADERS((char*)memory_p + i + DosHeader->e_lfanew);
							if (MmIsAddressValid(pNt))
							{
								if (pNt->Signature == IMAGE_NT_SIGNATURE)
								{
									KernelModulesVector temp_list;
									RtlZeroMemory(&temp_list, sizeof(KernelModulesVector));
									temp_list.Addr = (ULONG64)((char*)base_addr + i);
									temp_list.Size = pNt->OptionalHeader.SizeOfImage;
									RtlCopyMemory(temp_list.Name, "!!!!!!!", 20);
									temp_list.Check = 3;
									temp_vector.push_back(temp_list);
									_KernelModuleList.push_back(temp_list);
									break;
								}
							}
						}
					}
				}
				delete memory_p;
			}
		}
		ExFreePool(pBigPoolInfo);
	}
	return temp_vector;
}

std::vector<KernelModulesVector> KernelModules::GetKernelModuleList4()
{
	std::vector<KernelModulesVector> temp_vector;

	PSYSTEM_BIGPOOL_INFORMATION pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePool(NonPagedPool, sizeof(SYSTEM_BIGPOOL_INFORMATION));
	ULONG ReturnLength = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemBigPoolInformation, pBigPoolInfo, sizeof(SYSTEM_BIGPOOL_INFORMATION), &ReturnLength);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pBigPoolInfo);
		ReturnLength = ReturnLength + PAGE_SIZE;
		PSYSTEM_BIGPOOL_INFORMATION pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePool(NonPagedPool, ReturnLength);
		if (!pBigPoolInfo)
		{
			return temp_vector;
		}
		status = ZwQuerySystemInformation(SystemBigPoolInformation, pBigPoolInfo, ReturnLength, &ReturnLength);
		if (NT_SUCCESS(status))
		{
			for (auto j = 0; j < pBigPoolInfo->Count; j++)
			{
				SYSTEM_BIGPOOL_ENTRY poolEntry = pBigPoolInfo->AllocatedInfo[j];
				void* base_addr = poolEntry.VirtualAddress;
				void* memory_p = new char[poolEntry.SizeInBytes];
				SIZE_T NumberOfBytesTransferred;
				MM_COPY_ADDRESS SourceAddress;
				SourceAddress.VirtualAddress = (PVOID)base_addr;
				status = MmCopyMemory(memory_p, SourceAddress, poolEntry.SizeInBytes, MM_COPY_MEMORY_VIRTUAL, &NumberOfBytesTransferred);
				if (!NT_SUCCESS(status))
				{
					if (memory_p)
					{
						delete memory_p;
						memory_p = nullptr;
					}
					continue;
				}
				for (int i = 0; i < NumberOfBytesTransferred; i++)
				{
					PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)((char*)memory_p + i);
					if (MmIsAddressValid(&DosHeader->e_lfanew) &&
						DosHeader->e_magic == IMAGE_DOS_SIGNATURE && 
						NumberOfBytesTransferred - i > PAGE_SIZE)
					{
						PIMAGE_NT_HEADERS pNt = PIMAGE_NT_HEADERS((char*)memory_p + i + DosHeader->e_lfanew);
						if (MmIsAddressValid(pNt))
						{
							if (pNt->Signature == IMAGE_NT_SIGNATURE)
							{
								KernelModulesVector temp_list;
								RtlZeroMemory(&temp_list, sizeof(KernelModulesVector));
								temp_list.Addr = (ULONG64)((char*)base_addr + i);
								temp_list.Size = pNt->OptionalHeader.SizeOfImage;
								RtlCopyMemory(temp_list.Name, "!!!!!!!", 20);
								temp_list.Check = 3;
								temp_vector.push_back(temp_list);
								_KernelModuleList.push_back(temp_list);
								break;
							}
						}
					}
				}

				delete memory_p;
			}
		}
		ExFreePool(pBigPoolInfo);
	}
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

bool KernelModules::GetUnLoadKernelModuleList(PDRIVER_OBJECT  pdriver)
{
	_UnLoadKernelModuleList.clear();
	std::vector<KernelModulesVector> temp_vector = GetKernelModuleList2(pdriver);

	if (MedusaPDBInfo::_PDBInfo.CiEaCacheLookasideList)
	{

	}
	else
	{
		bool status = false;
		do 
		{
			unsigned long long ntoskrnl_address = 0;
			unsigned long ntoskrnl_size = 0;
			ntoskrnl_address = (ULONG64)cleaner::get_kernel_base(&ntoskrnl_size);
			if (ntoskrnl_address == 0 || ntoskrnl_size == 0) break;
			unsigned long long MmUnloadedDrivers = cleaner::find_pattern_image(ntoskrnl_address,
				"\x4C\x8B\x15\x00\x00\x00\x00\x4C\x8B\xC9",
				"xxx????xxx");
			if (MmUnloadedDrivers == 0) break;
			MmUnloadedDrivers = reinterpret_cast<unsigned long long>(reinterpret_cast<char*>(MmUnloadedDrivers) + 7 + *reinterpret_cast<int*>(reinterpret_cast<char*>(MmUnloadedDrivers) + 3));

			unsigned long long MmLastUnloadedDriver = cleaner::find_pattern_image(ntoskrnl_address,
				"\x8B\x05\x00\x00\x00\x00\x83\xF8\x32",
				"xx????xxx");
			if (MmLastUnloadedDriver == 0) break;
			MmLastUnloadedDriver =
				reinterpret_cast<unsigned long long>(reinterpret_cast<char*>(MmLastUnloadedDriver)
					+ 6 + *reinterpret_cast<int*>(reinterpret_cast<char*>(MmLastUnloadedDriver) + 2));

			cleaner::punloader_information unloaders = *(cleaner::punloader_information*)MmUnloadedDrivers;
			unsigned long* unloaders_count = (unsigned long*)MmLastUnloadedDriver;
			if (MmIsAddressValid(unloaders) == FALSE || MmIsAddressValid(unloaders_count) == FALSE) break;

			static ERESOURCE PsLoadedModuleResource;
			if (ExAcquireResourceExclusiveLite(&PsLoadedModuleResource, TRUE))
			{
				for (unsigned long i = 0; i < *unloaders_count && i < cleaner::max_unloader_driver; i++)
				{
					cleaner::unloader_information& t = unloaders[i];

					KernelUnloadModules temp_list;
					RtlZeroMemory(&temp_list, sizeof(KernelUnloadModules));
					temp_list.Addr = (ULONG64)t.module_start;
					temp_list.Size = (ULONG64)t.module_end - (ULONG64)t.module_start;
					temp_list.UnLoadTime = t.unload_time;
					RtlCopyMemory(temp_list.Name, t.name.Buffer, t.name.MaximumLength);
					_UnLoadKernelModuleList.push_back(temp_list);
				}
				ExReleaseResourceLite(&PsLoadedModuleResource);
			}
			status = true;
		} while (false);


		{
			bool status = false;

			unsigned long long ci_address = 0;
			unsigned long ci_size = 0;
			cleaner::get_module_base_address("CI.dll", ci_address, ci_size);
			if (ci_address == 0 || ci_size == 0) return status;
			unsigned long long HashCacheLock = 0;
			unsigned long long KernelHashBucketList = cleaner::find_pattern_image(ci_address,
				"\x48\x8B\x1D\x00\x00\x00\x00\xEB\x00\xF7\x43\x40\x00\x20\x00\x00",
				"xxx????x?xxxxxxx");
			if (KernelHashBucketList == 0) return status;
			else HashCacheLock = KernelHashBucketList - 0x13;

			KernelHashBucketList = reinterpret_cast<unsigned long long>(reinterpret_cast<char*>(KernelHashBucketList) 
				+ 7 + *reinterpret_cast<int*>(reinterpret_cast<char*>(KernelHashBucketList) + 3));
			HashCacheLock = reinterpret_cast<unsigned long long>(reinterpret_cast<char*>(HashCacheLock) 
				+ 7 + *reinterpret_cast<int*>(reinterpret_cast<char*>(HashCacheLock) + 3));
			if (ExAcquireResourceExclusiveLite((PERESOURCE)HashCacheLock, TRUE))
			{
				cleaner::phash_bucket_entry current_entry = ((cleaner::phash_bucket_entry)KernelHashBucketList)->next;
				cleaner::phash_bucket_entry prev_entry = (cleaner::phash_bucket_entry)KernelHashBucketList;

				while (current_entry)
				{
					KernelUnloadModules temp_list;
					RtlZeroMemory(&temp_list, sizeof(KernelUnloadModules));
					RtlCopyMemory(temp_list.Name, current_entry->name.Buffer, current_entry->name.MaximumLength);
					bool found = false;
					for (auto x : temp_vector)
					{
						std::wstring temp1 = Case_Upper(temp_list.Name);
						std::wstring temp2 = Case_Upper(x.Name);
						if (temp1.find(temp2) != std::string::npos)
						{
							found = true;
							break;
						}
					}
					for (auto x : _UnLoadKernelModuleList)
					{
						std::wstring temp1 = Case_Upper(temp_list.Name);
						std::wstring temp2 = Case_Upper(x.Name);
						if (temp1.find(temp2) != std::string::npos)
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						_UnLoadKernelModuleList.push_back(temp_list);
					}
					prev_entry = current_entry;
					current_entry = current_entry->next;
				}

				ExReleaseResourceLite((PERESOURCE)HashCacheLock);
			}
		}




		
		{
			unsigned long long ntoskrnl_address = 0;
			unsigned long ntoskrnl_size = 0;
			ntoskrnl_address = (ULONG64)cleaner::get_kernel_base(&ntoskrnl_size);
			if (ntoskrnl_address == 0 || ntoskrnl_size == 0) return status;
			unsigned long long PiDDBLock = cleaner::find_pattern_image(ntoskrnl_address,
				"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x4C\x8B\x8C",
				"xxx????x????xxx");
			if (PiDDBLock == 0) return status;
			PiDDBLock = reinterpret_cast<unsigned long long>(reinterpret_cast<char*>(PiDDBLock) 
				+ 7 + *reinterpret_cast<int*>(reinterpret_cast<char*>(PiDDBLock) + 3));
			unsigned long long PiDDBCacheTable = cleaner::find_pattern_image(ntoskrnl_address,
				"\x66\x03\xD2\x48\x8D\x0D",
				"xxxxxx");
			if (PiDDBCacheTable == 0) return status;
			PiDDBCacheTable += 3;
			PiDDBCacheTable = reinterpret_cast<unsigned long long>(reinterpret_cast<char*>(PiDDBCacheTable) 
				+ 7 + *reinterpret_cast<int*>(reinterpret_cast<char*>(PiDDBCacheTable) + 3));
			
			


			if (ExAcquireResourceExclusiveLite((PERESOURCE)PiDDBLock, TRUE))
			{
				PRTL_AVL_TABLE PiDDBCacheTable2 = (PRTL_AVL_TABLE)PiDDBCacheTable;
				PVOID FirstNode = PiDDBCacheTable2->BalancedRoot.RightChild;

				cleaner::piddb_cache_entry* FirstEntry = (cleaner::piddb_cache_entry*)
					((DWORD64)FirstNode + sizeof(RTL_BALANCED_LINKS));
				PLIST_ENTRY Head = FirstEntry->list.Flink;
				PLIST_ENTRY TempList = (PLIST_ENTRY)FirstEntry;

				while (true)
				{
					TempList = TempList->Flink;
					if (TempList->Flink == Head) { break; }
					cleaner::piddb_cache_entry* Entry = (cleaner::piddb_cache_entry*)TempList;
					



					KernelUnloadModules temp_list;
					RtlZeroMemory(&temp_list, sizeof(KernelUnloadModules));
					//temp_list.UnLoadTime = Entry->stamp;
					RtlCopyMemory(temp_list.Name, Entry->name.Buffer, Entry->name.MaximumLength);
					bool found = false;
					for (auto x : temp_vector)
					{
						std::wstring temp1 = Case_Upper(temp_list.Name);
						std::wstring temp2 = Case_Upper(x.Name);
						if (temp1.find(temp2) != std::string::npos)
						{
							found = true;
							break;
						}
					}
					for (auto x : _UnLoadKernelModuleList)
					{
						std::wstring temp1 = Case_Upper(temp_list.Name);
						std::wstring temp2 = Case_Upper(x.Name);
						if (temp1.find(temp2) != std::string::npos)
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						_UnLoadKernelModuleList.push_back(temp_list);
					}
				}
				ExReleaseResourceLite((PERESOURCE)PiDDBLock);
			}
		}


		for (auto x : _UnLoadKernelModuleList)
		{
			if (MmIsAddressValid((void*)x.Addr))
			{
				x.Check = 1;
			}
		}



		return status;
	}
}