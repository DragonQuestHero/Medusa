﻿#include "PDBInfo.h"
#include <Shlwapi.h>

#include "KernelModules.h"
#include "Modules.h"

#include "SymParser.h"


bool PDBInfo::DownLoadNtos()
{
	if (_Pdb.hPdbFile)
	{
		EzPdbUnload(&_Pdb);
	}
	_BaseAddr = 0;
	_Symbol.clear();
	std::string pe_file_path = std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe";

	KernelModules _KernelModules;
	_KernelModules.GetKernelModuleListR3();
	for (auto x : _KernelModules._KernelModuleList)
	{
		if (x.Name == "ntoskrnl.exe")
		{
			_BaseAddr = x.Addr;
			break;
		}
	}
	
	std::string symbolpath;
	if (std::getenv("_NT_SYMBOL_PATH"))
	{
		symbolpath = std::getenv("_NT_SYMBOL_PATH");
		if (symbolpath.find("SRV") != std::string::npos)
		{
			std::vector<std::string> temp_vector = Split(symbolpath, "*");
			for (auto x : temp_vector)
			{
				if (PathFileExistsA(x.c_str()))
				{
					symbolpath = x;
					break;
				}
			}
		}
	}
	std::string ntos_pdb_path = symbolpath + "\\" + GetPdbPath(pe_file_path);
	//ntos_pdb_path = Replace_ALL(ntos_pdb_path, "/", "\\");
	if (PathFileExistsA(ntos_pdb_path.c_str()))
	{
		return EzPdbLoad(ntos_pdb_path, &_Pdb);
	}
	else
	{
		if (symbolpath.length() > 0)
		{
			GetSetPdbDir(pe_file_path, symbolpath);
			std::string temp_path = EzPdbDownload2(pe_file_path, symbolpath,_SymbolServer);
			return EzPdbLoad(temp_path, &_Pdb);
		}
		else
		{
			char szDownloadDir[MAX_PATH] = { 0 };
			GetCurrentDirectoryA(sizeof(szDownloadDir), szDownloadDir);
			std::string pdb_path = std::string(szDownloadDir) + "\\" + GetPdbPath(pe_file_path);
			if (!PathFileExistsA(pdb_path.c_str()))
			{
				GetSetPdbDir(pe_file_path,"");
				std::string temp_path = EzPdbDownload(pe_file_path, "", _SymbolServer);
				return EzPdbLoad(temp_path, &_Pdb);
			}
			else
			{
				return EzPdbLoad(pdb_path, &_Pdb);
			}
		}
	}
	return false;
}

bool PDBInfo::DownLoad(std::string path, bool use_bassaddr)
{
	if (_Pdb.hPdbFile)
	{
		EzPdbUnload(&_Pdb);
	}
	_BaseAddr = 0;
	_Symbol.clear();

	if (use_bassaddr)
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		_splitpath(path.data(), drive, dir, fname, ext);
		//if (std::string(ext) == ".sys")
		{
			KernelModules _KernelModules;
			_KernelModules.GetKernelModuleListR3();
			for (auto x : _KernelModules._KernelModuleListR3)
			{
				if (Case_Upper(x.Name) == Case_Upper(std::string(fname) + ext))
				{
					_BaseAddr = x.Addr;
					break;
				}
			}
		}


		if (!_BaseAddr)
		{
			Modules _Modules;
			std::vector<MODULEENTRY32W> temp_vector = _Modules.GetUserMoudleListR3(GetCurrentProcessId());
			bool found = false;
			for (auto x : temp_vector)
			{
				if (path == W_TO_C(x.szExePath))
				{
					_BaseAddr = (ULONG64)x.modBaseAddr;
					found = true;
					break;
				}
			}
			if (!found)
			{
				_BaseAddr = (ULONG64)LoadLibraryA(path.data());
			}
		}
	}

	

	std::string symbolpath;
	if (std::getenv("_NT_SYMBOL_PATH"))
	{
		symbolpath = std::getenv("_NT_SYMBOL_PATH");
		if (symbolpath.find("SRV") != std::string::npos)
		{
			std::vector<std::string> temp_vector = Split(symbolpath, "*");
			for (auto x : temp_vector)
			{
				if (PathFileExistsA(x.c_str()))
				{
					symbolpath = x;
					break;
				}
			}
		}
	}
	std::string pdb_path = symbolpath + "\\" + GetPdbPath(path);
	if (PathFileExistsA(pdb_path.c_str()))
	{
		return EzPdbLoad(pdb_path, &_Pdb);
	}
	else
	{
		if (symbolpath.length() > 0)
		{
			GetSetPdbDir(path, symbolpath);
			std::string temp_path = EzPdbDownload2(path, symbolpath, _SymbolServer);
			return EzPdbLoad(temp_path, &_Pdb);
		}
		else
		{
			char szDownloadDir[MAX_PATH] = { 0 };
			GetCurrentDirectoryA(sizeof(szDownloadDir), szDownloadDir);
			pdb_path = std::string(szDownloadDir) + "\\" + GetPdbPath(path);
			if (!PathFileExistsA(pdb_path.c_str()))
			{
				GetSetPdbDir(path, "");
				std::string temp_path = EzPdbDownload(path, "", _SymbolServer);
				return EzPdbLoad(temp_path, &_Pdb);
			}
			else
			{
				return EzPdbLoad(pdb_path, &_Pdb);
			}
		}
	}
	return false;
}

bool PDBInfo::QueryDownLoad(std::string path)
{
	std::string symbolpath;
	if (std::getenv("_NT_SYMBOL_PATH"))
	{
		symbolpath = std::getenv("_NT_SYMBOL_PATH");
		if (symbolpath.find("SRV") != std::string::npos)
		{
			std::vector<std::string> temp_vector = Split(symbolpath, "*");
			for (auto x : temp_vector)
			{
				if (PathFileExistsA(x.c_str()))
				{
					symbolpath = x;
					break;
				}
			}
		}
	}
	std::string pdb_path = symbolpath + "\\" + GetPdbPath(path);
	if (PathFileExistsA(pdb_path.c_str()))
	{
		return true;
	}
	char szDownloadDir[MAX_PATH] = { 0 };
	GetCurrentDirectoryA(sizeof(szDownloadDir), szDownloadDir);
	pdb_path = std::string(szDownloadDir) + "\\" + GetPdbPath(path);
	if (PathFileExistsA(pdb_path.c_str()))
	{
		return true;
	}
	return false;
}

struct MyStruct
{
	ULONG64 _BaseAddr;
	std::vector<NTOSSYMBOL>* temp_vector;
};

std::string GBK_To_UTF8(char* p)
{
	DWORD strsize = MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
	wchar_t* pwstr = new wchar_t[strsize];
	MultiByteToWideChar(CP_ACP, 0, p, -1, pwstr, strsize);
	strsize = WideCharToMultiByte(CP_UTF8, 0, pwstr, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_UTF8, 0, pwstr, -1, p, strsize, NULL, NULL);
	std::string str = p;
	delete[] pwstr;
	pwstr = NULL;
	return str;
}
BOOL PsymEnumeratesymbolsCallback(
	PSYMBOL_INFO pSymInfo,
	ULONG SymbolSize,
	PVOID UserContext
)
{
	MyStruct* _MyStruct = (MyStruct*)UserContext;
	NTOSSYMBOL temp_list;
	RtlZeroMemory(&temp_list, sizeof(NTOSSYMBOL));
	temp_list.Addr = pSymInfo->Address - pSymInfo->ModBase + _MyStruct->_BaseAddr;
	temp_list.Name = GBK_To_UTF8(pSymInfo->Name);//默认中文环境
	temp_list.RVA = pSymInfo->Address - pSymInfo->ModBase;
	temp_list.Size = pSymInfo->Size;
	_MyStruct->temp_vector->push_back(temp_list);
	return TRUE;
}

bool PDBInfo::GetALL()
{
	_Symbol.clear();
	MyStruct _MyStruct;
	_MyStruct._BaseAddr = _BaseAddr;
	_MyStruct.temp_vector = &_Symbol;
	if (!SymEnumSymbols(GetCurrentProcess(), EZ_PDB_BASE_OF_DLL, "*!*", &PsymEnumeratesymbolsCallback, &_MyStruct))
	{
		return false;
	}
	return SymEnumTypesByName(GetCurrentProcess(), EZ_PDB_BASE_OF_DLL, "*!*", &PsymEnumeratesymbolsCallback, &_MyStruct);
}

std::vector<SYMBOLSTRUCT> PDBInfo::PdbGetStruct(IN PEZPDB Pdb, IN std::string StructName)
{
	std::vector<SYMBOLSTRUCT> temp_vector;

	SymParser _SymParser(Pdb->hProcess, EZ_PDB_BASE_OF_DLL);
	SYM_INFO SymInfo;
	_SymParser.DumpSymbol(StructName.data(), SymInfo);
	SYMBOLSTRUCT temp_list;
	temp_list.Name = SymInfo.Name;
	temp_list.Size = SymInfo.Size;
	temp_list.Offset = SymInfo.Offset;
	temp_vector.push_back(temp_list);
	for (auto x : SymInfo.Entries)
	{
		SYMBOLSTRUCT temp_list;
		temp_list.Name = x.Name;
		temp_list.Size = x.Size;
		temp_list.Offset = x.Offset;
		if (x.ElementsCount > 1)
		{
			temp_list.Type = "[" + std::to_string(x.ElementsCount) + "]" + x.TypeName;
		}
		else
		{
			temp_list.Type = x.TypeName;
		}
		if (x.IsBitField)
		{
			temp_list.Type = x.TypeName + " Pos:" + std::to_string(x.BitPosition);
		}
		temp_vector.push_back(temp_list);
	}
	return temp_vector;
}
//std::vector<SYMBOLSTRUCT> PDBInfo::PdbGetStruct(IN PEZPDB Pdb, IN std::string StructName)
//{
//	std::vector<SYMBOLSTRUCT> temp_vector;
//
//	ULONG SymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
//	SYMBOL_INFO* SymInfo = (SYMBOL_INFO*)malloc(SymInfoSize);
//	if (!SymInfo)
//	{
//		return  temp_vector;
//	}
//	ZeroMemory(SymInfo, SymInfoSize);
//	SymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
//	SymInfo->MaxNameLen = MAX_SYM_NAME;
//	if (!SymGetTypeFromName(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, StructName.c_str(), SymInfo))
//	{
//		return  temp_vector;
//	}
//
//	TI_FINDCHILDREN_PARAMS TempFp = { 0 };
//	if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_GET_CHILDRENCOUNT, &TempFp))
//	{
//		free(SymInfo);
//		return  temp_vector;
//	}
//
//	ULONG ChildParamsSize = sizeof(TI_FINDCHILDREN_PARAMS) + TempFp.Count * sizeof(ULONG);
//	TI_FINDCHILDREN_PARAMS* ChildParams = (TI_FINDCHILDREN_PARAMS*)malloc(ChildParamsSize);
//	if (ChildParams == NULL)
//	{
//		free(SymInfo);
//		return  temp_vector;
//	}
//	ZeroMemory(ChildParams, ChildParamsSize);
//	memcpy(ChildParams, &TempFp, sizeof(TI_FINDCHILDREN_PARAMS));
//	if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_FINDCHILDREN, ChildParams))
//	{
//		goto failed;
//	}
//	for (ULONG i = ChildParams->Start; i < ChildParams->Count; i++)
//	{
//		WCHAR* pSymName = NULL;
//		WCHAR* typeName = NULL;
//		std::string type_name = "";
//		DWORD typeId = 0;
//		ULONG Offset = 0;
//		DWORD symTag = 0;
//		DWORD length = 0;
//		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_OFFSET, &Offset))
//		{
//			goto failed;
//		}
//		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_SYMNAME, &pSymName))
//		{
//			goto failed;
//		}
//		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_TYPE, &typeId))
//		{
//			goto failed;
//		}
//		if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, typeId, TI_GET_SYMNAME, &typeName))
//		{
//			//goto failed;
//		}
//		else if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, typeId, TI_GET_TYPEID, &symTag))
//		{
//			BOOL SymStatus = SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, typeId, TI_GET_SYMTAG, &symTag);
//			if (symTag == SymTagPointerType) 
//			{
//				type_name = type_name + "Ptr64 ";
//				DWORD pointedTypeId;
//				if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, typeId, TI_GET_TYPEID, &pointedTypeId)) {
//					DWORD pointedSymTag;
//					if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, pointedTypeId, TI_GET_SYMTAG, &pointedSymTag)) {
//						// 处理指向的类型
//						if (pointedSymTag == SymTagBaseType) 
//						{
//							DWORD baseType;
//							if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, pointedTypeId, TI_GET_BASETYPE, &baseType)) {
//								type_name = type_name + GetTypeName(baseType);
//							}
//						}
//					}
//				}
//				if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, typeId, TI_GET_LENGTH, &length))
//				{
//					type_name = type_name + "(" + std::to_string(length) + ")";
//					//printf("Failed to get length information for type ID: %u\n", typeId);
//				}
//			}
//			else if (symTag == SymTagArrayType) 
//			{
//				type_name = GetArrayType(Pdb, symTag, typeId);
//			}
//			else if (symTag == SymTagBaseType)
//			{
//				DWORD baseType;
//				if (SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, typeId, TI_GET_BASETYPE, &baseType))
//				{
//					type_name = GetTypeName(baseType);
//				}
//			}
//		}
//		if (pSymName)
//		{
//			SYMBOLSTRUCT temp_list;
//			temp_list.Name = W_TO_C(pSymName);
//			if (typeName)
//			{
//				temp_list.Type = W_TO_C(typeName);
//				LocalFree(typeName);
//			}
//			else if (type_name != "")
//			{
//				temp_list.Type = type_name;
//			}
//			temp_list.Offset = Offset;
//			temp_list.Size = length;
//			LocalFree(pSymName);
//			temp_vector.push_back(temp_list);
//		}
//	}
//failed:
//	free(ChildParams);
//	free(SymInfo);
//	return  temp_vector;
//}


void PDBInfo::UnLoad()
{
	EzPdbUnload(&_Pdb);
}


#define TEST_GetPDBInfo CTL_CODE(FILE_DEVICE_UNKNOWN,0x7109,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool PDBInfo::SendMedusaPDBInfo()
{
	_BaseAddr = 0;
	_Symbol.clear();
	UnLoad();

	if (!DownLoadNtos())
	{
		return false;
	}
	GetALL();
	for (auto x : _Symbol)
	{
		if (x.Name=="MiProcessLoaderEntry")
		{
			_MedusaPDBInfo.MiProcessLoaderEntry = x.Addr;
		}
		if (x.Name=="PiDDBLock")
		{
			_MedusaPDBInfo.PiDDBLock = x.Addr;
		}
		if (x.Name=="PiDDBCacheTable")
		{
			_MedusaPDBInfo.PiDDBCacheTable = x.Addr;
		}
		if (x.Name=="MmUnloadedDrivers")
		{
			_MedusaPDBInfo.MmUnloadedDrivers = x.Addr;
		}
		if (x.Name=="MmLastUnloadedDriver")
		{
			_MedusaPDBInfo.MmLastUnloadedDriver = x.Addr;
		}
		if (x.Name == "RtlpLookupFunctionEntryForStackWalks")
		{
			_MedusaPDBInfo.RtlpLookupFunctionEntryForStackWalks = x.Addr;
		}
	}

	_BaseAddr = 0;
	_Symbol.clear();
	UnLoad();
	if (!DownLoad(std::getenv("systemroot") + std::string("\\System32\\CI.dll"), true))
	{
		return false;
	}
	GetALL();
	for (auto x : _Symbol)
	{
		if (x.Name == "g_KernelHashBucketList")
		{
			_MedusaPDBInfo.KernelHashBucketList = x.Addr;
		}
		if (x.Name=="g_HashCacheLock")
		{
			_MedusaPDBInfo.HashCacheLock = x.Addr;
		}
		if (x.Name=="g_CiEaCacheLookasideList")
		{
			_MedusaPDBInfo.CiEaCacheLookasideList = x.Addr;
		}
	}
	_BaseAddr = 0;
	_Symbol.clear();
	UnLoad();


	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	ULONG dwRet = 0;
	if (DeviceIoControl(m_hDevice, TEST_GetPDBInfo, &_MedusaPDBInfo, sizeof(MedusaPDBInfo), 0, 0, &dwRet, NULL))
	{
		CloseHandle(m_hDevice);
		return true;
	}
	CloseHandle(m_hDevice);
	return false;
}