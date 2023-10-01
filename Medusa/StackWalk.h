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

#include "ui_StackWalk.h"


class StackWalk : public QMainWindow
{
	Q_OBJECT
public:
	StackWalk(QWidget* parent = nullptr);
	~StackWalk() = default;
public:
	std::vector<ULONG64> GetStackWalkThreadR0(ULONG64 TID);
	void ShowStackWalkThreadR0(ULONG64 TID);
public:
public slots:
public:
	Ui::Form_StackWalk ui;
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
};
