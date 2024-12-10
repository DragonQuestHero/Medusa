#include "KernelModules.h"

#include "ntdll.h"

#include "PDBInfo.h"



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
		KernelModulesVector temp_list = { 0 };
		temp_list.Check = false;
		temp_list.Addr = (ULONG64)processModule->ImageBase;
		temp_list.Size = processModule->ImageSize;
		temp_list.Path = (char*)processModule->FullPathName;
		char* temp_str = (char*)(processModule->FullPathName + processModule->OffsetToFileName);
		temp_list.Name = temp_str;
		_KernelModuleListR3.push_back(temp_list);
	}
	if (mem)
	{
		delete mem;
		mem = NULL;
	}
	_KernelModuleList = _KernelModuleListR3;
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
		KernelModulesVector2* temp_list = (KernelModulesVector2*)new char[process_number * sizeof(KernelModulesVector2)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetALLKernelModule, 0, 0, temp_list, sizeof(KernelModulesVector2) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				KernelModulesVector temp_list2;
				if (temp_list[i].Check)
				{
					temp_list2.Addr = temp_list[i].Addr;
					temp_list2.Check = temp_list[i].Check;
					temp_list2.DriverObject = temp_list[i].DriverObject;
					temp_list2.Size = temp_list[i].Size;
					temp_list2.Name = W_TO_C(temp_list[i].Name);
					temp_list2.Path = temp_list[i].Path;
				}
				else
				{
					temp_list2.Addr = temp_list[i].Addr;
					temp_list2.Check = temp_list[i].Check;
					temp_list2.DriverObject = temp_list[i].DriverObject;
					temp_list2.Size = temp_list[i].Size;
					temp_list2.Name = (char*)(temp_list[i].Name);
					temp_list2.Path = temp_list[i].Path;
				}
				_KernelModuleListR0.push_back(temp_list2);
			}
		}
		delete temp_list;

		CloseHandle(m_hDevice);
		return true;
	} while (false);


	_KernelModuleList = _KernelModuleListR0;


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

#define TEST_DumpDriver CTL_CODE(FILE_DEVICE_UNKNOWN,0x7115,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool KernelModules::DumpDriver(ULONG64 Address, ULONG64 Size,void*buffer)
{
	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	do
	{
		DWORD dwRet = 0;
		char* temp_list = new char[Size + sizeof(ULONG64)];
		if (!temp_list)
		{
			break;
		}
		RtlCopyMemory(temp_list, &Address, sizeof(ULONG64));

		DeviceIoControl(m_hDevice, TEST_DumpDriver, temp_list, Size + sizeof(ULONG64), temp_list, Size + sizeof(ULONG64), &dwRet, NULL);
		if (dwRet)
		{
			RtlCopyMemory(buffer, temp_list + 8, Size);
			delete temp_list;
			CloseHandle(m_hDevice);
			return true;
		}
	} while (false);
	CloseHandle(m_hDevice);
	return false;
}




#define TEST_GetSSDTList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7123,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetSSDTListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7124,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetSSSDTList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7125,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetSSSDTListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7126,METHOD_BUFFERED ,FILE_ANY_ACCESS)

std::vector<SSDT_STRUCT2> KernelModules::GetALLSSDT(bool Setting_SSDT_SSSDT_PDB)
{
	if (!_KernelModuleList.size())
	{
		GetKernelModuleListR3();
	}
	_SSDTALL.clear();

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return _SSDTALL;
	}
	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetSSDTListNumber, 0, 0, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		SSDT_STRUCT* temp_list = (SSDT_STRUCT*)new char[process_number * sizeof(SSDT_STRUCT)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetSSDTList, 0, 0, temp_list, sizeof(SSDT_STRUCT) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			ModuleExportFunc _ModuleExportFunc;

			int index = GetDriversListIndexFromAddress(temp_list[0].Addr);

			bool use_pdb = false;
			PDBInfo _PDBInfo;
			if (index != -1 && (_PDBInfo.QueryDownLoad(ConvertSystemRootPath(_KernelModuleList[index].Path)) || Setting_SSDT_SSSDT_PDB))
			{
				_PDBInfo.DownLoad(ConvertSystemRootPath(_KernelModuleList[index].Path), _KernelModuleList[index].Addr);
				_PDBInfo.GetALL();
				use_pdb = true;
			}
			std::vector<ExportFunc> ntos_func;
			if (index != -1 && use_pdb == false)
			{
				ntos_func = _ModuleExportFunc.GetExportFunc(
					_KernelModuleList[index].Addr, ConvertSystemRootPath(_KernelModuleList[index].Path));
			}
			for (int i = 0; i < process_number; i++)
			{
				SSDT_STRUCT2 temp_SSDT_STRUCT;
				temp_SSDT_STRUCT.Addr = temp_list[i].Addr;
				temp_SSDT_STRUCT.Index = temp_list[i].Index;
				temp_SSDT_STRUCT.FuncName = "";
				temp_SSDT_STRUCT.Modules = "";
				if (index != -1)
				{
					if (use_pdb)
					{
						for (auto x : _PDBInfo._Symbol)
						{
							if (temp_list[i].Addr == x.Addr)
							{
								temp_SSDT_STRUCT.FuncName = x.Name;
								break;
							}
						}
					}
					else
					{
						for (auto x : ntos_func)
						{
							if (x.Addr == temp_SSDT_STRUCT.Addr)
							{
								temp_SSDT_STRUCT.FuncName = x.Name;
								break;
							}
						}
					}
					temp_SSDT_STRUCT.Modules = _KernelModuleList[index].Name;
				}
				_SSDTALL.push_back(temp_SSDT_STRUCT);
			}
		}
		delete temp_list;
	} while (false);
	CloseHandle(m_hDevice);
	return _SSDTALL;
}


std::vector<SSDT_STRUCT2> KernelModules::GetALLShadowSSDT(bool Setting_SSDT_SSSDT_PDB)
{
	if (!_KernelModuleList.size())
	{
		GetKernelModuleListR3();
	}
	_SSDTALL.clear();

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return _SSDTALL;
	}
	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetSSSDTListNumber, 0, 0, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		SSDT_STRUCT* temp_list = (SSDT_STRUCT*)new char[process_number * sizeof(SSDT_STRUCT)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetSSSDTList, 0, 0, temp_list, sizeof(SSDT_STRUCT) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			ModuleExportFunc _ModuleExportFunc;
			//没符号的情况下不支持查看了
			/*std::vector<ExportFunc> win32k_func = _ModuleExportFunc.GetExportFunc(
				_KernelModuleList[GetDriversListIndexFromName("win32k.sys")].Addr, 
				ConvertSystemRootPath(_KernelModuleList[GetDriversListIndexFromName("win32k.sys")].Path));*/

			int index = GetDriversListIndexFromAddress(temp_list[0].Addr);
			bool use_pdb = false;
			PDBInfo _PDBInfo;
			if (index != -1 && (_PDBInfo.QueryDownLoad(ConvertSystemRootPath(_KernelModuleList[index].Path)) || Setting_SSDT_SSSDT_PDB))
			{
				_PDBInfo.DownLoad(ConvertSystemRootPath(_KernelModuleList[index].Path), _KernelModuleList[index].Addr);
				_PDBInfo.GetALL();
				use_pdb = true;
			}


			for (int i = 0; i < process_number; i++)
			{
				SSDT_STRUCT2 temp_SSDT_STRUCT;
				temp_SSDT_STRUCT.Addr = temp_list[i].Addr;
				temp_SSDT_STRUCT.Index = temp_list[i].Index;
				temp_SSDT_STRUCT.FuncName = "";
				temp_SSDT_STRUCT.Modules = "";
				if (GetDriversListIndexFromAddress(temp_list[0].Addr))
				{
					/*for (auto x : win32k_func)
					{
						if (x.Addr == temp_SSDT_STRUCT.Addr)
						{
							temp_SSDT_STRUCT.FuncName = x.Name;
							break;
						}
					}*/
					temp_SSDT_STRUCT.Modules = _KernelModuleList[GetDriversListIndexFromAddress(temp_list[0].Addr)].Name;
				}
				if (use_pdb)
				{
					for (auto x : _PDBInfo._Symbol)
					{
						if (temp_list[i].Addr == x.Addr)
						{
							temp_SSDT_STRUCT.FuncName = x.Name;
							break;
						}
					}
				}
				_SSDTALL.push_back(temp_SSDT_STRUCT);
			}
		}
		delete temp_list;
	} while (false);
	CloseHandle(m_hDevice);
	return _SSDTALL;
}

#define TEST_DriverUnload CTL_CODE(FILE_DEVICE_UNKNOWN,0x7130,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool KernelModules::DriverUnload(ULONG64 Address)
{
	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	DWORD dwRet = 0;
	if (DeviceIoControl(m_hDevice, TEST_DriverUnload, &Address, 8, 0, 0, &dwRet, NULL))
	{
		CloseHandle(m_hDevice);
		return true;
	}
	CloseHandle(m_hDevice);
	return false;
}