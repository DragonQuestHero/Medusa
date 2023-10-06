#include "Medusa.h"

#include "FileCheck.h"
#include "Hypervisor.h"
#include "DLLInject.h"





Medusa::Medusa(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	Enable_Debug();

	ProcessUI();
	DriverUI();
	UnloadDriverUI();
	ProcessRightMenuUI();
	DriverRightMenuUI();
	
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);



	Set_SLOTS();



	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != m_hDevice)
	{
		_Driver_Loaded = true;
	}
	CloseHandle(m_hDevice);
	ChangeTab();
}


void Medusa::Set_SLOTS()
{
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(ChangeTab()));//进程
	//connect(ui.tableView_Driver, SIGNAL(currentChanged(int)), SLOT(ChangeTab()));//进程

	connect(&_TableView_Menu_Inject, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_HookCheck, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Modules, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Threads, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单


	connect(&_TableView_Menu_DriverClear, SIGNAL(triggered(QAction*)), SLOT(DriverRightMenu(QAction*)));
	connect(&_TableView_Action_DriverDump, SIGNAL(triggered(bool)), SLOT(DriverRightMenu(bool)));

	connect(ui.menuMenu, SIGNAL(triggered(QAction*)), SLOT(DriverLoadMenu(QAction*)));
	connect(ui.menuHypervisor, SIGNAL(triggered(QAction*)), SLOT(HypervisorMenu(QAction*)));
	connect(ui.menuPDB, SIGNAL(triggered(QAction*)), SLOT(PdbMenu(QAction*)));
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

void Medusa::PdbMenu(QAction* action)
{
	if (action->text() == "Down&Load ntos")
	{
		_PDBView._PDBInfo.UnLoad();
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoadNtos())
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		return;
	}
	if (action->text() == "Down&Load file")
	{
		_PDBView._PDBInfo.UnLoad();
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoad(temp_str.toStdString(),false))
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		return;
	}
	if (action->text() == "Down&Load file with baseaddr")
	{
		_PDBView._PDBInfo.UnLoad();
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		if (!_PDBView._PDBInfo.DownLoad(temp_str.toStdString(), true))
		{
			QMessageBox::information(this, "error", "error download pdb");
		}
		return;
	}
	if (action->text() == "Unload Symbol")
	{
		_PDBView._PDBInfo.UnLoad();
		return;
	}
	if (action->text() == "PdbView")
	{
		_PDBView._Model->removeRows(0, _PDBView._Model->rowCount());
		_PDBView.show();
		return;
	}
	if (action->text().toStdString().find("Use microsoft server") != std::string::npos)
	{
		action->setText(u8"Use microsoft server √");
		ui.actionUse_order_server->setText(u8"Use Order Server ×");
		_PDBView._PDBInfo._SymbolServer = "https://msdl.microsoft.com/download/symbols/";
	}
	if (action->text().toStdString().find("Use Order Server") != std::string::npos)
	{
		action->setText(u8"Use Order Server √");
		ui.actionUse_microsoft_server->setText(u8"Use microsoft server ×");
		_PDBView._PDBInfo._SymbolServer = "https://msdl.szdyg.cn/download/symbols/";
	}
	if (action->text() == "SendPDBInfo")
	{
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (_PDBView._PDBInfo.SendMedusaPDBInfo())
		{
			ui.progressBar->setValue(100);
			ui.label->setText("success load pdb");
			QMessageBox::information(this, "success", "pdb send");
		}
		else
		{
			ui.label->setText("error load pdb");
			QMessageBox::information(this, "error", "error");
		}
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
		action->text() == "R3APCInject" ||
		action->text() == "R3MapInject")
	{
		RightMenuDLLInject(action);
		return;
	}

	if (action->text() == "QuickCheckALLProcess" || 
		action->text() == "HookScanner" || 
		action->text() == "HookScannerSimple(Y/N)")
	{
		RightMenuHookScanner(action);
		return;
	}


	if (action->text() == "R3ModulesView")
	{
		RightMenuR3ModulesView(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
	if (action->text() == "R0ModulesView(second check)")
	{
		RightMenuR0ModulesView(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
	if (action->text() == "R3ModuleScanner")
	{
		RightMenuR3ModuleScanner(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}

	if (action->text() == "R3ThreadView")
	{
		RightMenuR3ThreadsView(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
	if (action->text() == "R0ThreadView(second check)")
	{
		RightMenuR0ThreadsView(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
}

void Medusa::DriverRightMenu(QAction* action)
{
}

void Medusa::DriverRightMenu(bool)
{
	std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 2).data().toString().toStdString();
	addr_str.erase(0, 2);
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);
	std::string size_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 3).data().toString().toStdString();
	size_str.erase(0, 2);
	ULONG64 size = strtoull(size_str.data(), 0, 16);
	char* temp_buffer = new char[size];
	if (_KernelModules.DumpDriver(addr, size, temp_buffer))
	{
		std::fstream temp_file(addr_str, std::ios::out | std::ios::binary);
		if (temp_file.is_open())
		{
			temp_file << std::string(temp_buffer, size);
			temp_file.close();
			QMessageBox::information(this, "Ret", "susscss");
		}
	}
	else
	{
		QMessageBox::information(this, "Ret", "error");
	}
	delete temp_buffer;
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
	else if (ui.tabWidget->currentIndex() == 2)
	{
		GetUnLoadKernelModuleList();
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
	if (action->text() == "R3MapInject")
	{
		DLLInject _DLLInject;
		_DLLInject.R3MapInject(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
	}
}

void Medusa::RightMenuHookScanner(QAction* action)
{
	if (action->text() == "QuickCheckALL")
	{
		FileCheck temp_check(_Driver_Loaded);
		std::string temp_str;
		ui.progressBar->setMaximum(_Process._Process_List_R3.size());
		ui.progressBar->setValue(0);
		for (auto x : _Process._Process_List_R3)
		{
			if (x.PID == 0 || x.PID == 4)
			{
				continue;
			}
			int temp_ret = temp_check.CheckSimple(x.PID);
			if (temp_ret == -1)
			{
				temp_str = temp_str + QString::fromWCharArray(x.Name).toStdString() + "-----Insufficient permissions or file miss\r\n";
			}
			if (temp_ret == 0)
			{
				temp_str = temp_str + QString::fromWCharArray(x.Name).toStdString() + "-----detected hook\r\n";
			}
			ui.progressBar->setValue(ui.progressBar->value() + 1);
		}
		ui.progressBar->setValue(_Process._Process_List_R3.size());
		QMessageBox::information(this, "Ret", temp_str.data());
	}
	if (action->text() == "HookScannerSimple(Y/N)")
	{
		if (_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 0
			|| _Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 4)
		{
			return;
		}
		FileCheck temp_check(_Driver_Loaded);
		int temp_ret = temp_check.CheckSimple(_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID);
		if (temp_ret == 1)
		{
			QMessageBox::information(this, "Ret", "not detected hook");
		}
		else if (temp_ret == 0)
		{
			QMessageBox::information(this, "Ret", "detected hook");
		}
		else
		{
			QMessageBox::information(this, "Ret", "Insufficient permissions or file miss");
		}
	}
	if (action->text() == "HookScanner")
	{
		if (_Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 0
			|| _Process._Process_List_R3.at(ui.tableView->currentIndex().row()).PID == 4)
		{
			return;
		}
		_HookScanner._Model->removeRows(0, _HookScanner._Model->rowCount());
		FileCheck temp_check(_Driver_Loaded);
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
		_Modules.GetUserMoudleListR3(PID);
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
		_Modules._Model->setData(_Modules._Model->index(i, 4), QString::fromWCharArray(x.szExePath));
		std::wstring retStr;
		if (_Process.QueryValue(L"FileDescription", x.szExePath, retStr))
		{
			_Modules._Model->setData(_Modules._Model->index(i, 5), QString::fromWCharArray(retStr.data()));
		}
		i++;
	}
	_Modules.show();
}

void Medusa::RightMenuR0ModulesView(ULONG64 PID)
{
	if (_Driver_Loaded)
	{
		_Modules._Model->removeRows(0, _Modules._Model->rowCount());
		std::vector<MODULEENTRY32W> temp_vector = _Modules.GetUserMoudleListR3(PID);
		std::vector<UserModule> temp_vector2 = _Modules.GetUserMoudleListR0(PID);
		int i = 0;
		for (auto x : temp_vector2)
		{
			_Modules._Model->setVerticalHeaderItem(i, new QStandardItem);
			_Modules._Model->setData(_Modules._Model->index(i, 0), i);
			_Modules._Model->setData(_Modules._Model->index(i, 1), QString::fromWCharArray(x.Name));
			std::ostringstream ret;
			ret << std::hex << "0x" << (ULONG64)x.Addr;
			_Modules._Model->setData(_Modules._Model->index(i, 2), ret.str().data());
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << x.Size;
			_Modules._Model->setData(_Modules._Model->index(i, 3), ret2.str().data());
			_Modules._Model->setData(_Modules._Model->index(i, 4), QString::fromWCharArray(x.Path));
			std::wstring retStr;
			if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
			{
				_Modules._Model->setData(_Modules._Model->index(i, 5), QString::fromWCharArray(retStr.data()));
			}
			else
			{
				_Modules._Model->setData(_Modules._Model->index(i, 5), "");
			}
			bool found = false;
			for (auto y: temp_vector)
			{
				if (x.Addr == (ULONG64)y.modBaseAddr)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				_Modules._Model->item(i, 0)->setBackground(QColor(Qt::red));
				_Modules._Model->item(i, 1)->setBackground(QColor(Qt::red));
				_Modules._Model->item(i, 2)->setBackground(QColor(Qt::red));
				_Modules._Model->item(i, 3)->setBackground(QColor(Qt::red));
				_Modules._Model->item(i, 4)->setBackground(QColor(Qt::red));
				_Modules._Model->item(i, 5)->setBackground(QColor(Qt::red));
			}
			i++;
		}
		_Modules.show();
	}
}

void Medusa::RightMenuR3ModuleScanner(ULONG64 PID)
{
	auto proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (!proc)
	{
		QMessageBox::information(this, "Ret", "permissions error");
		return;
	}
	_Modules._Model->removeRows(0, _Modules._Model->rowCount());
	std::vector<UserModule> temp_vector = _Modules.R3ModuleScanner(PID, proc);
	CloseHandle(proc);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Modules._Model->setVerticalHeaderItem(i, new QStandardItem);
		_Modules._Model->setData(_Modules._Model->index(i, 0), i);
		std::ostringstream ret;
		ret << std::hex << (ULONG64)x.Addr;
		_Modules._Model->setData(_Modules._Model->index(i, 2), ret.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << x.Size;
		_Modules._Model->setData(_Modules._Model->index(i, 3), ret2.str().data());
		if (std::wstring(x.Name) == L"shellcode")
		{
			_Modules._Model->setData(_Modules._Model->index(i, 1), u8"RWXMemory");
			_Modules._Model->item(i, 0)->setBackground(QColor(Qt::green));
		}
		else
		{
			_Modules._Model->setData(_Modules._Model->index(i, 1), u8"MemoryDLL");
			_Modules._Model->item(i, 0)->setBackground(QColor(Qt::red));
		}
		i++;
	}
	_Modules.show();
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
	if (_Driver_Loaded)
	{
		_Threads._Model->removeRows(0, _Threads._Model->rowCount());
		std::vector<ThreadList> temp_vector = _Threads.GetThreadListR0(PID);
		std::vector<ThreadList> temp_vector2 = _Threads.GetThreadListR3(PID);
		int i = 0;
		for (auto x : temp_vector)
		{
			bool found = false;
			for (auto y : temp_vector2)
			{
				if (x.TID == y.TID)
				{
					found = true;
					break;
				}
			}
			_Threads._Model->setVerticalHeaderItem(i, new QStandardItem);
			_Threads._Model->setData(_Threads._Model->index(i, 0), i);
			_Threads._Model->setData(_Threads._Model->index(i, 1), x.TID);
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << (ULONG64)x.ETHREAD;
			_Threads._Model->setData(_Threads._Model->index(i, 2), ret2.str().data());
			std::ostringstream ret;
			ret << std::hex << "0x" << (ULONG64)x.StartAddr;
			_Threads._Model->setData(_Threads._Model->index(i, 3), ret.str().data());
			_Threads._Model->setData(_Threads._Model->index(i, 4), QString::fromWCharArray(x.Name));
			if (!found)
			{
				_Threads._Model->item(i, 0)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 1)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 2)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 3)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 4)->setBackground(QColor(Qt::red));
			}
			i++;
		}
		_Threads.show();
	}
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
		_KernelModules.GetKernelModuleListR0();
		if (_KernelModules._KernelModuleListR0.size() != 0)
		{
			int i = 0;
			for (auto x : _KernelModules._KernelModuleListR0)
			{
				_Model_Driver->setVerticalHeaderItem(i, new QStandardItem);
				_Model_Driver->setData(_Model_Driver->index(i, 0), i);
				if (x.Check == 1 || x.Check == 2)
				{
					_Model_Driver->setData(_Model_Driver->index(i, 1), QString::fromWCharArray((WCHAR*)x.Name));
				}
				else
				{
					_Model_Driver->setData(_Model_Driver->index(i, 1), (char*)x.Name);
				}
				
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.Addr;
				_Model_Driver->setData(_Model_Driver->index(i, 2), ret.str().data());
				std::ostringstream ret2;
				ret2 << std::hex << "0x" << (ULONG64)x.Size;
				_Model_Driver->setData(_Model_Driver->index(i, 3), ret2.str().data());
				if (x.Check == 1 || x.Check == 2)
				{
					std::wstring temp_wstr = (WCHAR*)x.Path;
					temp_wstr = ReplaceStr(temp_wstr, L"\\SystemRoot\\", L"C:\\Windows\\");
					temp_wstr = ReplaceStr(temp_wstr, L"\\??\\", L"");
					_Model_Driver->setData(_Model_Driver->index(i, 4), QString::fromWCharArray(temp_wstr.data()));
				}
				else
				{
					std::string temp_str = x.Path;
					temp_str = ReplaceStr2(temp_str, "\\SystemRoot\\", "C:\\Windows\\");
					temp_str = ReplaceStr2(temp_str, "\\??\\", "");
					_Model_Driver->setData(_Model_Driver->index(i, 4), temp_str.data());
				}
				if (x.Check == 1 || x.Check == 2)
				{
					std::wstring retStr;
					std::wstring temp_wstr = (WCHAR*)x.Path;
					temp_wstr = ReplaceStr(temp_wstr, L"\\SystemRoot\\", L"C:\\Windows\\");
					temp_wstr = ReplaceStr(temp_wstr, L"\\??\\", L"");
					if (_Process.QueryValue(L"FileDescription", temp_wstr.data(), retStr))
					{
						_Model_Driver->setData(_Model_Driver->index(i, 5), QString::fromWCharArray(retStr.data()));
					}
					else
					{
						_Model_Driver->setData(_Model_Driver->index(i, 5), "");
					}
				}
				else
				{
					std::wstring retStr;
					std::wstring temp_wstr = C_TO_W(x.Path);
					temp_wstr = ReplaceStr(temp_wstr, L"\\SystemRoot\\", L"C:\\Windows\\");
					temp_wstr = ReplaceStr(temp_wstr, L"\\??\\", L"");
					if (_Process.QueryValue(L"FileDescription", temp_wstr.data(), retStr))
					{
						_Model_Driver->setData(_Model_Driver->index(i, 5), QString::fromWCharArray(retStr.data()));
					}
					else
					{
						_Model_Driver->setData(_Model_Driver->index(i, 5), "");
					}
				}
				QColor temp_color = QColor(Qt::white);
				if (x.Check == 1)
				{
					temp_color = QColor(Qt::green);
				}
				if (x.Check == 2)
				{
					temp_color = QColor(Qt::red);
				}
				if (x.Check == 3)
				{
					temp_color = QColor(Qt::red);
				}
				_Model_Driver->item(i, 0)->setBackground(temp_color);
				_Model_Driver->item(i, 1)->setBackground(temp_color);
				_Model_Driver->item(i, 2)->setBackground(temp_color);
				_Model_Driver->item(i, 3)->setBackground(temp_color);
				_Model_Driver->item(i, 4)->setBackground(temp_color);
				_Model_Driver->item(i, 5)->setBackground(temp_color);
				i++;
			}
			ui.label->setText(QString((std::string("R0 get kernel moduls number:") +
				std::to_string(_KernelModules._KernelModuleListR0.size())).data()));

		}
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

				std::string temp_str = x.Path;
				temp_str = ReplaceStr2(temp_str, "\\SystemRoot\\", "C:\\Windows\\");
				temp_str = ReplaceStr2(temp_str, "\\??\\", "");
				_Model_Driver->setData(_Model_Driver->index(i, 4), temp_str.data());

				std::wstring temp_wstr = C_TO_W(x.Path);
				if (temp_wstr.find(L"SystemRoot") != std::wstring::npos)
				{
					temp_wstr = ReplaceStr(temp_wstr, L"\\SystemRoot\\", L"C:\\Windows\\");
					temp_wstr = ReplaceStr(temp_wstr, L"\\??\\", L"");
				}
				std::wstring retStr;
				if (_Process.QueryValue(L"FileDescription", temp_wstr.data(), retStr))
				{
					_Model_Driver->setData(_Model_Driver->index(i, 5), QString::fromWCharArray(retStr.data()));
				}
				i++;
			}
			ui.label->setText(QString((std::string("R3 get kernel moduls number:") +
				std::to_string(_KernelModules._KernelModuleListR3.size())).data()));
		}
	}
}

void Medusa::GetUnLoadKernelModuleList()
{
	_Model_UnloadDriver->removeRows(0, _Model_UnloadDriver->rowCount());
	KernelModules _KernelModules;
	_KernelModules.GetUnLoadKernelModuleListR0();
	if (!_KernelModules._KernelUnLoadModuleListR0.size())
	{
		return;
	}
	int i = 0;
	for (auto x : _KernelModules._KernelUnLoadModuleListR0)
	{
		_Model_UnloadDriver->setVerticalHeaderItem(i, new QStandardItem);
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 0), i);
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 1), QString::fromWCharArray((WCHAR*)x.Name));

		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.Addr;
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 2), ret.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << (ULONG64)x.Size;
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 3), ret2.str().data());
		std::ostringstream ret3;
		ret3 << std::hex << "0x" << (ULONG64)x.UnLoadTime;
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 4), ret3.str().data());


		QColor temp_color = QColor(Qt::white);
		if (x.Check == 1)
		{
			temp_color = QColor(Qt::red);
		}
		if (x.Check == 2)
		{
			temp_color = QColor(Qt::red);
		}
		_Model_UnloadDriver->item(i, 0)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 1)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 2)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 3)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 4)->setBackground(temp_color);

		i++;
	}
}