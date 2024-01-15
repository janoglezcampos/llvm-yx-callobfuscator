/**
 * @file callDispatcher.c
 * @author Alejandro González (@httpyxel)
 * @brief Funcionality to invoke windows native functions applying obfuscation.
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

#include "callDispatcher/callDispatcher.h"
#include "common/commonUtils.h"
#include "pe/peUtils.h"
#include "syscalls/syscalls.h"
#include "common/debug.h"

// Not fully implemented, meant to return errors
unsigned long __callobf_getLastError()
{
    return __callobf_lastError;
}

void *__callobf_loadFunction(PFUNCTION_TABLE_ENTRY p_fEntry)
{
    PDLL_TABLE_ENTRY p_dllEntry = NULL;
    DWORD32 moduleHash = 0;

    p_dllEntry = &__callobf_dllTable.entries[p_fEntry->moduleIndex];
    moduleHash = __callobf_hashA(p_dllEntry->name);

    DEBUG_PRINT("Loading function: %lX", p_fEntry->hash);

    if (!p_dllEntry->handle)
    {
        DEBUG_PRINT("Loading module: %s", p_dllEntry->name);
        p_dllEntry->handle = __callobf_getModuleAddrH(moduleHash);
        if (!p_dllEntry->handle)
        {
            // TODO: Opsec this
            // Import everything by hash
            // Add eaf bypass
            // Make a placeholder for LoadLibraryA
            // Make a load error field for both dlls and functions
            // Encript dll names at compile time
            PVOID p_kernel32 = NULL;
            PLOADLIBRARYA p_LoadLibraryA = NULL;
            p_kernel32 = __callobf_getModuleAddrA("kernelbase.dll");
            p_LoadLibraryA = __callobf_getFunctionAddrA(p_kernel32, "LoadLibraryA");
            if (!p_LoadLibraryA)
            {
                DEBUG_PRINT("Error, couldnt get ptr to LoadLibraryA");
                return NULL;
            }
            p_dllEntry->handle = p_LoadLibraryA(p_dllEntry->name);
        }
    }

    if (!p_dllEntry->handle)
    {
        DEBUG_PRINT("Error, couldnt load dll");
        return NULL;
    }

    USHORT ssn = 0;
    PVOID p_function = NULL;

    if (moduleHash == __callobf_hashA("ntdll.dll"))
    {
        if (__callobf_loadSyscall(p_fEntry->hash, p_dllEntry->handle, &ssn, &p_function))
        {
            p_fEntry->ssn = (((DWORD32)ssn) | 0xFFFF0000);
            p_fEntry->functionPtr = p_function;

            DEBUG_PRINT("Loaded function 0x%08lX as syscall with ssn 0x%04X at %p", p_fEntry->hash, ssn, p_function);
            return p_fEntry->functionPtr;
        }
    };

    p_fEntry->ssn = 0;
    p_fEntry->functionPtr = __callobf_getFunctionAddrH(p_dllEntry->handle, p_fEntry->hash);

    if (!p_fEntry->functionPtr)
    {
        DEBUG_PRINT("Error, load function");
        return NULL;
    }

    return p_fEntry->functionPtr;
}

void *__callobf_callDispatcher(DWORD32 index, ...)
{
    PVOID p_returnAddress = __builtin_frame_address(0) - 0x8;
    PVOID p_function = NULL;
    USHORT ssn = 0;
    BOOL isSyscall = FALSE;
    DWORD32 argCount = 0;
    PFUNCTION_TABLE_ENTRY p_fEntry = NULL;

#ifdef __clang__
    __builtin_ms_va_list p_args;
#else
    __builtin_va_list p_args;
#endif

    __builtin_ms_va_start(p_args, index);

    if (index > __callobf_functionTable.count)
    {
        return NULL;
    }

    p_fEntry = &(__callobf_functionTable.entries[index]);

    if (!(p_function = p_fEntry->functionPtr))
    {
        if (!(p_function = __callobf_loadFunction(p_fEntry)))
        {
            return NULL;
        }
    }

    argCount = p_fEntry->argCount;

    if ((p_fEntry->ssn & 0xFFFF0000))
    {
        p_fEntry->ssn &= 0x0000FFFF;
        ssn = (USHORT)(p_fEntry->ssn);
        isSyscall = TRUE;
    }

    return __callobf_doCall(p_function, ssn, isSyscall, argCount, p_args, p_returnAddress, &__callobf_globalFrameTable);
}