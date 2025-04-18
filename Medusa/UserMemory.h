#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QScrollBar>

#include <fstream>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "ui_UserMemory.h"
#include "ntdll.h"


struct Message_NtReadWriteVirtualMemory
{
	HANDLE ProcessId;
	HANDLE ProcessHandle;
	PVOID BaseAddress;
	PVOID Buffer;
	SIZE_T BufferBytes;
	PSIZE_T ReturnBytes;
	bool Read;
};


class UserMemory : public QMainWindow
{
	Q_OBJECT
public:
	UserMemory(QWidget* parent = nullptr);
	~UserMemory() = default;
public:
	ULONG64 ReadUserMemoryR3(ULONG64 Addr, ULONG64 Size, void* Buffer);
	ULONG64 ReadUserMemoryR0(ULONG64 Addr, ULONG64 Size, void* Buffer);
	ULONG64 WriteUserMemoryR3(ULONG64 Addr, ULONG64 Size, void* Buffer);
	ULONG64 WriteUserMemoryR0(ULONG64 Addr, ULONG64 Size, void* Buffer);
	ULONG64 GetKernelCR3();
public:
	void QueryMemoryTable1(char* temp_buffer, ULONG64 ret, ULONG64 Addr);
	void QueryMemoryTable2(char* temp_buffer, ULONG64 ret, ULONG64 Addr, ULONG64 Size);
public slots:
	void DumpMemory();
	void DumpASM();
	void QueryMemory();
	void TexeBar(int value);
	void ChangeTab();
public:
	Ui::Form_UserMemory ui;
	QStandardItemModel* _Model;
	ULONG64 PID = 0;
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
