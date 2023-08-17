#include "DLLInject.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Threads.h"


bool DLLInject::R3CreateThread(ULONG64 PID)
{
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (handle != NULL)
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return false;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		int Path_Len = temp_str.length();
		PVOID New_Get_Addr = VirtualAllocEx(handle, NULL, Path_Len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (New_Get_Addr)
		{
			if (WriteProcessMemory(handle, New_Get_Addr, temp_str.toStdString().data(), Path_Len, 0))
			{
				PROC Get_Load_Addr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
				HANDLE New_Hand = CreateRemoteThread(handle, NULL, 0,
					(LPTHREAD_START_ROUTINE)Get_Load_Addr, New_Get_Addr, 0, NULL);
				if (New_Hand != NULL)
				{
					QMessageBox::information(nullptr, "dll inject", "success");
					VirtualFreeEx(handle, New_Get_Addr, 0, MEM_RELEASE | MEM_DECOMMIT);
					CloseHandle(New_Hand);
					CloseHandle(handle);
					return true;
				}
			}
			VirtualFreeEx(handle, New_Get_Addr, 0, MEM_RELEASE | MEM_DECOMMIT);
		}
		QMessageBox::information(nullptr, "dll inject", "unsuccess");
		CloseHandle(handle);
	}
}

bool DLLInject::R3APCInject(ULONG64 PID)
{
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (handle != NULL)
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return false;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		int Path_Len = temp_str.length();
		PVOID New_Get_Addr = VirtualAllocEx(handle, NULL, Path_Len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (New_Get_Addr)
		{
			if (WriteProcessMemory(handle, New_Get_Addr, temp_str.toStdString().data(), Path_Len, 0))
			{
				PROC Get_Load_Addr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

				Threads _Threads;
				std::vector<ThreadList> temp_vector = _Threads.GetThreadListR3(PID);
				for (auto x : temp_vector)
				{
					HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, x.TID);
					if (hThread)
					{
						SuspendThread(hThread);
						if (QueueUserAPC((PAPCFUNC)Get_Load_Addr, hThread, (ULONG_PTR)New_Get_Addr) != 0)
						{
							//VirtualFreeEx(handle, New_Get_Addr, 0, MEM_RELEASE | MEM_DECOMMIT);
							CloseHandle(hThread);
							//return true;
						}
						ResumeThread(hThread);
					}
				}
			}
			VirtualFreeEx(handle, New_Get_Addr, 0, MEM_RELEASE | MEM_DECOMMIT);
		}
		CloseHandle(handle);
	}
	return true;
}