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
#include "HookScanner.h"
#include "Modules.h"

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
	void ProcessRightMenu(QAction*);
	void DriverLoad(QAction*);
	void GetProcessList();
	void R3ModulesView(ULONG64 PID);
	void R0ModulesView(ULONG64 PID);
private:
	bool _Driver_Loaded = false;
private:
    Process _Process;
	Driver_Load _Driver_Load;
	Driver_Load _Driver_Load_Other;
private:
    Ui::MedusaClass ui;
	HookScanner _HookScanner;
	Modules _Modules;
    QStandardItemModel* _Model;
private:
	QMenu _TableView_Menu_Inject;
	QAction _TableView_Action_Inject;

	QMenu _HookCheck;
	QAction _Hook_QAction_Check;

	QMenu _TableView_Menu_Modules;
	QAction _TableView_Action_Modules;
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
};
