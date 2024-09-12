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

using UserMemoryListStruct = struct
{
	ULONG64 Addr;
	ULONG64 Size;
	ULONG64 Protect;
	ULONG64 Type;
	ULONG64 State;
	ULONG64 Color;
};


class UserMemoryList : public QMainWindow
{
	Q_OBJECT

public:
	UserMemoryList(QWidget* parent = nullptr);
	~UserMemoryList() = default;
public:
	void SetUserMemory(UserMemory* _UserMemory)
	{
		this->_UserMemory = _UserMemory;
	}
	bool ShowUserMemoryListR3(ULONG64 PID, bool CheckHide);
	bool ShowUserMemoryListR0(ULONG64 PID);
	std::vector<UserMemoryListStruct> GetUserMemoryListR3(ULONG64 PID);
	std::vector<UserMemoryListStruct> GetUserMemoryListR3CheckHide(ULONG64 PID);
	std::vector<UserMemoryListStruct> GetUserMemoryListR0(ULONG64 PID);
public:
public slots:
	void Check(bool);
public:
	Ui::Form_UserMemoryList ui;
	QStandardItemModel* _Model;
	QAction _TableView_Action_Check;
	UserMemory* _UserMemory = nullptr;
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