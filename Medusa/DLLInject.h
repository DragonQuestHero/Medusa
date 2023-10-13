#pragma once
#include <windows.h>
#include <fstream>

#include <TlHelp32.h>
#include <Psapi.h>


class DLLInject
{
public:
	DLLInject() = default;
	~DLLInject() = default;
public:
	bool R3CreateThread(ULONG64 PID);
	bool R3APCInject(ULONG64 PID);
	bool R3MapInject(ULONG64 PID);
private:
	std::string Read_ALL(std::string file_name)
	{
		std::string result;
		//n 和 binary 是 ios 类里定义的以二进制方式打开文件用于读入，用 | 或 运算隔开
		std::fstream file(file_name, std::ios::in | std::ios::binary);
		if (file.is_open() == false)
		{
			return "";
		}
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		result = str;
		return result;
	}
public:
	bool injectdll_x64(const PROCESS_INFORMATION& pi, std::wstring dll) {
		static unsigned char sc[] = {
			0x9c,                                                                   // pushfq
			0x50,                                                                   // push rax
			0x51,                                                                   // push rcx
			0x52,                                                                   // push rdx
			0x53,                                                                   // push rbx
			0x55,                                                                   // push rbp
			0x56,                                                                   // push rsi
			0x57,                                                                   // push rdi
			0x41, 0x50,                                                             // push r8
			0x41, 0x51,                                                             // push r9
			0x41, 0x52,                                                             // push r10
			0x41, 0x53,                                                             // push r11
			0x41, 0x54,                                                             // push r12
			0x41, 0x55,                                                             // push r13
			0x41, 0x56,                                                             // push r14
			0x41, 0x57,                                                             // push r15
			0x48, 0x83, 0xEC, 0x28,                                                 // sub rsp, 0x28
			0x49, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov  r9, 0  // DllHandle
			0x49, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov  r8, 0  // DllPath
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // mov  rax,0  // LdrLoadDll
			0x48, 0x31,                                                             // xor  rcx,rcx
			0xC9, 0x48,                                                             // xor  rdx,rdx
			0xFF, 0xD0,                                                             // call rax   LdrLoadDll
			0x48, 0x83, 0xC4, 0x28,                                                 // add rsp, 0x28
			0x41, 0x5F,                                                             // pop r15
			0x41, 0x5E,                                                             // pop r14
			0x41, 0x5D,                                                             // pop r13
			0x41, 0x5C,                                                             // pop r12
			0x41, 0x5B,                                                             // pop r11
			0x41, 0x5A,                                                             // pop r10
			0x41, 0x59,                                                             // pop r9
			0x41, 0x58,                                                             // pop r8
			0x5F,                                                                   // pop rdi
			0x5E,                                                                   // pop rsi
			0x5D,                                                                   // pop rbp
			0x5B,                                                                   // pop rbx
			0x5A,                                                                   // pop rdx
			0x59,                                                                   // pop rcx
			0x58,                                                                   // pop rax
			0x9D,                                                                   // popfq
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,                                     // jmp offset
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                          // rip
		};

		DWORD64 pfLoadLibrary = (DWORD64)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "LdrLoadDll");
		if (!pfLoadLibrary) {
			return false;
		}

		struct UNICODE_STRING {
			USHORT    Length;
			USHORT    MaximumLength;
			DWORD64   Buffer;
		};
		SIZE_T memsize = sizeof(DWORD64) + sizeof(UNICODE_STRING) + (dll.size() + 1) * sizeof(wchar_t);
		DWORD64 memory = (DWORD64)VirtualAllocEx(pi.hProcess, NULL, memsize, MEM_COMMIT, PAGE_READWRITE);
		if (!memory) {
			return false;
		}
		DWORD64 shellcode = (DWORD64)VirtualAllocEx(pi.hProcess, NULL, sizeof(sc), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!shellcode) {
			return false;
		}

		UNICODE_STRING us;
		us.Length = (USHORT)(dll.size() * sizeof(wchar_t));
		us.MaximumLength = us.Length + sizeof(wchar_t);
		us.Buffer = memory + sizeof(UNICODE_STRING);

		SIZE_T written = 0;
		BOOL ok = FALSE;
		ok = WriteProcessMemory(pi.hProcess, (void*)memory, &us, sizeof(UNICODE_STRING), &written);
		if (!ok || written != sizeof(UNICODE_STRING)) {
			return false;
		}
		ok = WriteProcessMemory(pi.hProcess, (void*)us.Buffer, dll.data(), us.MaximumLength, &written);
		if (!ok || written != us.MaximumLength) {
			return false;
		}

		CONTEXT ctx = { 0 };
		ctx.ContextFlags = CONTEXT_CONTROL;
		if (!GetThreadContext(pi.hThread, &ctx)) {
			return false;
		}

		DWORD64 handle = us.Buffer + us.MaximumLength;
		memcpy(sc + 30, &handle, sizeof(handle));
		memcpy(sc + 40, &memory, sizeof(memory));
		memcpy(sc + 50, &pfLoadLibrary, sizeof(pfLoadLibrary));
		memcpy(sc + 98, &ctx.Rip, sizeof(ctx.Rip));
		ok = WriteProcessMemory(pi.hProcess, (void*)shellcode, &sc, sizeof(sc), &written);
		if (!ok || written != sizeof(sc)) {
			return false;
		}

		ctx.ContextFlags = CONTEXT_CONTROL;
		ctx.Rip = shellcode;
		if (!SetThreadContext(pi.hThread, &ctx)) {
			return false;
		}
		return true;
	}
	bool injectdll(const PROCESS_INFORMATION& pi, const std::wstring& x64dll) {
		return injectdll_x64(pi, x64dll);
	}
	bool setdebugprivilege() {
		TOKEN_PRIVILEGES tp = { 0 };
		HANDLE hToken = NULL;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
			return false;
		}
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &tp.Privileges[0].Luid)) {
			CloseHandle(hToken);
			return false;
		}
		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL)) {
			CloseHandle(hToken);
			return false;
		}
		CloseHandle(hToken);
		return true;
	}
	DWORD getthreadid(DWORD pid)
	{
		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (h != INVALID_HANDLE_VALUE) {
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			for (BOOL ok = Thread32First(h, &te); ok; ok = Thread32Next(h, &te)) {
				if (te.th32OwnerProcessID == pid) {
					CloseHandle(h);
					return te.th32ThreadID;
				}
			}
		}
		CloseHandle(h);
		return 0;
	}
	void closeprocess(PROCESS_INFORMATION& pi) {
		if (pi.hProcess) CloseHandle(pi.hProcess);
		if (pi.hThread) CloseHandle(pi.hThread);
		pi = { 0 };
	}
	bool openprocess(DWORD pid, PROCESS_INFORMATION& pi) {
		closeprocess(pi);

		static bool ok = setdebugprivilege();
		if (!ok) {
			return false;
		}
		pi.dwProcessId = pid;
		pi.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
		if (!pi.hProcess) {
			closeprocess(pi);
			return false;
		}
		pi.dwThreadId = getthreadid(pi.dwProcessId);
		if (!pi.dwThreadId) {
			closeprocess(pi);
			return false;
		}
		pi.hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, pi.dwThreadId);
		if (!pi.hThread) {
			closeprocess(pi);
			return false;
		}
		return true;
	}
	bool injectdll(DWORD pid, std::wstring x64dll) {
		PROCESS_INFORMATION pi = { 0 };
		if (!openprocess(pid, pi)) {
			return false;
		}
		SuspendThread(pi.hThread);
		if (!injectdll(pi, x64dll)) {
			ResumeThread(pi.hThread);
			closeprocess(pi);
			return false;
		}
		ResumeThread(pi.hThread);
		closeprocess(pi);
		return true;
	}
};



