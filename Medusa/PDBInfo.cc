#include "PDBInfo.h"
#include <Shlwapi.h>


bool PDBInfo::DownLoadNtos()
{
	std::string symbolpath = std::getenv("_NT_SYMBOL_PATH");
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
	std::string ntos_pdb_path = symbolpath + "\\" + GetPdbPath(std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe");
	//ntos_pdb_path = Replace_ALL(ntos_pdb_path, "/", "\\");
	if (PathFileExistsA(ntos_pdb_path.c_str()))
	{
		if (EzPdbLoad(ntos_pdb_path, &_Pdb))
		{
			return true;
		}
	}
	else
	{
		if (symbolpath.length() > 0)
		{
			std::string temp_path = EzPdbDownload2(std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe", symbolpath);
			if (EzPdbLoad(temp_path, &_Pdb))
			{
				return true;
			}
		}
		else
		{
			std::string temp_path = EzPdbDownload(std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe");
			if (EzPdbLoad(temp_path, &_Pdb))
			{
				return true;
			}
		}
	}
	return false;
}