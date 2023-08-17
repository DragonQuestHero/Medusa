#include "EzPdb.h"



// download pdb file from symbol server
// return pdb path if success, 
// or return empty string if failed, user can call GetLastError() to know wth is going on
std::string EzPdbDownload(
	IN std::string pePath, 
	IN OPTIONAL std::string pdbDownloadPath,
	IN OPTIONAL std::string symbolServer)
{
	// pdb download directory
	// if not specify, then pdb will download to current directory
	if (pdbDownloadPath == "")
	{
		char szDownloadDir[MAX_PATH] = { 0 };
		if (!GetCurrentDirectoryA(sizeof(szDownloadDir), szDownloadDir))
		{
			return "";
		}
		pdbDownloadPath = szDownloadDir;
	}

	if (pdbDownloadPath[pdbDownloadPath.size() - 1] != '\\')
	{
		pdbDownloadPath += "\\";
	}

	// make sure the directory exist
	if (!CreateDirectoryA(pdbDownloadPath.c_str(), NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return "";
		}
	}

	// read pe file

#ifndef _AMD64_
	PVOID OldValue = NULL;
	Wow64DisableWow64FsRedirection(&OldValue);
#endif

	std::ifstream file(pePath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> buffer(size);

#ifndef _AMD64_
	Wow64RevertWow64FsRedirection(&OldValue);
#endif

	if (!file.read(buffer.data(), size) || size==0)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		return "";
	}

	std::string pdbPath = pdbDownloadPath + Md5(buffer.data(), (ULONG)size) + ".pdb";

	// get pdb info from debug info directory
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)buffer.data();
	IMAGE_NT_HEADERS* pNT = (IMAGE_NT_HEADERS*)(buffer.data() + pDos->e_lfanew);
	IMAGE_FILE_HEADER* pFile = &pNT->FileHeader;
	IMAGE_OPTIONAL_HEADER64* pOpt64 = NULL;
	IMAGE_OPTIONAL_HEADER32* pOpt32 = NULL;
	BOOL x86 = FALSE;
	if (pFile->Machine == IMAGE_FILE_MACHINE_AMD64)
	{
		pOpt64 = (IMAGE_OPTIONAL_HEADER64*)(&pNT->OptionalHeader);
	}
	else if (pFile->Machine == IMAGE_FILE_MACHINE_I386)
	{
		pOpt32 = (IMAGE_OPTIONAL_HEADER32*)(&pNT->OptionalHeader);
		x86 = TRUE;
	}
	else
	{
		SetLastError( ERROR_NOT_SUPPORTED);
		return "";
	}
	DWORD ImageSize = x86 ? pOpt32->SizeOfImage : pOpt64->SizeOfImage;

	// file buffer to image buffer
	PBYTE ImageBuffer = (PBYTE)malloc(ImageSize);
	if (!ImageBuffer)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return "";
	}
	memcpy(ImageBuffer, buffer.data(), x86 ? pOpt32->SizeOfHeaders : pOpt64->SizeOfHeaders);
	IMAGE_SECTION_HEADER* pCurrentSectionHeader = IMAGE_FIRST_SECTION(pNT);
	for (UINT i = 0; i != pFile->NumberOfSections; ++i, ++pCurrentSectionHeader)
	{
		if (pCurrentSectionHeader->SizeOfRawData)
		{
			memcpy(ImageBuffer + pCurrentSectionHeader->VirtualAddress, buffer.data() + pCurrentSectionHeader->PointerToRawData, pCurrentSectionHeader->SizeOfRawData);
		}
	}
	IMAGE_DATA_DIRECTORY* pDataDir = nullptr;
	if (x86)
	{
		pDataDir = &pOpt32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	}
	else
	{
		pDataDir = &pOpt64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	}
	IMAGE_DEBUG_DIRECTORY* pDebugDir = (IMAGE_DEBUG_DIRECTORY*)(ImageBuffer + pDataDir->VirtualAddress);
	if (!pDataDir->Size || IMAGE_DEBUG_TYPE_CODEVIEW != pDebugDir->Type)
	{
		// invalid debug dir
		free(ImageBuffer);
		SetLastError( ERROR_NOT_SUPPORTED);
		return "";
	}
	PdbInfo* pdb_info = (PdbInfo*)(ImageBuffer + pDebugDir->AddressOfRawData);
	if (pdb_info->Signature != 0x53445352)
	{
		// invalid debug dir
		free(ImageBuffer);
		SetLastError(ERROR_NOT_SUPPORTED);
		return "";
	}

	

	// download pdb
	DeleteFileA(pdbPath.c_str());
	wchar_t w_GUID[100] = { 0 };
	if (!StringFromGUID2(pdb_info->Guid, w_GUID, 100))
	{
		free(ImageBuffer);
		SetLastError(ERROR_NOT_SUPPORTED);
		return "";
	}
	char a_GUID[100]{ 0 };
	size_t l_GUID = 0;
	if (wcstombs_s(&l_GUID, a_GUID, w_GUID, sizeof(a_GUID)) || !l_GUID)
	{
		free(ImageBuffer);
		SetLastError(ERROR_NOT_SUPPORTED);
		return "";
	}

	char guid_filtered[256] = { 0 };
	for (UINT i = 0; i != l_GUID; ++i)
	{
		if ((a_GUID[i] >= '0' && a_GUID[i] <= '9') || (a_GUID[i] >= 'A' && a_GUID[i] <= 'F') || (a_GUID[i] >= 'a' && a_GUID[i] <= 'f'))
		{
			guid_filtered[strlen(guid_filtered)] = a_GUID[i];
		}
	}

	char age[3] = { 0 };
	_itoa_s(pdb_info->Age, age, 10);

	// url
	std::string url = symbolServer;
	url += pdb_info->PdbFileName;
	url += "/";
	url += guid_filtered;
	url += age;
	url += "/";
	url += pdb_info->PdbFileName;

	// download
	HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), pdbPath.c_str(), NULL, NULL);
	if (FAILED(hr))
	{
		free(ImageBuffer);
		return "";
	}

	free(ImageBuffer);
	return pdbPath;
}


// load pdb
// return true if success, or return false if failed, user can call GetLastError() to know wth is going on
bool EzPdbLoad(IN std::string pdbPath, OUT PEZPDB Pdb)
{
	memset(Pdb, 0, sizeof(EZPDB));

	// get pdb file size
	WIN32_FILE_ATTRIBUTE_DATA file_attr_data{ 0 };
	if (!GetFileAttributesExA(pdbPath.c_str(), GetFileExInfoStandard, &file_attr_data))
	{
		return false;
	}
	auto pdbSize = file_attr_data.nFileSizeLow;

	// open pdb file
	HANDLE hPdbFile = CreateFileA(pdbPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hPdbFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// open current process
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId());
	if (!hProcess)
	{
		CloseHandle(hPdbFile);
		return false;
	}

	// Initializes the symbol handler for a process
	if (!SymInitialize(hProcess, pdbPath.c_str(), FALSE))
	{
		CloseHandle(hProcess);
		CloseHandle(hPdbFile);
		return false;
	}

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_AUTO_PUBLICS | SYMOPT_DEBUG | SYMOPT_LOAD_ANYTHING);
	//SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_AUTO_PUBLICS | SYMOPT_LOAD_ANYTHING);

	DWORD64 SymbolTable = SymLoadModuleEx(hProcess, NULL, pdbPath.c_str(), NULL, EZ_PDB_BASE_OF_DLL, pdbSize, NULL, NULL);
	if (!SymbolTable)
	{
		SymCleanup(hProcess);
		CloseHandle(hProcess);
		CloseHandle(hPdbFile);
		return false;
	}

	Pdb->hPdbFile = hPdbFile;
	Pdb->hProcess = hProcess;
	return true;
}

// get function / global variable rva
// return -1 if failed
ULONG EzPdbGetRva(IN PEZPDB Pdb, IN std::string SymName)
{
	SYMBOL_INFO si = { 0 };
	si.SizeOfStruct = sizeof(SYMBOL_INFO);
	if (!SymFromName(Pdb->hProcess, SymName.c_str(), &si))
	{
		return (ULONG)-1;
	}
	return (ULONG)(si.Address - si.ModBase);
}

// get struct property offset
// return -1 if failed
ULONG EzPdbGetStructPropertyOffset(IN PEZPDB Pdb, IN std::string StructName, IN std::wstring PropertyName)
{
	ULONG SymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
	SYMBOL_INFO* SymInfo = (SYMBOL_INFO*)malloc(SymInfoSize);
	if (!SymInfo)
	{
		return  (ULONG)-1;
	}
	ZeroMemory(SymInfo, SymInfoSize);
	SymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	SymInfo->MaxNameLen = MAX_SYM_NAME;
	if (!SymGetTypeFromName(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, StructName.c_str(), SymInfo))
	{
		return  (ULONG)-1;
	}

	TI_FINDCHILDREN_PARAMS TempFp = { 0 };
	if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_GET_CHILDRENCOUNT, &TempFp))
	{
		free(SymInfo);
		return  (ULONG)-1;
	}

	ULONG ChildParamsSize = sizeof(TI_FINDCHILDREN_PARAMS) + TempFp.Count * sizeof(ULONG);
	TI_FINDCHILDREN_PARAMS* ChildParams = (TI_FINDCHILDREN_PARAMS*)malloc(ChildParamsSize);
	if (ChildParams == NULL)
	{
		free(SymInfo);
		return (ULONG)-1;
	}
	ZeroMemory(ChildParams, ChildParamsSize);
	memcpy(ChildParams, &TempFp, sizeof(TI_FINDCHILDREN_PARAMS));
	if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, SymInfo->TypeIndex, TI_FINDCHILDREN, ChildParams))
	{
		goto failed;
	}
	for (ULONG i = ChildParams->Start; i < ChildParams->Count; i++)
	{
		WCHAR* pSymName = NULL;
		ULONG Offset = 0;
		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_OFFSET, &Offset))
		{
			goto failed;
		}
		if (!SymGetTypeInfo(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, ChildParams->ChildId[i], TI_GET_SYMNAME, &pSymName))
		{
			goto failed;
		}
		if (pSymName)
		{
			//wprintf(L"%x %s\n", Offset, pSymName);
			if (wcscmp(pSymName, PropertyName.c_str()) == 0)
			{
				LocalFree(pSymName);
				free(ChildParams);
				free(SymInfo);
				return Offset;
			}
		}
	}
failed:
	free(ChildParams);
	free(SymInfo);
	return  (ULONG)-1;
}

// get struct size, failed return -1
ULONG EzPdbGetStructSize(IN PEZPDB Pdb, IN std::string StructName)
{
	ULONG SymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR);
	SYMBOL_INFO* SymInfo = (SYMBOL_INFO*)malloc(SymInfoSize);
	if (!SymInfo)
	{
		return (ULONG)-1;
	}
	ZeroMemory(SymInfo, SymInfoSize);
	SymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	SymInfo->MaxNameLen = MAX_SYM_NAME;
	if (!SymGetTypeFromName(Pdb->hProcess, EZ_PDB_BASE_OF_DLL, StructName.c_str(), SymInfo))
	{
		return  (ULONG)-1;
	}
	return SymInfo->Size;
}

// close handle
VOID EzPdbUnload(IN PEZPDB Pdb)
{
	// 清理工作
	SymUnloadModule64(Pdb->hProcess, EZ_PDB_BASE_OF_DLL);
	SymCleanup(Pdb->hProcess);
	CloseHandle(Pdb->hProcess);
	CloseHandle(Pdb->hPdbFile);
}



