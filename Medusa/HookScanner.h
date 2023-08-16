#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>

#include <fstream>

#include "ui_HookScanner.h"


class HookScanner : public QMainWindow
{
	Q_OBJECT

public:
	HookScanner(QWidget* parent = nullptr);
	~HookScanner() = default;
public:
public slots:
public:
	Ui::From_HookScanner ui;
	QStandardItemModel* _Model;
};