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


#include "ui_Modules.h"


class Modules : public QMainWindow
{
	Q_OBJECT

public:
	Modules(QWidget* parent = nullptr);
	~Modules() = default;
public:
	std::vector<MODULEENTRY32W> GetWin32MoudleList(ULONG64 PID);
public slots:
public:
	Ui::Form_Modules ui;
	QStandardItemModel* _Model;
};