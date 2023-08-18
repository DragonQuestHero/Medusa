#include "Medusa.h"

#include "FileCheck.h"
#include "Hypervisor.h"
#include "DLLInject.h"
#include "KernelModules.h"



Medusa::Medusa(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	Enable_Debug();

	ProcessUI();
	ProcessRightMenuUI();
	
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);



	Set_SLOTS();



	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != m_hDevice)
	{
		_Driver_Loaded = true;
		CloseHandle(m_hDevice);
	}
	ChangeTab();
}


void Medusa::Set_SLOTS()
{
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(ChangeTab()));//进程
	connect(ui.tableView_Driver, SIGNAL(currentChanged(int)), SLOT(ChangeTab()));//进程

	connect(&_TableView_Menu_Inject, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_HookCheck, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Modules, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Threads, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单


	connect(ui.menuMenu, SIGNAL(triggered(QAction*)), SLOT(DriverLoadMenu(QAction*)));
	connect(ui.menuHypervisor, SIGNAL(triggered(QAction*)), SLOT(HypervisorMenu(QAction*)));
}

void Medusa::DriverLoadMenu(QAction* action)
{
	if (action->text() == "NtLoad" ||
		action->text() == "NormalLoad" ||
		action->text() == "UnLoadMedusaDriver" ||
		action->text() == "Nt" ||
		action->text() == "Normal" ||
		action->text() == "Unload")
	{
		DriverLoad(action);
		return;
	}
}

void Medusa::HypervisorMenu(QAction* action)
{
	if (action->text() == "R3Check")
	{
		Hypervisor _Hypervisor;
		std::string str = u8"×=detected virtua environment\r\n√=not\r\n";
		str = str + "[Checking for known hypervisor vendors]:  ";
		if (_Hypervisor.check_for_known_hypervisor())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		str = str + "[Checking highest low function leaf]:  ";
		if (_Hypervisor.check_highest_low_function_leaf())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		str = str + "[Checking invalid leaf]:  ";
		if (_Hypervisor.check_invalid_leaf())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		str = str + "[Profiling CPUID against FYL2XP1]:  ";
		if (_Hypervisor.take_time_cpuid_against_fyl2xp1())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		QMessageBox::information(this, "Ret", str.data());
	}
}

void Medusa::DriverLoad(QAction* action)
{
	if (!_Driver_Loaded)
	{
		if (action->text() == "NtLoad")
		{
			std::fstream check_file("MedusaKernel.sys", std::ios::in);
			if (check_file.is_open())
			{
				check_file.close();

				char temp_str[MAX_PATH];
				if (GetCurrentDirectoryA(MAX_PATH, temp_str) > 0)
				{
					_Driver_Load.Init(temp_str + std::string("\\MedusaKernel.sys"));
					if (_Driver_Load.Nt_Register_Driver())
					{
						if (_Driver_Load.Nt_Start_Driver())
						{
							_Driver_Loaded = true;
							ChangeTab();
							QMessageBox::information(this, "success", "driver loaded");
							return;
						}
					}
				}
			}
			QMessageBox::information(this, "error", "cannot load driver");
		}
		if (action->text() == "NormalLoad")
		{
			std::fstream check_file("MedusaKernel.sys", std::ios::in);
			if (check_file.is_open())
			{
				check_file.close();

				char temp_str[MAX_PATH];
				if (GetCurrentDirectoryA(MAX_PATH, temp_str) > 0)
				{
					_Driver_Load.Init(temp_str + std::string("\\MedusaKernel.sys"));
					if (_Driver_Load.Register_Driver())
					{
						if (_Driver_Load.Start_Driver())
						{
							_Driver_Loaded = true;
							ChangeTab();
							QMessageBox::information(this, "success", "driver loaded");
							return;
						}
					}
				}
			}
			QMessageBox::information(this, "error", "cannot load driver");
		}
	}
	
	if (action->text() == "UnLoadMedusaDriver")
	{
		if (_Driver_Load._NtModule)
		{
			if (_Driver_Load.Nt_Stop_Driver() && _Driver_Load.Nt_UnRegister_Driver())
			{
				_Driver_Loaded = false;
				QMessageBox::information(this, "success", "driver unload");
				exit(0);
			}
		}
		else
		{
			if (_Driver_Load.Stop_Driver() && _Driver_Load.UnRegister_Driver())
			{
				_Driver_Loaded = false;
				QMessageBox::information(this, "success", "driver unload");
				exit(0);
			}
		}
		
	}



	//加载其他驱动
	if (action->text() == "Nt")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		if (_Driver_Load_Other.Init(temp_str.toStdString()))
		{
			if (_Driver_Load_Other.Nt_Register_Driver())
			{
				if (_Driver_Load_Other.Nt_Start_Driver())
				{
					QMessageBox::information(this, "success", "driver loaded");
					return;
				}
			}
		}
	}
	if (action->text() == "Normal")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		_Driver_Load_Other.Init(temp_str.toStdString());
		if (_Driver_Load_Other.Register_Driver())
		{
			if (_Driver_Load_Other.Start_Driver())
			{
				QMessageBox::information(this, "success", "driver loaded");
				return;
			}
		}
	}
	if (action->text() == "Unload")
	{
		if (_Driver_Load_Other._NtModule)
		{
			if (_Driver_Load_Other.Nt_Stop_Driver() && _Driver_Load_Other.Nt_UnRegister_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
			}
		}
		else
		{
			if (_Driver_Load_Other.Stop_Driver() && _Driver_Load_Other.UnRegister_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
			}
		}
	}
}

void Medusa::ProcessRightMenu(QAction* action)
{
	if (action->text() == "R3CreateRemoteThread+LoadLibraryA" ||
		action->text() == "R3APCInject")
	{
		RightMenuDLLInject(action);
		return;
	}

	if (action->text() == "R3QuickCheckALL" || 
		action->text() == "R3HookScanner" || 
		action->text() == "R3HookScannerSimple(Y/N)")
	{
		RightMenuHookScanner(action);
		return;
	}


	if (action->text() == "R3ModulesView")
	{
		RightMenuR3ModulesView(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}

	if (action->text() == "R3ThreadView")
	{
		RightMenuR3ThreadsView(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
}

void Medusa::ChangeTab()
{
	if (ui.tabWidget->currentIndex() == 0)
	{
		GetProcessList();
	}
	else if (ui.tabWidget->currentIndex() == 1)
	{
		GetKernelModuleList();
	}
}

void Medusa::RightMenuDLLInject(QAction* action)
{
	if (action->text() == "R3CreateRemoteThread+LoadLibraryA")
	{
		DLLInject _DLLInject;
		_DLLInject.R3CreateThread(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
	if (action->text() == "R3APCInject")
	{
		DLLInject _DLLInject;
		_DLLInject.R3APCInject(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
}

void Medusa::RightMenuHookScanner(QAction* action)
{
	if (action->text() == "R3QuickCheckALL")
	{
		FileCheck temp_check;
		std::string temp_str;
		ui.progressBar->setMaximum(_Process._Process_List_R3.size());
		ui.progressBar->setValue(0);
		for (auto x : _Process._Process_List_R3)
		{
			if (x.PID == 0 || x.PID == 4)
			{
				continue;
			}
			if (!temp_check.CheckSimple(x.PID))
			{
				temp_str = temp_str + QString::fromWCharArray(x.Name).toStdString() + "\r\n";
			}
			ui.progressBar->setValue(ui.progressBar->value() + 1);
		}
		temp_str = temp_str + "\r\n detected hook";
		QMessageBox::information(this, "Ret", temp_str.data());
	}
	if (action->text() == "R3HookScannerSimple(Y/N)")
	{
		if (_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 0
			|| _Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 4)
		{
			return;
		}
		FileCheck temp_check;
		if (temp_check.CheckSimple(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID))
		{
			QMessageBox::information(this, "Ret", "not detected hook");
		}
		else
		{
			QMessageBox::information(this, "Ret", "detected hook");
		}
	}
	if (action->text() == "R3HookScanner")
	{
		if (_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 0
			|| _Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 4)
		{
			return;
		}
		_HookScanner._Model->removeRows(0, _HookScanner._Model->rowCount());
		FileCheck temp_check;
		std::vector<_CheckDifferent> temp_vector = temp_check.CheckPlain(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
		int i = 0;
		for (auto x : temp_vector)
		{
			_HookScanner._Model->setVerticalHeaderItem(i, new QStandardItem);
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 0), i);
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 1), QString::fromWCharArray(x.Name));
			std::ostringstream ret;
			ret << std::hex << "0x" << x.Addr;
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 2), ret.str().data());
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 3), String_TO_HEX(std::string(x.FileHex, 20)).data());
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 4), String_TO_HEX(std::string(x.MemoryHex, 20)).data());
			_HookScanner._Model->setData(_HookScanner._Model->index(i, 5), QString::fromWCharArray(x.Path));
			if (x.Fail)
			{
				_HookScanner._Model->item(i, 0)->setBackground(QColor(Qt::red));
				_HookScanner._Model->item(i, 1)->setBackground(QColor(Qt::red));
				_HookScanner._Model->item(i, 2)->setBackground(QColor(Qt::red));
				_HookScanner._Model->item(i, 3)->setBackground(QColor(Qt::red));
				_HookScanner._Model->item(i, 4)->setBackground(QColor(Qt::red));
				_HookScanner._Model->item(i, 5)->setBackground(QColor(Qt::red));
			}
			i++;
		}
		_HookScanner.show();
	}
}

void Medusa::RightMenuR3ModulesView(ULONG64 PID)
{
	_Modules._Model->removeRows(0, _Modules._Model->rowCount());
	std::vector<MODULEENTRY32W> temp_vector =
		_Modules.GetWin32MoudleList(PID);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Modules._Model->setVerticalHeaderItem(i, new QStandardItem);
		_Modules._Model->setData(_Modules._Model->index(i, 0), i);
		_Modules._Model->setData(_Modules._Model->index(i, 1), QString::fromWCharArray(x.szModule));
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.modBaseAddr;
		_Modules._Model->setData(_Modules._Model->index(i, 2), ret.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << x.modBaseSize;
		_Modules._Model->setData(_Modules._Model->index(i, 3), ret2.str().data());
		std::wstring retStr;
		if (_Process.QueryValue(L"FileDescription", x.szExePath, retStr))
		{
			_Modules._Model->setData(_Modules._Model->index(i, 4), QString::fromWCharArray(retStr.data()));
		}
		i++;
	}
	_Modules.show();
}

void Medusa::RightMenuR0ModulesView(ULONG64 PID)
{
	
}

void Medusa::RightMenuR3ThreadsView(ULONG64 PID)
{
	_Threads._Model->removeRows(0, _Threads._Model->rowCount());
	std::vector<ThreadList> temp_vector = _Threads.GetThreadListR3(PID);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Threads._Model->setVerticalHeaderItem(i, new QStandardItem);
		_Threads._Model->setData(_Threads._Model->index(i, 0), i);
		_Threads._Model->setData(_Threads._Model->index(i, 1), x.TID);
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.StartAddr;
		_Threads._Model->setData(_Threads._Model->index(i, 3), ret.str().data());
		_Threads._Model->setData(_Threads._Model->index(i, 4), QString::fromWCharArray(x.Name));
		i++;
	}
	_Threads.show();
}

void Medusa::RightMenuR0ThreadsView(ULONG64 PID)
{

}

void Medusa::GetProcessList()
{
	_Model->removeRows(0, _Model->rowCount());
	_Process.GetProcessList(_Driver_Loaded);
	int i = 0;
	if (_Driver_Loaded)
	{
		for (auto x : _Process._Process_List_R0)
		{
			bool found = false;
			PROCESS_LIST temp_list;

			int j = 0;
			for (; j < _Process._Process_List_R3.size(); j++)
			{
				if (_Process._Process_List_R3.at(j).PID == x.PID)
				{
					found = true;
					break;
				}
			}
			if (found)
			{
				_Model->setVerticalHeaderItem(i, new QStandardItem);
				_Model->setData(_Model->index(i, 0), i);
				_Model->setData(_Model->index(i, 1), _Process._Process_List_R3.at(j).PID);
				_Model->setData(_Model->index(i, 2), QString::fromWCharArray(_Process._Process_List_R3.at(j).Name));
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.EPROCESS;
				_Model->setData(_Model->index(i, 3), ret.str().data());
				if (std::wstring(_Process._Process_List_R3.at(j).Path) != L"")
				{
					_Model->setData(_Model->index(i, 4), QString::fromWCharArray(_Process._Process_List_R3.at(j).Path));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", _Process._Process_List_R3.at(j).Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
					}
				}
				else
				{
					_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
					}
				}

			}
			else
			{
				_Model->setVerticalHeaderItem(i, new QStandardItem);
				_Model->setData(_Model->index(i, 0), i);
				_Model->setData(_Model->index(i, 1), x.PID);
				_Model->setData(_Model->index(i, 2), (char*)x.Name);
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.EPROCESS;
				_Model->setData(_Model->index(i, 3), ret.str().data());
				_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));


				if (x.Check == false)
				{
					_Model->item(i, 0)->setBackground(QColor(Qt::red));
					_Model->item(i, 1)->setBackground(QColor(Qt::red));
					_Model->item(i, 2)->setBackground(QColor(Qt::red));
					_Model->item(i, 3)->setBackground(QColor(Qt::red));
					_Model->item(i, 4)->setBackground(QColor(Qt::red));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
						_Model->item(i, 5)->setBackground(QColor(Qt::red));
					}
				}
				else
				{
					_Model->item(i, 0)->setBackground(QColor(Qt::green));
					_Model->item(i, 1)->setBackground(QColor(Qt::green));
					_Model->item(i, 2)->setBackground(QColor(Qt::green));
					_Model->item(i, 3)->setBackground(QColor(Qt::red));
					_Model->item(i, 4)->setBackground(QColor(Qt::green));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
						_Model->item(i, 5)->setBackground(QColor(Qt::green));
					}
				}
			}



			i++;
		}
		ui.label->setText(QString((std::string("R3 get process number:") +
			std::to_string(_Process._Process_List_R3.size()) +
			std::string("=====R0 get process number:") +
			std::to_string(_Process._Process_List_R0.size())).data()));
	}
	else
	{
		for (auto x : _Process._Process_List_R3)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), x.PID);
			_Model->setData(_Model->index(i, 2), QString::fromWCharArray(x.Name));
			_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));

			std::wstring retStr;
			if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
			{
				_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
			}
			i++;
		}

		ui.label->setText(QString((std::string("R3 get process number:") +
			std::to_string(_Process._Process_List_R3.size())).data()));
	}
}

void Medusa::GetKernelModuleList()
{
	_Model_Driver->removeRows(0, _Model_Driver->rowCount());
	if (_Driver_Loaded)
	{
	}
	else
	{
		KernelModules _KernelModules;
		_KernelModules.GetKernelModuleListR3();
		if (_KernelModules._KernelModuleListR3.size() != 0)
		{
			int i = 0;
			for (auto x: _KernelModules._KernelModuleListR3)
			{
				_Model_Driver->setVerticalHeaderItem(i, new QStandardItem);
				_Model_Driver->setData(_Model_Driver->index(i, 0), i);
				_Model_Driver->setData(_Model_Driver->index(i, 1), (char*)x.Name);
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.Addr;
				_Model_Driver->setData(_Model_Driver->index(i, 2), ret.str().data());
				std::ostringstream ret2;
				ret2 << std::hex << "0x" << (ULONG64)x.Size;
				_Model_Driver->setData(_Model_Driver->index(i, 3), ret2.str().data());
				_Model_Driver->setData(_Model_Driver->index(i, 4), (char*)x.Path);
				i++;
			}
			ui.label->setText(QString((std::string("R3 get kernel moduls number:") +
				std::to_string(_KernelModules._KernelModuleListR3.size())).data()));
			
		}
	}
}