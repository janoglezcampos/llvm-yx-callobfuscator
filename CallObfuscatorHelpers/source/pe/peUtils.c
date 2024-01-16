/**
 * @file peUtils.c
 * @author Alejandro González (@httpyxel)
 * @brief Utilities to manipulate and work with in-memory PEs.
 * @version 0.1
 * @date 2024-01-14
 *
 * @copyright
 *   Copyright (C) 2024  Alejandro González
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pe/peUtils.h"
#include "common/commonUtils.h"

PVOID __callobf_getModuleAddrA(const PCHAR p_moduleName)
{
    return __callobf_getModuleAddrH(__callobf_hashA(p_moduleName));
}

PVOID __callobf_getModuleAddrW(const PWCHAR p_moduleName)
{
    return __callobf_getModuleAddrH(__callobf_hashW(p_moduleName));
}

PVOID __callobf_getModuleAddrH(const UINT32 moduleHash)
{
    PPEB peb;
    PPEB_LDR_DATA ldr;
    PLDR_DATA_TABLE_ENTRY dte;
    PLIST_ENTRY ent;
    PLIST_ENTRY hdr;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    peb = NtCurrentTeb()->ProcessEnvironmentBlock;
#pragma GCC diagnostic pop

    ldr = peb->Ldr;
    hdr = &ldr->InLoadOrderModuleList;
    ent = hdr->Flink;

    for (; hdr != ent; ent = ent->Flink)
    {
        dte = (PLDR_DATA_TABLE_ENTRY)ent;
        if (moduleHash == __callobf_hashU(&dte->BaseDllName))
        {
            return (PVOID)dte->DllBase;
        }
    }
    return NULL;
}

PVOID __callobf_getFunctionAddrA(const PVOID p_module, const PCHAR p_functionName)
{
    return __callobf_getFunctionAddrH(p_module, __callobf_hashA(p_functionName));
}

PVOID __callobf_getFunctionAddrW(const PVOID p_module, const PWCHAR p_functionName)
{
    return __callobf_getFunctionAddrH(p_module, __callobf_hashW(p_functionName));
}

PVOID __callobf_getFunctionAddrH(const PVOID p_module, const UINT32 funtionHash)
{
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nth;
    PIMAGE_DATA_DIRECTORY dir;
    PIMAGE_EXPORT_DIRECTORY exp;
    PDWORD aof;
    PDWORD aon;
    PUSHORT ano;
    PCHAR str;
    DWORD cnt;

    if (p_module == NULL)
        return NULL;

    if (*(PSHORT)p_module != PE_MAGIC)
        return NULL;

    dos = p_module;
    nth = (PVOID)((DWORD_PTR)dos + dos->e_lfanew);
    dir = (PVOID)(&nth->OptionalHeader.DataDirectory[0]);

    if (dir->VirtualAddress)
    {
        exp = (PVOID)((DWORD_PTR)dos + dir->VirtualAddress);
        aof = (PVOID)((DWORD_PTR)dos + exp->AddressOfFunctions);
        aon = (PVOID)((DWORD_PTR)dos + exp->AddressOfNames);
        ano = (PVOID)((DWORD_PTR)dos + exp->AddressOfNameOrdinals);

        for (cnt = 0; cnt < exp->NumberOfNames; cnt++)
        {
            str = (PVOID)((DWORD_PTR)dos + aon[cnt]);
            if (funtionHash == __callobf_hashA(str))
            {
                return (PVOID)((DWORD_PTR)dos + aof[ano[cnt]]);
            };
        };
    };
    return NULL;
};

PVOID __callobf_getExceptionDirectoryAddress(const PVOID p_module, PDWORD p_size)
{
    if (p_module == NULL)
        return NULL;

    if (*(PSHORT)p_module != PE_MAGIC)
        return NULL;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)p_module;
    PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)p_module + dosHeader->e_lfanew);

    DWORD exceptionDirectoryRVA = ntHeader->OptionalHeader.DataDirectory[3].VirtualAddress;
    if (exceptionDirectoryRVA == 0)
        return NULL;

    *p_size = ntHeader->OptionalHeader.DataDirectory[3].Size;
    return (PVOID)((DWORD_PTR)p_module + exceptionDirectoryRVA);
}

BOOL __callobf_getCodeBoundaries(PVOID p_module, PVOID *pp_base, PVOID *pp_top)
{
    PIMAGE_SECTION_HEADER codeHeader = NULL;

    if (!p_module || !pp_base || !pp_top)
        return FALSE;

    // Get boundaries for gadget search
    PIMAGE_DOS_HEADER dos = p_module;
    PIMAGE_NT_HEADERS nth = (PVOID)((DWORD_PTR)dos + dos->e_lfanew);
    PIMAGE_SECTION_HEADER sectionHeaders = IMAGE_FIRST_SECTION(nth);

    for (WORD i = 0; i < nth->FileHeader.NumberOfSections; i++)
    {
        if (sectionHeaders[i].Characteristics & IMAGE_SCN_CNT_CODE)
        {
            codeHeader = &sectionHeaders[i];
            break;
        }
    }

    if (!codeHeader)
        return FALSE;

    if (!codeHeader->VirtualAddress || !codeHeader->SizeOfRawData)
        return FALSE;

    *pp_base = (PVOID)((DWORD_PTR)dos + codeHeader->VirtualAddress);
    *pp_top = (PVOID)((DWORD_PTR)*pp_base + codeHeader->SizeOfRawData);
    return TRUE;
}
