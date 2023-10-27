#include "DLLInject.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Threads.h"
#include "DLLInjectShellCode.h"



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
	return false;
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

bool RemoteInjectDll(DWORD ProcessId, PVOID DllData, SIZE_T dwSize)//来自群友赞助 这年头不会真有人自己动手写代码吧?
{
	if (!ProcessId || !DllData || !dwSize) {
		return false;
	}

	HANDLE hProcess{};
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	if (!hProcess) {
		return false;
	}

	auto ShellCode = VirtualAllocEx(hProcess, nullptr, sizeof(MemLoadShellcode_x64), MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);
	if (!ShellCode) {
		CloseHandle(hProcess);
		return false;
	}
	auto DllPtr = VirtualAllocEx(hProcess, nullptr, dwSize, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE);
	if (!DllPtr) {
		VirtualFreeEx(hProcess, ShellCode, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	WriteProcessMemory(hProcess, ShellCode, MemLoadShellcode_x64, sizeof(MemLoadShellcode_x64), nullptr);
	WriteProcessMemory(hProcess, DllPtr, DllData, dwSize, nullptr);
	auto Thread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)ShellCode, DllPtr, 0, nullptr);
	if (Thread)
	{
		WaitForSingleObject(Thread, INFINITE);
		CloseHandle(Thread);
	}

	VirtualFreeEx(hProcess, ShellCode, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, DllPtr, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return true;
}

bool DLLInject::R3MapInject(ULONG64 PID)
{
	QFileDialog file_path;
	QString temp_str = file_path.getOpenFileName();
	if (temp_str.size() == 0)
	{
		return false;
	}
	temp_str = QDir::toNativeSeparators(temp_str);
	std::string file_str = Read_ALL(temp_str.toStdString());
	return RemoteInjectDll(PID, (void*)file_str.data(), file_str.size());
}

