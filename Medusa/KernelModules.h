#pragma once
#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>

#include <fstream>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include <vector>
#include <string>

#include "ModuleExportFunc.h"


struct KernelModulesVector2
{
	ULONG64 Addr;
	ULONG64 Size;
	ULONG64 DriverObject;
	WCHAR Name[260];
	char Path[260];
	USHORT Check;
};

struct KernelModulesVector
{
	ULONG64 Addr;
	ULONG64 Size;
	ULONG64 DriverObject;
	std::string Name;
	std::string Path;
	USHORT Check;
};

struct KernelUnloadModules
{
	ULONG64 Addr;
	ULONG64 Size;
	ULONG64 UnLoadTime;
	WCHAR Name[260];
	USHORT Check;
};

struct SSDT_STRUCT
{
	ULONG64 Index;
	ULONG64 Addr;
};

struct SSDT_STRUCT2
{
	ULONG64 Index;
	ULONG64 Addr;
	std::string FuncName;
	std::string Modules;
};

class KernelModules
{
public:
	KernelModules() = default;
	~KernelModules() = default;
public:
	bool GetKernelModuleListR3();
	bool GetKernelModuleListR0();
	bool GetUnLoadKernelModuleListR0();
	bool DumpDriver(ULONG64 Address, ULONG64, void*);
	std::vector<SSDT_STRUCT2> GetALLSSDT(bool);
	std::vector<SSDT_STRUCT2> GetALLShadowSSDT(bool);
	bool DriverUnload(ULONG64 Address);
public:
	bool KernelModules::IsAddressInAnyDriversList(ULONG64 Address)
	{
		for (auto x : _KernelModuleList)
		{
			if (Address >= x.Addr && Address < x.Addr + x.Size)
				return true;
		}
		return false;
	}
	int KernelModules::GetDriversListIndexFromAddress(ULONG64 Address)
	{
		int i = 0;
		for (auto x : _KernelModuleList)
		{
			if (Address >= x.Addr && Address < x.Addr + x.Size)
			{
				return i;
			}
			i++;
		}
		return -1;
	}
	int KernelModules::GetDriversListIndexFromName(std::string name)
	{
		int i = 0;
		for (auto x : _KernelModuleList)
		{
			if (name == x.Name)
			{
				return i;
			}
			i++;
		}
		return -1;
	}
	bool KernelModules::IsAddressInDriversList(KernelModulesVector KernelModule, ULONG64 Address)
	{
		if (Address >= (ULONG64)KernelModule.Addr &&
			Address < (ULONG64)KernelModule.Addr + (ULONG64)KernelModule.Size)
		{
			return true;
		}
		return false;
	}
public:
	std::vector<KernelModulesVector> _KernelModuleListR3;
	std::vector<KernelModulesVector> _KernelModuleListR0;
	std::vector<KernelModulesVector> _KernelModuleList;
	std::vector<KernelUnloadModules> _KernelUnLoadModuleListR0;
	std::vector<SSDT_STRUCT2> _SSDTALL;
	std::vector<SSDT_STRUCT2> _SSSDTALL;
private:
	std::wstring C_TO_W(std::string str)
	{
		std::wstring result;
		DWORD strsize = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, NULL, 0);
		wchar_t* pwstr = new wchar_t[strsize];
		MultiByteToWideChar(CP_ACP, 0, str.data(), -1, pwstr, strsize);
		result = pwstr;
		delete[] pwstr;
		return result;
	}
	std::string W_TO_C(std::wstring str)
	{
		std::string result;
		DWORD strsize = WideCharToMultiByte(CP_ACP, 0, str.data(), -1, NULL, 0, NULL, NULL);
		char* pstr = new char[strsize];
		WideCharToMultiByte(CP_ACP, 0, str.data(), -1, pstr, strsize, NULL, NULL);
		result = pstr;
		return result;
	}
	std::string ConvertSystemRootPath(const std::string& path) {
		// 获取系统目录
		char systemDir[MAX_PATH];
		GetSystemDirectoryA(systemDir, MAX_PATH);

		// 获取 Windows 目录
		char windowsDir[MAX_PATH];
		GetWindowsDirectoryA(windowsDir, MAX_PATH);

		// 检查路径是否以 \SystemRoot 开头
		if (path.find("\\SystemRoot") == 0) {
			// 替换为 Windows 目录
			return std::string(windowsDir) + path.substr(strlen("\\SystemRoot"));
		}

		return path; // 如果不匹配，返回原路径
	}
};

