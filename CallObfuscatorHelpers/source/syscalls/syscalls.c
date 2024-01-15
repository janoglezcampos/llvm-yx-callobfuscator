/**
 * @file syscalls.c
 * @author Alejandro González (@httpyxel)
 * @brief Utilities to work with windows x64 syscalls.
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

#include "syscalls/syscalls.h"
#include "common/commonUtils.h"
#include "common/debug.h"

BOOL __callobf_iterateSyscalls(PSYSCALL_ITER_CTX p_ctx, PCHAR *pp_name, PVOID *pp_functionAddr)
{
    PIMAGE_DOS_HEADER p_dosHeaders = (PIMAGE_DOS_HEADER)p_ctx->p_ntdll;
    PIMAGE_EXPORT_DIRECTORY p_expDir = p_ctx->p_expDir;

    if (!p_dosHeaders)
    {
        return FALSE;
    }

    if (!p_expDir)
    {

        PIMAGE_NT_HEADERS p_ntHeaders = (PVOID)((DWORD_PTR)p_dosHeaders + p_dosHeaders->e_lfanew);

        PIMAGE_DATA_DIRECTORY p_dataDir = (PVOID)(&p_ntHeaders->OptionalHeader.DataDirectory[0]);

        if (!p_dataDir->VirtualAddress)
        {
            DEBUG_PRINT("No data directory virtual address");
            return FALSE;
        }

        p_ctx->p_expDir = p_expDir = (PVOID)((DWORD_PTR)p_dosHeaders + p_dataDir->VirtualAddress);
    }

    if (!p_expDir)
    {
        DEBUG_PRINT("No export dir pointer");
        return FALSE;
    }

    PDWORD aof = (PVOID)((DWORD_PTR)p_dosHeaders + p_expDir->AddressOfFunctions);
    PDWORD aon = (PVOID)((DWORD_PTR)p_dosHeaders + p_expDir->AddressOfNames);
    PUSHORT ano = (PVOID)((DWORD_PTR)p_dosHeaders + p_expDir->AddressOfNameOrdinals);

    if (p_ctx->lastEntry >= p_expDir->NumberOfNames)
    {
        DEBUG_PRINT("Iterator already ended");
        return FALSE;
    }

    for (DWORD cnt = p_ctx->lastEntry; cnt < p_expDir->NumberOfNames; cnt++)
    {
        PCHAR p_name = (PVOID)((DWORD_PTR)p_dosHeaders + aon[cnt]);

        if (p_name[0] == 'Z' && p_name[1] == 'w')
        {
            // p_name[0] = 'N';
            // p_name[1] = 't';

            *pp_name = p_name;
            *pp_functionAddr = (PVOID)((DWORD_PTR)p_dosHeaders + aof[ano[cnt]]);
            p_ctx->lastEntry = ++cnt;
            return TRUE;
        }
    }
    return FALSE;
}

UINT32 __callobf_hashSyscallA(PCHAR p_str, BOOL asZw)
{
    UINT h;
    PCHAR p;
    CHAR c;

    h = 0;

    if (p_str[0] == 'Z' && p_str[1] == 'w')
    {
        if (!asZw)
        {
            h = HASH_MULTIPLIER * h + 'n';
            h = HASH_MULTIPLIER * h + 't';
            p_str += 2;
        }
    }
    else if (p_str[0] == 'N' && p_str[1] == 't')
    {
        if (asZw)
        {
            h = HASH_MULTIPLIER * h + 'z';
            h = HASH_MULTIPLIER * h + 'w';
            p_str += 2;
        }
    }

    for (p = p_str; *p != '\0'; p++)
    {
        c = (*p >= 65 && *p <= 90) ? *p + 32 : *p;
        h = HASH_MULTIPLIER * h + c;
    }
    return h;
}

BOOL __callobf_checkHashSyscallA(PCHAR p_functionName, DWORD32 hash)
{

    DWORD32 hashAsZw = __callobf_hashSyscallA(p_functionName, TRUE);
    DWORD32 hashAsNt = __callobf_hashSyscallA(p_functionName, FALSE);
    if (!hashAsZw || !hashAsZw)
        return FALSE;

    return (BOOL)(hash == hashAsZw || hash == hashAsNt);
}

PVOID __callobf_getSyscallAddr(PVOID p_function, PVOID p_lowBoundary, PVOID p_highBoundary)
{
    PBYTE lowerRunner = (PBYTE)p_function;
    PBYTE higherRunner = (PBYTE)p_function;
    USHORT tmpVal = 0x0;

    lowerRunner += 0x15;
    higherRunner += 0x15;

    // Is this overkill?
    while (lowerRunner >= (PBYTE)p_lowBoundary || higherRunner < (PBYTE)p_highBoundary - 0x3) // 3 because 0f05C3 aka syscallret
    {
        if (higherRunner < (PBYTE)p_highBoundary - 0x3)
        {
            tmpVal = ((*(PUSHORT)higherRunner) & 0x00FFFFFF) ^ 0xdeaddead;
            if (tmpVal == (USHORT)0xde6edba2)
            {
                return higherRunner;
            }
            higherRunner++;
        }

        if (lowerRunner >= (PBYTE)p_lowBoundary)
        {
            tmpVal = ((*(PUSHORT)lowerRunner) & 0x00FFFFFF) ^ 0xdeaddead;
            if (tmpVal == (USHORT)0xde6edba2)
            {
                return lowerRunner;
            }
            lowerRunner--;
        }
    }
    return NULL;
}

BOOL __callobf_loadSyscall(
    DWORD32 nameHash,
    PVOID p_ntdll,
    PUSHORT p_ssn,
    PVOID *pp_function)
{
    SYSCALL_ITER_CTX iterCtx = {0};
    USHORT ssn = 0;

    PCHAR p_stubNameTmp = NULL;
    PVOID p_functionTmp = NULL;

    PVOID p_function = NULL;

    BOOL found = FALSE;

    if (!p_ntdll)
        return FALSE;

    iterCtx.p_ntdll = p_ntdll;

    while (__callobf_iterateSyscalls(&iterCtx, &p_stubNameTmp, &p_functionTmp))
    {
        if (__callobf_checkHashSyscallA(p_stubNameTmp, nameHash))
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        return FALSE;
    }

    p_function = p_functionTmp;

    iterCtx.lastEntry = 0;

    while (__callobf_iterateSyscalls(&iterCtx, &p_stubNameTmp, &p_functionTmp))
    {
        if (p_functionTmp < p_function)
            ssn++;
    }

    PIMAGE_NT_HEADERS p_ntHeaders = (PIMAGE_NT_HEADERS)(p_ntdll + ((PIMAGE_DOS_HEADER)p_ntdll)->e_lfanew);
    PIMAGE_SECTION_HEADER p_sectionHeaders = IMAGE_FIRST_SECTION(p_ntHeaders);
    PIMAGE_SECTION_HEADER p_currentSection = NULL;
    PVOID p_sectionStart = NULL;
    PVOID p_sectionEnd = NULL;

    for (WORD sectionIndex = 0; sectionIndex < p_ntHeaders->FileHeader.NumberOfSections; sectionIndex++)
    {
        PIMAGE_SECTION_HEADER p_sectionHeader = &p_sectionHeaders[sectionIndex];
        p_sectionStart = p_ntdll + p_sectionHeader->VirtualAddress;
        p_sectionEnd = p_sectionStart + p_sectionHeader->SizeOfRawData;

        if (((DWORD_PTR)p_sectionStart <= (DWORD_PTR)p_function) && ((DWORD_PTR)p_function < (DWORD_PTR)p_sectionEnd))
        {

            p_currentSection = p_sectionHeader;
            break;
        }
    }

    if (!p_currentSection)
        return FALSE;

    if (!(p_functionTmp = __callobf_getSyscallAddr(p_function, p_sectionStart, p_sectionEnd)))
        return FALSE;

    *p_ssn = ssn;
    *pp_function = p_functionTmp;

    return TRUE;
}