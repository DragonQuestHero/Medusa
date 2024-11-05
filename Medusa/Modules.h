#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QMenu>

#include <fstream>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include <vector>
#include <string>


#include "ui_Modules.h"
#include "ModuleExportFunc.h"


struct UserModule
{
	ULONG64 Addr;
	ULONG64 Size;
	WCHAR Name[260];
	WCHAR Path[260];
};

class Modules : public QMainWindow
{
	Q_OBJECT

public:
	Modules(QWidget* parent = nullptr);
	~Modules() = default;
public:
	std::vector<MODULEENTRY32W> GetUserMoudleListR3(ULONG64 PID);
	std::vector<UserModule> GetUserMoudleListR0(ULONG64 PID);
	std::vector<UserModule> R3ModuleScanner(ULONG64 PID, HANDLE handle);
public:
public slots:
	void Dump(bool);
	void DumpToFile(bool);
	void ViewExportFunc(bool);
public:
	Ui::Form_Modules ui;
	QStandardItemModel* _Model;
	ModuleExportFunc _ModuleExportFunc;
	QAction _TableView_Action_Dump;
	QAction _TableView_Action_DumpToFile;
	QAction _TableView_Action_ViewExportFunc;
private:
	ULONG64 _PID;
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
};