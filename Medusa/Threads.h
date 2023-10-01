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


#include "ui_Threads.h"

#include "StackWalk.h"

struct ThreadList
{
	ULONG64 TID;
	ULONG64 ETHREAD;
	ULONG64 StartAddr;
	WCHAR Name[MAX_PATH];
};


class Threads : public QMainWindow
{
	Q_OBJECT

public:
	Threads(QWidget* parent = nullptr);
	~Threads() = default;
public:
	std::vector<ThreadList> GetThreadListR3(ULONG64 PID);
	std::vector<ThreadList> GetThreadListR0(ULONG64 PID);
	std::vector<ULONG64> GetStackWalkThreadR0(ULONG64);
public:
public slots:
	void ShowStackWalkThread(bool);
public:
	Ui::Form_Threads ui;
	QStandardItemModel* _Model;
	QAction _TableView_Action_ThreadStackWalk;
private:
	StackWalk _StackWalk;
};