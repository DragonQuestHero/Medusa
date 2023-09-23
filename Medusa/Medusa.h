#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "ui_Medusa.h"
#include "ui_HookScanner.h"
#include "ui_Modules.h"
#include "ui_Threads.h"

#include "HookScanner.h"
#include "Modules.h"
#include "Threads.h"
#include "PDBView.h"



#include "Process.h"
#include "Driver_Load.h"

class Medusa : public QMainWindow
{
    Q_OBJECT

public:
    Medusa(QWidget *parent = nullptr);
    ~Medusa() = default;
public:
    void Set_SLOTS();
public slots:
    void ChangeTab();
	void DriverLoadMenu(QAction*);
	void HypervisorMenu(QAction*);
	void PdbMenu(QAction*);
	void ProcessRightMenu(QAction*);
public:
	void GetProcessList();
	void GetKernelModuleList();
	void DriverLoad(QAction*);
	void RightMenuDLLInject(QAction*);
	void RightMenuHookScanner(QAction*);
public:
	void RightMenuR3ThreadsView(ULONG64 PID);
	void RightMenuR0ThreadsView(ULONG64 PID);
public:
	void RightMenuR3ModulesView(ULONG64 PID);
	void RightMenuR0ModulesView(ULONG64 PID);
	void RightMenuR3ModuleScanner(ULONG64 PID);
private:
	bool _Driver_Loaded = false;
private:
    Process _Process;
	Driver_Load _Driver_Load;
	Driver_Load _Driver_Load_Other;
private:
    Ui::MedusaClass ui;
private:
	HookScanner _HookScanner;
	Modules _Modules;
	Threads _Threads;
	PDBView _PDBView;
private:
    QStandardItemModel* _Model;
	QStandardItemModel* _Model_Driver;
private:
	QMenu _TableView_Menu_Inject;
	QAction _TableView_Action_Inject;

	QMenu _TableView_Menu_HookCheck;
	QAction _TableView_Action_HookCheck;

	QMenu _TableView_Menu_Modules;
	QAction _TableView_Action_Modules;

	QMenu _TableView_Menu_Threads;
	QAction _TableView_Action_Threads;

	QAction _TableView_Action_HideProcess;
public:
	void ProcessUI()
	{
		_Model = new QStandardItemModel();
		ui.tableView->setModel(_Model);
		ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView->horizontalHeader()->setSectionsClickable(false);
		ui.tableView->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model->setColumnCount(6);
		_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model->setHeaderData(1, Qt::Horizontal, u8"PID");
		_Model->setHeaderData(2, Qt::Horizontal, u8"Name");
		_Model->setHeaderData(3, Qt::Horizontal, u8"EPROCESS");
		_Model->setHeaderData(4, Qt::Horizontal, u8"Path");
		_Model->setHeaderData(5, Qt::Horizontal, u8"Desciption");
		ui.tableView->setColumnWidth(0, 50);
		ui.tableView->setColumnWidth(1, 100);
		ui.tableView->setColumnWidth(2, 200);
		ui.tableView->setColumnWidth(3, 200);
		ui.tableView->setColumnWidth(4, 500);
		ui.tableView->setColumnWidth(5, 400);
		ui.tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		//-----------------------
		_Model_Driver = new QStandardItemModel();
		ui.tableView_Driver->setModel(_Model_Driver);
		ui.tableView_Driver->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView_Driver->horizontalHeader()->setSectionsClickable(false);
		ui.tableView_Driver->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView_Driver->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model_Driver->setColumnCount(6);
		_Model_Driver->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model_Driver->setHeaderData(1, Qt::Horizontal, u8"Name");
		_Model_Driver->setHeaderData(2, Qt::Horizontal, u8"Addr");
		_Model_Driver->setHeaderData(3, Qt::Horizontal, u8"Size");
		_Model_Driver->setHeaderData(4, Qt::Horizontal, u8"Path");
		_Model_Driver->setHeaderData(5, Qt::Horizontal, u8"Desciption");
		ui.tableView_Driver->setColumnWidth(0, 50);
		ui.tableView_Driver->setColumnWidth(1, 150);
		ui.tableView_Driver->setColumnWidth(2, 200);
		ui.tableView_Driver->setColumnWidth(3, 150);
		ui.tableView_Driver->setColumnWidth(4, 400);
		ui.tableView_Driver->setColumnWidth(5, 500);
		ui.tableView_Driver->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	void ProcessRightMenuUI()
	{
		_TableView_Action_Inject.setMenu(&_TableView_Menu_Inject);
		_TableView_Menu_Inject.setTitle("Inject DLL");
		_TableView_Menu_Inject.addAction("R3CreateRemoteThread+LoadLibraryA");
		//_TableView_Menu_Inject.addAction("R3NtCreateRemoteThread+syscall+shellcode+ldrloadlibaby");
		_TableView_Menu_Inject.addAction("R3APCInject");


		_TableView_Action_HookCheck.setMenu(&_TableView_Menu_HookCheck);
		_TableView_Menu_HookCheck.setTitle("HookScanner");
		_TableView_Menu_HookCheck.addAction("HookScannerSimple(Y/N)");
		_TableView_Menu_HookCheck.addAction("HookScanner");
		_TableView_Menu_HookCheck.addAction("QuickCheckALLProcess");


		_TableView_Action_Modules.setMenu(&_TableView_Menu_Modules);
		_TableView_Menu_Modules.setTitle("Modules");
		_TableView_Menu_Modules.addAction("R3ModulesView");
		_TableView_Menu_Modules.addAction("R0ModulesView(second check)");
		_TableView_Menu_Modules.addAction("R3ModuleScanner");

		_TableView_Action_Threads.setMenu(&_TableView_Menu_Threads);
		_TableView_Menu_Threads.setTitle("ThreadView");
		_TableView_Menu_Threads.addAction("R3ThreadView");
		_TableView_Menu_Threads.addAction("R0ThreadView(second check)");

		_TableView_Action_HideProcess.setText("HideProcess");

		ui.tableView->addAction(&_TableView_Action_Inject);
		ui.tableView->addAction(&_TableView_Action_HookCheck);
		ui.tableView->addAction(&_TableView_Action_Modules);
		ui.tableView->addAction(&_TableView_Action_Threads);
		ui.tableView->addAction(&_TableView_Action_HideProcess);
	}
public:
	int Enable_Debug()
	{
		BOOL fOK = FALSE;
		HANDLE hToken;
		//把一个访问令牌中没有启用该权限但是本身是具有该权限的进程提权
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //打开进程访问令牌  
		{
			//试图修改“调试”特权  
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
			fOK = (GetLastError() == ERROR_SUCCESS);
			CloseHandle(hToken);
		}
		return fOK;
	}
	std::string GBK_To_UTF8(char* p)
	{
		DWORD strsize = MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
		wchar_t* pwstr = new wchar_t[strsize];
		MultiByteToWideChar(CP_ACP, 0, p, -1, pwstr, strsize);
		strsize = WideCharToMultiByte(CP_UTF8, 0, pwstr, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, pwstr, -1, p, strsize, NULL, NULL);
		std::string str = p;
		delete[] pwstr;
		pwstr = NULL;
		return str;
	}
	std::string String_TO_HEX(const std::string& s, bool upper_case = true)
	{
		std::ostringstream ret;
		for (std::string::size_type i = 0; i < s.length(); ++i)
			ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)(unsigned char)s[i] << " ";
		return ret.str();
	}
	bool C_TO_W(std::string str, std::wstring& result)
	{
		DWORD strsize = MultiByteToWideChar(CP_ACP, 0, str.data(), -1, NULL, 0);
		wchar_t* pwstr = new wchar_t[strsize];
		MultiByteToWideChar(CP_ACP, 0, str.data(), -1, pwstr, strsize);
		result = pwstr;
		delete[] pwstr;
		return true;
	}
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
	bool W_TO_C(std::wstring str, std::string& result)
	{
		DWORD strsize = WideCharToMultiByte(CP_ACP, 0, str.data(), -1, NULL, 0, NULL, NULL);
		char* pstr = new char[strsize];
		WideCharToMultiByte(CP_ACP, 0, str.data(), -1, pstr, strsize, NULL, NULL);
		result = pstr;
		return true;
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
	std::wstring Replace(std::wstring& str,
		const std::wstring& old_value, const std::wstring& new_value)
	{
		for (std::wstring::size_type pos(0); pos != std::wstring::npos; pos += new_value.length()) {
			if ((pos = str.find(old_value, pos)) != std::wstring::npos)
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else
			{
				break;
			}
		}
		return str;
	}
};
