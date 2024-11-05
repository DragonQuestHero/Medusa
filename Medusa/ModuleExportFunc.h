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


#include "ui_ModuleExportFunc.h"

struct ExportFunc
{
	ULONG64 RVA;
	ULONG64 Addr;
	std::string Name;
};


class ModuleExportFunc : public QMainWindow
{
	Q_OBJECT
public:
	ModuleExportFunc(QWidget* parent = nullptr);
	~ModuleExportFunc() = default;
public:
	std::vector<ExportFunc> GetExportFunc(ULONG64 Addr, std::string Path);
public:
public slots:
public:
	Ui::Form_ModuleExportFunc ui;
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
	std::string Read_ALL(std::string file_name)
	{
		std::string result;
		//n �� binary �� ios ���ﶨ����Զ����Ʒ�ʽ���ļ����ڶ��룬�� | �� �������
		std::fstream file(file_name, std::ios::in | std::ios::binary);
		if (file.is_open() == false)
		{
			return "Error";
		}
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		result = str;
		return result;
	}
};