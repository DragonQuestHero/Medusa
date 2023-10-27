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

#include "ui_IOCTLScanner.h"
#include "KernelModules.h"

struct IOCTLS
{
	ULONG64 Index;
	ULONG64 Addr;
	bool Check;
};


class IOCTLScanner : public QMainWindow
{
	Q_OBJECT
public:
	IOCTLScanner(QWidget* parent = nullptr);
	~IOCTLScanner() = default;
public:
	bool GetIOCTLFunction(ULONG64 Addr, KernelModules &_KernelModules,std::string);
	bool QueryIOCTLHook(ULONG64 Addr, KernelModules& _KernelModules, std::string);
public:
	std::string IOCTLScanner::GetFunctionName(ULONG64 Index);
public slots:
public:
	Ui::Form_IOCTLScanner ui;
	QStandardItemModel* _Model;
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
};
