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

typedef union _HardwarePteX64 {
	struct {
		ULONG64 valid : 1;               //!< [0]
		ULONG64 write : 1;               //!< [1]
		ULONG64 owner : 1;               //!< [2]
		ULONG64 write_through : 1;       //!< [3]     PWT
		ULONG64 cache_disable : 1;       //!< [4]     PCD
		ULONG64 accessed : 1;            //!< [5]
		ULONG64 dirty : 1;               //!< [6]
		ULONG64 large_page : 1;          //!< [7]     PAT
		ULONG64 global : 1;              //!< [8]
		ULONG64 copy_on_write : 1;       //!< [9]
		ULONG64 prototype : 1;           //!< [10]
		ULONG64 reserved0 : 1;           //!< [11]
		ULONG64 page_frame_number : 36;  //!< [12:47]
		ULONG64 reserved1 : 4;           //!< [48:51]
		ULONG64 software_ws_index : 11;  //!< [52:62]
		ULONG64 no_execute : 1;          //!< [63]
	}Bits;
	ULONG64 value;
} HardwarePteX64;

typedef union _HardwarePteX64ForWindows {
	struct {
		ULONG64 valid : 1;               //!< [0]
		ULONG64 write : 1;               //!< [1]
		ULONG64 owner : 1;               //!< [2]
		ULONG64 write_through : 1;       //!< [3]     PWT
		ULONG64 cache_disable : 1;       //!< [4]     PCD
		ULONG64 accessed : 1;            //!< [5]
		ULONG64 dirty : 1;               //!< [6]
		ULONG64 large_page : 1;          //!< [7]     PAT
		ULONG64 global : 1;              //!< [8]
		ULONG64 copy_on_write : 1;       //!< [9]
		ULONG64 prototype : 1;           //!< [10]
		ULONG64 reserved0 : 1;           //!< [11]
		ULONG64 page_frame_number : 36;  //!< [12:47]
		unsigned __int64 ReservedForHardware : 4;           //!< [48:51]
		unsigned __int64 ReservedForSoftware : 4;
		unsigned __int64 WsleAge : 4;
		unsigned __int64 WsleProtection : 3;
		ULONG64 no_execute : 1;          //!< [63]
	}Bits;
	ULONG64 value;
} HardwarePteX64ForWindows;

using UserMemoryListStructCR3 = struct
{
	ULONG64 Addr;
	ULONG64 Size;
	HardwarePteX64 PteX64;
};


class UserMemoryList : public QMainWindow
{
	Q_OBJECT

public:
	UserMemoryList(QWidget* parent = nullptr);
	~UserMemoryList() = default;
public:
	bool ShowUserMemoryListR3(ULONG64 PID, bool CheckHide);
	bool ShowUserMemoryListR0(ULONG64 PID);
	bool ShowUserMemoryListR0CheckFromCR3(ULONG64 PID);
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