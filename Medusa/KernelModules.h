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



struct KernelModulesVector
{
	ULONG64 Addr;
	ULONG64 Size;
	ULONG64 DriverObject;
	WCHAR Name[260];
	char Path[260];
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
public:
	std::vector<KernelModulesVector> _KernelModuleListR3;
	std::vector<KernelModulesVector> _KernelModuleListR0;
	std::vector<KernelUnloadModules> _KernelUnLoadModuleListR0;
private:

};

