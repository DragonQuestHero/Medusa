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
#include <sstream>


struct ObjectCallBackList
{
	ULONG64 PreOperation;
	ULONG64 PostOperation;
	ULONG64 CallbackEntry;
	ULONG64 Type;
	WCHAR Name[260];
};


class KernelCallBackScanner
{
public:
	KernelCallBackScanner() = default;
	~KernelCallBackScanner() = default;
public:
	std::vector<ObjectCallBackList> GetALLCallBackList();
public:
	std::vector<ObjectCallBackList> __KernelCallBackListR0;
private:

};

