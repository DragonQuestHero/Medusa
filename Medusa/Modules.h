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


#include "ui_Modules.h"


struct UserModule
{
	ULONG64 Addr;
	ULONG64 Size;
	WCHAR Name[260];
	WCHAR Path[260];
};

class Modules : public QMainWindow
{
	Q_OBJECT

public:
	Modules(QWidget* parent = nullptr);
	~Modules() = default;
public:
	std::vector<MODULEENTRY32W> GetUserMoudleListR3(ULONG64 PID);
	std::vector<UserModule> GetUserMoudleListR0(ULONG64 PID);
	std::vector<UserModule> R3ModuleScanner(ULONG64 PID, HANDLE handle);
public:
public slots:
	void Dump(bool);
public:
	Ui::Form_Modules ui;
	QStandardItemModel* _Model;
	QAction _TableView_Action_Dump;
private:
	ULONG64 _PID;
};