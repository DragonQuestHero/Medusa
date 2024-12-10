#include "Driver_Load.h"

bool Driver_Load::Init(std::string Driver_Path)
{
	if (Driver_Path == "")
	{
		return false;
	}
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(Driver_Path.data(), drive, dir, fname, ext);
	_Driver_Name = fname;
	_Driver_Path = Driver_Path;
	_Driver_Ext = ext;
	if (std::string(ext)==".sys")
	{
		return true;
	}
	/*if (Sys_File > 0)
	{
		return true;
	}
	Sys_File++;*/
	return false;
}

bool Driver_Load::Get_Driver_Handle()
{
	if (!_Server_Handle)
	{
		_Server_Handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (!_Server_Handle)
		{
			_Last_Error = GetLastError();
			return false;
		}

	}
	if (!_Drive_Handle)
	{
		_Drive_Handle = OpenServiceA(_Server_Handle,
			_Driver_Name.data(), SERVICE_ALL_ACCESS);
		if (!_Drive_Handle)
		{
			_Last_Error = GetLastError();
			return false;
		}
	}
}

bool Driver_Load::Register_Driver()
{
	_NtModule = false;
	_Server_Handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!_Server_Handle)
	{
		_Last_Error = GetLastError();
		return false;
	}
	_Drive_Handle = CreateServiceA(_Server_Handle,
		_Driver_Name.data(), _Driver_Name.data(),
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
		_Driver_Path.data(), NULL, NULL, NULL, NULL, NULL);
	if (!_Drive_Handle)
	{
		_Last_Error = GetLastError();
		if (_Last_Error == 0x431)
		{
			return true;
		}
		return false;
	}
	return true;
}

bool Driver_Load::Start_Driver()
{
	Get_Driver_Handle();
	if (!StartServiceA(_Drive_Handle, NULL, NULL))
	{
		_Last_Error = GetLastError();
		if(_Last_Error == ERROR_SERVICE_ALREADY_RUNNING)
		{
			return true;
		}
		return false;
	}
	return true;
}

bool Driver_Load::Stop_Driver()
{
	Get_Driver_Handle();
	SERVICE_STATUS status;
	status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	if (!ControlService(_Drive_Handle,
		SERVICE_CONTROL_STOP, &status))
	{
		_Last_Error = GetLastError();
		return false;
	}
	if (status.dwCurrentState == SERVICE_STOP_PENDING || status.dwCurrentState == SERVICE_STOPPED)
	{
		return true;
	}
	_Last_Error = status.dwServiceSpecificExitCode;
	return false;
}

bool Driver_Load::UnRegister_Driver()
{
	Get_Driver_Handle();
	if (!DeleteService(_Drive_Handle))
	{
		_Last_Error = GetLastError();
		return false;
	}
	return true;
}

bool Driver_Load::Minifilter_Register_Driver()
{
	_Server_Handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!_Server_Handle)
	{
		_Last_Error = GetLastError();
		return false;
	}
	_Drive_Handle = CreateServiceA(_Server_Handle,
		_Driver_Name.data(), _Driver_Name.data(),
		SERVICE_ALL_ACCESS, SERVICE_FILE_SYSTEM_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
		_Driver_Path.data(), NULL, NULL, NULL, NULL, NULL);
	if (!_Drive_Handle)
	{
		_Last_Error = GetLastError();
		return false;
	}

	std::string temp_str;
	HKEY phkResult;
	temp_str = "SYSTEM\\ControlSet001\\Services\\" + _Driver_Name;
	if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, temp_str.data(), 0, KEY_ALL_ACCESS, &phkResult))
	{
		RegSetValueExA(phkResult, "DependOnService", 0, REG_MULTI_SZ, (BYTE*)"FltMgr", 7);
		temp_str = _Driver_Name + " Driver";
		RegSetValueExA(phkResult, "Description", 0, REG_SZ, (BYTE*)temp_str.data(), temp_str.length());
		RegSetValueExA(phkResult, "Group", 0, REG_SZ, (BYTE*)"FSFilter Activity Monitor", 26);
		DWORD temp_value = 3;
		RegSetValueExA(phkResult, "SupportedFeatures", 0, REG_DWORD, (LPBYTE)&temp_value, sizeof(DWORD));
		temp_value = 2;
		RegSetValueExA(phkResult, "Tag", 0, REG_DWORD, (BYTE*)&temp_value, 4);
		temp_value = 2;
		RegSetValueExA(phkResult, "Type", 0, REG_DWORD, (BYTE*)&temp_value, 4);

		RegCreateKeyA(phkResult, "Instances", &phkResult);
		temp_str = _Driver_Name + " Instance";
		RegSetValueExA(phkResult, "DefaultInstance", 0, REG_SZ, (BYTE*)temp_str.data(), temp_str.length());

		temp_str = _Driver_Name + " Instance";
		RegCreateKeyA(phkResult, temp_str.data(), &phkResult);
		RegSetValueExA(phkResult, "Altitude", 0, REG_SZ, (BYTE*)"325107", 7);
		temp_value = 4;
		RegSetValueExA(phkResult, "Flags", 0, REG_DWORD, (BYTE*)&temp_value, 4);
		RegCloseKey(phkResult);
	}
	else
	{
		_Last_Error = GetLastError();
		return false;
	}
	return true;
}

bool Driver_Load::Enable_Debug()
{
	HANDLE hToken;
	//把一个访问令牌中没有启用该权限但是本身是具有该权限的进程提权
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //打开进程访问令牌  
	{
		//试图修改“调试”特权  
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		BOOL fOK = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		_Last_Error = GetLastError();
		if (_Last_Error != ERROR_SUCCESS || fOK != TRUE)
		{
			QMessageBox::information(nullptr, "Error", std::to_string(_Last_Error).data());
			CloseHandle(hToken);
			return false;
		}
		CloseHandle(hToken);
		return true;
	}
	else
	{
		goto _ERROR;
	}
_ERROR:
	_Last_Error = GetLastError();
	return false;
}

bool Driver_Load::Nt_Register_Driver()
{
	_NtModule = true;
	std::string temp_str = "SYSTEM\\CurrentControlSet\\Services\\" + _Driver_Name;
	HKEY phkResult;
	if (ERROR_SUCCESS == RegCreateKeyA(HKEY_LOCAL_MACHINE, temp_str.data(), &phkResult))
	{
		DWORD temp_value = 1;
		RegSetValueExA(phkResult, "ErrorControl", 0, REG_DWORD, (LPBYTE)&temp_value, sizeof(DWORD));
		RegSetValueExA(phkResult, "Type", 0, REG_DWORD, (LPBYTE)&temp_value, sizeof(DWORD));
		temp_value = 3;
		RegSetValueExA(phkResult, "Start", 0, REG_DWORD, (LPBYTE)&temp_value, sizeof(DWORD));
		temp_str = "\\??\\" + _Driver_Path;
		RegSetValueExA(phkResult, "ImagePath", 0, REG_EXPAND_SZ, (const unsigned char*)temp_str.data(), temp_str.length());
		RegCloseKey(phkResult);

		return true;
	}
	else
	{
		_Last_Error = GetLastError();
		return false;
	}
}

bool Driver_Load::Nt_Start_Driver()
{
	if (Enable_Debug())
	{
		HMODULE hNtdll = GetModuleHandleA("Ntdll.dll");
		_NtLoadDriver NtLoadDriver = (_NtLoadDriver)GetProcAddress(hNtdll, "NtLoadDriver");
		if (NtLoadDriver == nullptr)
		{
			_Last_Error = GetLastError();
			return false;
		}

		std::string temp_str = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + _Driver_Name;
		UNICODE_STRING uDriver;
		ANSI_STRING asDriverKey;
		RtlInitAnsiString(&asDriverKey, temp_str.data());
		RtlAnsiStringToUnicodeString(&uDriver, &asDriverKey, TRUE);
		ULONG ret = NtLoadDriver(&uDriver);
		RtlFreeUnicodeString(&uDriver);
		if (ret == 0 || ret == 0xc000010e)
		{
			return true;
		}
		else
		{
			char* temp_error = nullptr;
			if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				ret,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(char*)&temp_error,
				0,
				NULL))
			{
				QMessageBox::information(nullptr, ("Error:" + std::to_string(ret)).data(), temp_error);
				LocalFree(temp_error);
			}
			return false;
		}
	}
}

bool Driver_Load::Nt_Stop_Driver()
{
	if (Enable_Debug())
	{
		HMODULE hNtdll = GetModuleHandleA("Ntdll.dll");
		_NtUnLoadDriver NtUnLoadDriver = (_NtLoadDriver)GetProcAddress(hNtdll, "NtUnloadDriver");
		if (NtUnLoadDriver == nullptr)
		{
			_Last_Error = GetLastError();
			return false;
		}

		std::string temp_str = "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + _Driver_Name;
		UNICODE_STRING uDriver;
		ANSI_STRING asDriverKey;
		RtlInitAnsiString(&asDriverKey, temp_str.data());
		RtlAnsiStringToUnicodeString(&uDriver, &asDriverKey, TRUE);
		ULONG ret = NtUnLoadDriver(&uDriver);
		RtlFreeUnicodeString(&uDriver);
		if (ret == 0)
		{
			return true;
		}
		else
		{
			_Last_Error = ret;
			char* temp_error = nullptr;
			if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				ret,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(char*)&temp_error,
				0,
				NULL))
			{
				QMessageBox::information(nullptr, ("Error:" + std::to_string(ret)).data(), temp_error);
				LocalFree(temp_error);
			}
			return false;
		}
	}
}

bool Driver_Load::Nt_UnRegister_Driver()
{
	std::string temp_str = "SYSTEM\\CurrentControlSet\\Services\\" + _Driver_Name;
	if (ERROR_SUCCESS == SHDeleteKeyA(HKEY_LOCAL_MACHINE, temp_str.data()))
	{
		return true;
	}
	_Last_Error = GetLastError();
	return false;
}