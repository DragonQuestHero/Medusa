#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>

#include <windows.h>

#include "ui_HookScanner.h"

#include "UserMemory.h"


class HookScanner : public QMainWindow
{
	Q_OBJECT

public:
	HookScanner(QWidget* parent = nullptr);
	~HookScanner() = default;
public:
	bool ProcessHookScanner(ULONG64 PID, bool kernel_mode);
	bool ProcessHookScannerQuick(ULONG64, bool);
public slots:
	void MemoryView(bool);
public:
	Ui::From_HookScanner ui;
	QStandardItemModel* _Model;
public:
	ULONG64 _PID = 0;
	QAction _TableView_Action_UserMemory;
	UserMemory _UserMemory;
private:
	std::string String_TO_HEX(const std::string& s, bool upper_case = true)
	{
		std::ostringstream ret;
		for (std::string::size_type i = 0; i < s.length(); ++i)
			ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)(unsigned char)s[i] << " ";
		return ret.str();
	}
};