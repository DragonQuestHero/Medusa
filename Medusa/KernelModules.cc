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

#define TEST_GetALLKernelModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7102,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLKernelModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7103,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool KernelModules::GetKernelModuleListR0()
{
	_KernelModuleListR0.clear();

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetALLKernelModuleNumber, 0, 0, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		KernelModulesVector* temp_list = (KernelModulesVector*)new char[process_number * sizeof(KernelModulesVector)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetALLKernelModule, 0, 0, temp_list, sizeof(KernelModulesVector) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				_KernelModuleListR0.push_back(temp_list[i]);
			}
		}
		delete temp_list;

		CloseHandle(m_hDevice);
		return true;
	} while (false);





	CloseHandle(m_hDevice);
	return false;
}

#define TEST_GetUnLoadKernelModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7111,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetUnLoadKernelModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7112,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool KernelModules::GetUnLoadKernelModuleListR0()
{
	_KernelUnLoadModuleListR0.clear();

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetUnLoadKernelModuleNumber, 0, 0, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		KernelUnloadModules* temp_list = (KernelUnloadModules*)new char[process_number * sizeof(KernelUnloadModules)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetUnLoadKernelModule, 0, 0, temp_list, sizeof(KernelUnloadModules) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				_KernelUnLoadModuleListR0.push_back(temp_list[i]);
			}
		}
		delete temp_list;

		CloseHandle(m_hDevice);
		return true;
	} while (false);





	CloseHandle(m_hDevice);
	return false;
}