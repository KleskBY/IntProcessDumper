#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <TlHelp32.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>

static std::string PageProtectionToString(DWORD pageProtection) 
{
	switch (pageProtection)
	{
	case PAGE_NOACCESS:
		return "NOACCESS ";
	case PAGE_READONLY:
		return "READONLY ";
	case PAGE_READWRITE:
		return "READWRITE";
	case PAGE_WRITECOPY:
		return "WRITECOPY";
	case PAGE_EXECUTE:
		return "EXECUTE  ";
	case PAGE_EXECUTE_READ:
		return "READ     ";
	case PAGE_EXECUTE_READWRITE:
		return "READWRITE";
	case PAGE_EXECUTE_WRITECOPY:
		return "EXECUTE_WRITECOPY";
	case PAGE_GUARD:
		return "GUARD    ";
	case PAGE_NOCACHE:
		return "NOCACHE  ";
	case PAGE_WRITECOMBINE:
		return "WRITECOMBINE";
	case PAGE_ENCLAVE_THREAD_CONTROL:
		return "ENCLAVE_THREAD_CONTROL";
	case PAGE_TARGETS_NO_UPDATE:
		return "TARGETS_NO_UPDATE";
	case PAGE_ENCLAVE_UNVALIDATED:
		return "ENCLAVE_UNVALIDATED";
	case PAGE_ENCLAVE_DECOMMIT:
		return "ENCLAVE_DECOMMIT";
	default:
		return "PROTECT_UNKNOWN";
	}
}

static std::string State2String(DWORD state) 
{
	if (state == MEM_COMMIT) return			"IMAGE  ";
	else if (state == MEM_FREE) return		"MAPPED ";
	else if (state == MEM_RESERVE) return	"PRIVATE";
	else return								"UNKNOWN";
}

static std::string Type2String(DWORD type) 
{
	if (type == MEM_IMAGE) return			"IMAGE  ";
	else if (type == MEM_MAPPED) return		"MAPPED ";
	else if (type == MEM_PRIVATE) return	"PRIVATE";
	else return								"UNKNOWN";
}


bool IsDumpableMemoryInformation(const MEMORY_BASIC_INFORMATION& mbi)
{
	if (mbi.Protect & PAGE_NOACCESS || mbi.Protect & PAGE_GUARD) return false;
	if (mbi.State & MEM_FREE) return false;
	if (!(mbi.Protect & PAGE_EXECUTE_READWRITE || mbi.Protect & PAGE_EXECUTE_READ ||
		mbi.Protect & PAGE_READONLY || mbi.Protect & PAGE_READWRITE))
		return false;

	return true;
};

std::string GetMemoryInformationFileName(const MEMORY_BASIC_INFORMATION& mbi)
{
	std::stringstream fileName;
	fileName << PageProtectionToString(mbi.Protect) << " " << Type2String(mbi.Type) <<" 0x" << mbi.BaseAddress << " " << "0x" << (void*)((size_t)mbi.BaseAddress + mbi.RegionSize) << ".bin";
	return fileName.str();
}


