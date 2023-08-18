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
	WCHAR Name[260];
	char Path[260];
	bool Check;
};


class KernelModules
{
public:
	KernelModules() = default;
	~KernelModules() = default;
public:
	bool GetKernelModuleListR3();
public:
	std::vector<KernelModulesVector> _KernelModuleListR3;
private:

};

