#pragma once


using SYM_CHILD_ENTRY = struct {
	std::string Name;
	std::string TypeName;
	UINT64 ElementsCount;
	UINT64 Size;
	ULONG Offset;
	BOOL IsBitField;
	ULONG BitPosition;
};
using SYM_INFO = struct {
	std::string Name;
	UINT64 Size;
	ULONG Offset;
	std::vector<SYM_CHILD_ENTRY> Entries;
};

class SymParser 
{
private:
	std::string W_TO_C(std::wstring str)
	{
		std::string result;
		DWORD strsize = WideCharToMultiByte(CP_ACP, 0, str.data(), -1, NULL, 0, NULL, NULL);
		char* pstr = new char[strsize];
		WideCharToMultiByte(CP_ACP, 0, str.data(), -1, pstr, strsize, NULL, NULL);
		result = pstr;
		return result;
	}
    // From cvconst.h:
    enum BasicType {
        btNoType = 0,
        btVoid = 1,
        btChar = 2,
        btWChar = 3,
        btInt = 6,
        btUInt = 7,
        btFloat = 8,
        btBCD = 9,
        btBool = 10,
        btLong = 13,
        btULong = 14,
        btCurrency = 25,
        btDate = 26,
        btVariant = 27,
        btComplex = 28,
        btBit = 29,
        btBSTR = 30,
        btHresult = 31
    };

    std::string GetSymName(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    std::string GetSymTypeName(ULONG Index, OPTIONAL OUT PUINT64 BaseTypeSize = NULL, OPTIONAL OUT PBOOL Status = NULL);
    UINT64 GetSymSize(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    ULONG GetSymOffset(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    ULONG GetSymAddressOffset(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    ULONG GetSymBitPosition(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    ULONG GetSymTypeId(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    ULONG GetSymArrayTypeId(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    enum SymTagEnum GetSymTag(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    enum BasicType GetSymType(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
    enum BasicType GetSymBaseType(ULONG Index, OPTIONAL OUT PBOOL Status = NULL);
public:
    SymParser(HANDLE hProcess1, ULONG64 ModuleBase1)
    {
        hProcess = hProcess1;
        ModuleBase = ModuleBase1;
    }
	~SymParser() = default;
    HANDLE hProcess = 0;
    ULONG64 ModuleBase = 0;
    BOOL DumpSymbol(PCSTR SymbolName, OUT SYM_INFO& SymInfo);
};