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


#include "ui_UserMemoryList.h"
#include "UserMemory.h"
#include "UserMemoryList.h"



class UserMemoryListKernel : public QMainWindow
{
	Q_OBJECT

public:
	UserMemoryListKernel(QWidget* parent = nullptr);
	~UserMemoryListKernel() = default;
public:
	bool ShowUserMemoryListR0(ULONG64 PID);
	bool ShowUserMemoryListR0_2(ULONG64 PID);
	bool ShowUserMemoryListR0ALL(ULONG64 PID);
	std::vector<UserMemoryListStruct> GetUserMemoryListR3(ULONG64 PID);
	std::vector<UserMemoryListStruct> GetUserMemoryListR3CheckHide(ULONG64 PID);
	std::vector<UserMemoryListStructCR3> GetUserMemoryListR0(ULONG64 PID);
public:
public slots:
	void MemoryView(bool);
public:
	Ui::Form_UserMemoryList ui;
	QStandardItemModel* _Model;
	QAction _TableView_Action_Check;
	UserMemory _UserMemory;
	UserMemoryList _UserMemoryList;
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