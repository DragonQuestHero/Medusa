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


#include "ui_PDBView.h"


#include "PDBInfo.h"


class PDBView : public QMainWindow
{
	Q_OBJECT

public:
	PDBView(QWidget* parent = nullptr);
	~PDBView() = default;
public:
	void ThreadLoadALL();
public:
public slots:
	void LoadALL();
	void Serch();
public:
	Ui::Form_PDBView ui;
	QStandardItemModel* _Model;
private:
public:
	PDBInfo _PDBInfo;
};