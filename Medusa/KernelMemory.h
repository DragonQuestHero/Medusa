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

#include "ui_KernelMemory.h"



class KernelMemory : public QMainWindow
{
	Q_OBJECT
public:
	KernelMemory(QWidget* parent = nullptr);
	~KernelMemory() = default;
public:
	ULONG64 ReadKernelMemory(ULONG64 Addr, ULONG64 Size, void* Buffer);
public:
	void QueryMemoryTable1(char* temp_buffer, ULONG64 ret, ULONG64 Addr);
	void QueryMemoryTable2(char* temp_buffer, ULONG64 ret, ULONG64 Addr, ULONG64 Size);
public slots:
	void DumpMemory();
	void QueryMemory();
	void TexeBar(int value);
public:
	Ui::Form_KernelMemory ui;
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
