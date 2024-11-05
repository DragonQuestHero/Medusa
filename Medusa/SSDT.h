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


struct SSDT_STRUCT
{
	ULONG64 Index;
	ULONG64 Addr;
	std::string FuncName;
	std::string Modules;
};

class SSDT
{
public:
	SSDT() = default;
	~SSDT() = default;
public:
	void GetALLSSDT();
	void GetALLShadowSSDT();
private:

};

