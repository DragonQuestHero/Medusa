#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QTimer>

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
#include "KernelModules.h"
#include "IOCTLScanner.h"
#include "KernelMemory.h"
#include "UserMemory.h"
#include "UserMemoryList.h"
#include "ModuleExportFunc.h"


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
	void HideProcess(bool);
	void DriverRightMenu(QAction*);
	void DriverRightMenuDumpToFILE(bool);
	void DriverRightMenuDumpToMemory(bool);
	void DriverRightMenuViewExportFunc(bool);
	void DriverRightMenuIOCTLScanner(QAction*);
	void ViewKernelMemory(QAction*);
public:
	void GetProcessList();
	void GetKernelModuleList();
	void GetUnLoadKernelModuleList();
	void GetALLCallBackList();
	void GetSSDT();
	void GetShadowSSDT();
public:
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
	bool _Setting_SSDT_SSSDT_PDB = false;
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
	KernelModules _KernelModules;
	IOCTLScanner _IOCTLScanner;
	KernelMemory _KernelMemory;
	UserMemory _UserMemory;
	UserMemoryList _UserMemoryList;
	ModuleExportFunc _ModuleExportFunc;
private:
	QTimer *_QTimer;
    QStandardItemModel* _Model;
	QStandardItemModel* _Model_Driver;
	QStandardItemModel* _Model_UnloadDriver;
	QStandardItemModel* _Model_CallBackList;
	QStandardItemModel* _Model_SSDT;
	QStandardItemModel* _Model_SSSDT;
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

	QMenu _TableView_Menu_KillProcess;
	QAction _TableView_Action_KillProcess;

	QAction _TableView_Action_HideDriver;

	QAction _TableView_Action_ViewExportFunc;

	QMenu _TableView_Menu_DriverClear;
	QAction _TableView_Action_DriverClear;

	QMenu _TableView_Menu_PDBViewProcess;
	QAction _TableView_Action_PDBViewProcess;

	QMenu _TableView_Menu_Memory;
	QAction _TableView_Action_Memory;

	QAction _TableView_Action_DriverDumpFILE;
	QAction _TableView_Action_DriverDumpMemory;

	QMenu _TableView_Menu_IOCTLScanner;
	QAction _TableView_Action_IOCTLScanner;
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
	}
	void DriverUI()
	{
		_Model_Driver = new QStandardItemModel();
		ui.tableView_Driver->setModel(_Model_Driver);
		ui.tableView_Driver->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView_Driver->horizontalHeader()->setSectionsClickable(false);
		ui.tableView_Driver->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView_Driver->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model_Driver->setColumnCount(7);
		_Model_Driver->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model_Driver->setHeaderData(1, Qt::Horizontal, u8"Name");
		_Model_Driver->setHeaderData(2, Qt::Horizontal, u8"Addr");
		_Model_Driver->setHeaderData(3, Qt::Horizontal, u8"Size");
		_Model_Driver->setHeaderData(4, Qt::Horizontal, u8"Path");
		_Model_Driver->setHeaderData(5, Qt::Horizontal, u8"Desciption");
		_Model_Driver->setHeaderData(6, Qt::Horizontal, u8"DriverObject");
		ui.tableView_Driver->setColumnWidth(0, 50);
		ui.tableView_Driver->setColumnWidth(1, 150);
		ui.tableView_Driver->setColumnWidth(2, 180);
		ui.tableView_Driver->setColumnWidth(3, 150);
		ui.tableView_Driver->setColumnWidth(4, 400);
		ui.tableView_Driver->setColumnWidth(5, 500);
		ui.tableView_Driver->setColumnWidth(6, 180);
		ui.tableView_Driver->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	void UnloadDriverUI()
	{
		_Model_UnloadDriver = new QStandardItemModel();
		ui.tableView_UnloadDriver->setModel(_Model_UnloadDriver);
		ui.tableView_UnloadDriver->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView_UnloadDriver->horizontalHeader()->setSectionsClickable(false);
		ui.tableView_UnloadDriver->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView_UnloadDriver->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model_UnloadDriver->setColumnCount(5);
		_Model_UnloadDriver->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model_UnloadDriver->setHeaderData(1, Qt::Horizontal, u8"Name");
		_Model_UnloadDriver->setHeaderData(2, Qt::Horizontal, u8"Addr");
		_Model_UnloadDriver->setHeaderData(3, Qt::Horizontal, u8"Size");
		_Model_UnloadDriver->setHeaderData(4, Qt::Horizontal, u8"UnLoadTime");
		ui.tableView_UnloadDriver->setColumnWidth(0, 50);
		ui.tableView_UnloadDriver->setColumnWidth(1, 600);
		ui.tableView_UnloadDriver->setColumnWidth(2, 200);
		ui.tableView_UnloadDriver->setColumnWidth(3, 150);
		ui.tableView_UnloadDriver->setColumnWidth(4, 200);
		ui.tableView_UnloadDriver->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	void CallBackListUI()
	{
		_Model_CallBackList = new QStandardItemModel();
		ui.tableView_CallBackList->setModel(_Model_CallBackList);
		ui.tableView_CallBackList->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView_CallBackList->horizontalHeader()->setSectionsClickable(false);
		ui.tableView_CallBackList->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView_CallBackList->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model_CallBackList->setColumnCount(6);
		_Model_CallBackList->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model_CallBackList->setHeaderData(1, Qt::Horizontal, u8"Name");
		_Model_CallBackList->setHeaderData(2, Qt::Horizontal, u8"PreAddr");
		_Model_CallBackList->setHeaderData(3, Qt::Horizontal, u8"PostAddr");
		_Model_CallBackList->setHeaderData(4, Qt::Horizontal, u8"Type");
		_Model_CallBackList->setHeaderData(5, Qt::Horizontal, u8"PointerAddr");
		ui.tableView_CallBackList->setColumnWidth(0, 50);
		ui.tableView_CallBackList->setColumnWidth(1, 300);
		ui.tableView_CallBackList->setColumnWidth(2, 200);
		ui.tableView_CallBackList->setColumnWidth(3, 200);
		ui.tableView_CallBackList->setColumnWidth(4, 250);
		ui.tableView_CallBackList->setColumnWidth(5, 200);
		ui.tableView_CallBackList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	void SSDTListUI()
	{
		_Model_SSDT = new QStandardItemModel();
		ui.tableView_SSDT->setModel(_Model_SSDT);
		ui.tableView_SSDT->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView_SSDT->horizontalHeader()->setSectionsClickable(false);
		ui.tableView_SSDT->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView_SSDT->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model_SSDT->setColumnCount(4);
		_Model_SSDT->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model_SSDT->setHeaderData(1, Qt::Horizontal, u8"Name");
		_Model_SSDT->setHeaderData(2, Qt::Horizontal, u8"Addr");
		_Model_SSDT->setHeaderData(3, Qt::Horizontal, u8"Moudle");
		ui.tableView_SSDT->setColumnWidth(0, 50);
		ui.tableView_SSDT->setColumnWidth(1, 400);
		ui.tableView_SSDT->setColumnWidth(2, 300);
		ui.tableView_SSDT->setColumnWidth(3, 300);
		ui.tableView_SSDT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	void ShadowSSDTListUI()
	{
		_Model_SSSDT = new QStandardItemModel();
		ui.tableView_SSSDT->setModel(_Model_SSSDT);
		ui.tableView_SSSDT->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.tableView_SSSDT->horizontalHeader()->setSectionsClickable(false);
		ui.tableView_SSSDT->verticalHeader()->setDefaultSectionSize(25);
		ui.tableView_SSSDT->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model_SSSDT->setColumnCount(4);
		_Model_SSSDT->setHeaderData(0, Qt::Horizontal, u8"Index");
		_Model_SSSDT->setHeaderData(1, Qt::Horizontal, u8"Name");
		_Model_SSSDT->setHeaderData(2, Qt::Horizontal, u8"Addr");
		_Model_SSSDT->setHeaderData(3, Qt::Horizontal, u8"Moudle");
		ui.tableView_SSSDT->setColumnWidth(0, 50);
		ui.tableView_SSSDT->setColumnWidth(1, 400);
		ui.tableView_SSSDT->setColumnWidth(2, 300);
		ui.tableView_SSSDT->setColumnWidth(3, 300);
		ui.tableView_SSSDT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
public:
	void ProcessRightMenuUI()
	{
		_TableView_Action_Inject.setMenu(&_TableView_Menu_Inject);
		_TableView_Menu_Inject.setTitle("Inject DLL");
		_TableView_Menu_Inject.addAction("R3CreateRemoteThread+LoadLibraryA");
		_TableView_Menu_Inject.addAction("R3SetThreadContext+LoadLibrary");
		_TableView_Menu_Inject.addAction("R3APCInject");
		_TableView_Menu_Inject.addAction("R3MapInject");
		_TableView_Menu_Inject.addAction("R0MapInject");



		_TableView_Action_HookCheck.setMenu(&_TableView_Menu_HookCheck);
		_TableView_Menu_HookCheck.setTitle("HookScanner");
		_TableView_Menu_HookCheck.addAction("HookScannerSimple(Y/N)");
		_TableView_Menu_HookCheck.addAction("HookScanner");
		_TableView_Menu_HookCheck.addAction("HookScannerQuick");
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

		_TableView_Action_KillProcess.setMenu(&_TableView_Menu_KillProcess);
		_TableView_Menu_KillProcess.setTitle("KillProcess");
		_TableView_Menu_KillProcess.addAction("R3KillProcess");
		_TableView_Menu_KillProcess.addAction("R0KillProcess");

		_TableView_Action_PDBViewProcess.setMenu(&_TableView_Menu_PDBViewProcess);
		_TableView_Menu_PDBViewProcess.setTitle("PDBViewProcess");
		_TableView_Menu_PDBViewProcess.addAction("_EPROCESS");
		_TableView_Menu_PDBViewProcess.addAction("_KPROCESS");

		_TableView_Action_HideProcess.setText("HideProcess");

		_TableView_Action_Memory.setMenu(&_TableView_Menu_Memory);
		_TableView_Menu_Memory.setTitle("Memory");
		_TableView_Menu_Memory.addAction("MemoryListR3");
		_TableView_Menu_Memory.addAction("MemoryListR3(second check)");
		_TableView_Menu_Memory.addAction("MemoryListR0");
		_TableView_Menu_Memory.addAction("ViewMemory");

		ui.tableView->addAction(&_TableView_Action_Inject);
		ui.tableView->addAction(&_TableView_Action_HookCheck);
		ui.tableView->addAction(&_TableView_Action_Modules);
		ui.tableView->addAction(&_TableView_Action_Threads);
		ui.tableView->addAction(&_TableView_Action_Memory);
		ui.tableView->addAction(&_TableView_Action_KillProcess);
		ui.tableView->addAction(&_TableView_Action_PDBViewProcess);
		ui.tableView->addAction(&_TableView_Action_HideProcess);
	}
	void DriverRightMenuUI()
	{
		_TableView_Action_HideDriver.setText("HideDriver");

		_TableView_Action_ViewExportFunc.setText("ViewExportFunc");

		_TableView_Action_DriverClear.setMenu(&_TableView_Menu_DriverClear);
		_TableView_Menu_DriverClear.setTitle("DriverClear");
		_TableView_Menu_DriverClear.addAction("ClearLoadInfo");
		_TableView_Menu_DriverClear.addAction("ClearLoadInfo(usepdb)");

		_TableView_Action_DriverDumpFILE.setText("DriverDumpToFILE");
		_TableView_Action_DriverDumpMemory.setText("DriverDumpToMemory");

		_TableView_Action_IOCTLScanner.setMenu(&_TableView_Menu_IOCTLScanner);
		_TableView_Menu_IOCTLScanner.setTitle("IOCTLScanner");
		_TableView_Menu_IOCTLScanner.addAction("ViewIOCTL-Functions");
		_TableView_Menu_IOCTLScanner.addAction("ScanIOCTLHook");
		_TableView_Menu_IOCTLScanner.addAction("ScanAllDriverIOCTLHook");


		ui.tableView_Driver->addAction(&_TableView_Action_HideDriver);
		ui.tableView_Driver->addAction(&_TableView_Action_DriverClear);
		ui.tableView_Driver->addAction(&_TableView_Action_DriverDumpFILE);
		ui.tableView_Driver->addAction(&_TableView_Action_DriverDumpMemory);
		ui.tableView_Driver->addAction(&_TableView_Action_IOCTLScanner);
		ui.tableView_Driver->addAction(&_TableView_Action_ViewExportFunc);
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
	std::wstring ReplaceStr(std::wstring& str,
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
	std::string ReplaceStr2(std::string& str,
		const std::string& old_value, const std::string& new_value)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
			if ((pos = str.find(old_value, pos)) != std::string::npos)
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
