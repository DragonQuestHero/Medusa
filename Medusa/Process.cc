#include "Process.h"



std::vector<ModuleList> Process::GetModuleListR3(ULONG64 PID)
{
	std::vector<ModuleList> temp_vector;

	HANDLE        hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,PID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		int a = GetLastError();
		return temp_vector;
	}

	if (!Module32First(hModuleSnap, &me32)) 
	{
		CloseHandle(hModuleSnap);
		return temp_vector;
	}

	do {
		ModuleList temp_list;
		RtlCopyMemory(temp_list.Name, me32.szModule, MAX_MODULE_NAME32 + 1);
		RtlCopyMemory(temp_list.Path, me32.szExePath, MAX_PATH);
		temp_vector.push_back(temp_list);
	} while (Module32Next(hModuleSnap, &me32));
	CloseHandle(hModuleSnap);
	return temp_vector;
}


bool Process::GetProcessList(bool _Driver_Loaded)
{
	if (_Driver_Loaded)
	{
		GetProcessListFromR0();
	}
	return GetProcessListR3();
}

bool Process::GetProcessListR3()
{
	_Process_List_R3.clear();

	BOOL            next = 0;
	PROCESSENTRY32 lppe = { 0 };
	DWORD            targetPid = 0;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot)
	{
		lppe.dwSize = sizeof(lppe);
		next = Process32First(hSnapshot, &lppe);
		while (next)
		{
			PROCESS_LIST temp_list;
			RtlZeroMemory(&temp_list, sizeof(PROCESS_LIST));
			temp_list.PID = lppe.th32ProcessID;
			RtlCopyMemory(temp_list.Name, lppe.szExeFile, MAX_PATH);

			std::vector<ModuleList> temp_vector = GetModuleListR3(lppe.th32ProcessID);
			for (auto x : temp_vector)
			{
				if (wcscmp(temp_list.Name, x.Name) == 0)
				{
					RtlCopyMemory(temp_list.Path, x.Path, MAX_PATH);
					break;
				}
			}
			_Process_List_R3.push_back(temp_list);
			next = Process32Next(hSnapshot, &lppe);
		}
	}
	CloseHandle(hSnapshot);
	return targetPid;
}

#define TEST_GetALLProcess CTL_CODE(FILE_DEVICE_UNKNOWN,0x7100,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLProcessNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7101,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool Process::GetProcessListFromR0()
{
	_Process_List_R0.clear();

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	do 
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetALLProcessNumber, 0, 0, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		PROCESS_LIST* temp_list = (PROCESS_LIST*)new char[process_number * sizeof(PROCESS_LIST)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetALLProcess, 0, 0, temp_list, sizeof(PROCESS_LIST) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				_Process_List_R0.push_back(temp_list[i]);
			}
		}
		delete temp_list;

		CloseHandle(m_hDevice);
		return true;
	} while (false);


	


	CloseHandle(m_hDevice);
	return false;
}

//直接抄 费那劲写他干啥
bool Process::QueryValue(const std::wstring& valueName, const LPCWSTR& szModuleName, std::wstring& retStr)
{
	bool bSuccess = FALSE; // 判断是否获取文件信息
	BYTE* pVersionData = NULL;
	DWORD LangCharset = 0;
	WCHAR* pTempStr = NULL;

	do
	{
		if (!valueName.size() || !wcslen(szModuleName))
			break;

		DWORD dwHandle;
		// 判断系统能否检索到指定文件的版本信息
		DWORD dwDataSize = ::GetFileVersionInfoSizeW(szModuleName, &dwHandle); // 获取缓冲区大小
		if (dwDataSize == 0)
			break;

		pVersionData = new (std::nothrow) BYTE[dwDataSize]; // 分配缓冲区,std::nothrow保证在内存不足时不抛出异常
		if (NULL == pVersionData)
			break;

		// 检索信息
		if (!::GetFileVersionInfoW(szModuleName, dwHandle, dwDataSize, (void*)pVersionData))
			break;

		UINT nQuerySize;
		DWORD* pTransTable;

		// 设置语言
		if (!::VerQueryValueW(pVersionData, L"\\VarFileInfo\\Translation", (void**)&pTransTable, &nQuerySize))
			break;

		LangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
		if (pVersionData == NULL)
			break;

		pTempStr = new (std::nothrow) WCHAR[128];// 分配缓冲区
		if (NULL == pTempStr)
			break;

		swprintf_s(pTempStr, 128, L"\\StringFileInfo\\%08lx\\%s", LangCharset, valueName.c_str());
		LPVOID lpData;

		// 调用此函数查询前需要先依次调用函数GetFileVersionInfoSize和GetFileVersionInfo
		if (::VerQueryValue((void*)pVersionData, pTempStr, &lpData, &nQuerySize)) {
			retStr = (WCHAR*)lpData;
		}

		bSuccess = TRUE;
	} while (FALSE);

	// 销毁缓冲区
	if (pVersionData)
	{
		delete[] pVersionData;
		pVersionData = NULL;
	}
	if (pTempStr)
	{
		delete[] pTempStr;
		pTempStr = NULL;
	}

	return bSuccess;
}
