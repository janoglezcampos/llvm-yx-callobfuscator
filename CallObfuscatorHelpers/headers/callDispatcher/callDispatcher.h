/**
 * @file callDispatcher.h
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

#ifndef _CALL_DISPATCHER_H_
#define _CALL_DISPATCHER_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"
#include "stackSpoof/stackSpoof.h"

// ==============================================================================
// ============================ STRUCT DEFINITIONS ==============================

#pragma pack(push, 1)
typedef struct _DLL_TABLE_ENTRY
{
    PCHAR name;
    PVOID handle;
} DLL_TABLE_ENTRY, *PDLL_TABLE_ENTRY;

typedef struct _FUNCTION_TABLE_ENTRY
{
    DWORD hash;
    DWORD moduleIndex;
    DWORD argCount;
    DWORD ssn; // As any other time that ssn is defined as a 4byte value, the
               // lower bytes are set to either 0xFF or 0x00, and it means if
               // it is actually a valid ssn. So we can check if the function
               // is a syscall by checking if any of thos bits are set to 1.
    PVOID functionPtr;
} FUNCTION_TABLE_ENTRY, *PFUNCTION_TABLE_ENTRY;

typedef struct _DLL_TABLE
{
    DWORD count;
    DWORD __padding;
    DLL_TABLE_ENTRY entries[];
} DLL_TABLE, *PDLL_TABLE;

typedef struct _FUNCTION_TABLE
{
    DWORD count;
    DWORD __padding;
    FUNCTION_TABLE_ENTRY entries[];
} FUNCTION_TABLE, *PFUNCTION_TABLE;
#pragma pack(pop)

// ==============================================================================
// =============================== GLOBALS ======================================

// ==============================================================================
// =========================== EXTERNAL GLOBALS =================================

extern DLL_TABLE __callobf_dllTable;
extern FUNCTION_TABLE __callobf_functionTable;

// ==============================================================================
// =========================== EXTERNAL FUNCTIONS ===============================

/**
 * @brief Given all the information about a function call, generates an obfuscated
 *        stack, and "applies indirect syscalling" if possible to the call.
 *
 *        Note: Returning errors is still something that is not resolved.
 *
 * @param p_function Pointer to function to be called, in case of syscall, this is
 *                   the syscall instruction pointer.
 * @param ssn SSN of the syscall, in case it is.
 * @param isSyscall If it is a syscall.
 * @param argCount Number of arguments taken by the function to be called.
 * @param p_args Pointer to arguments to be passed to the function.
 * @param p_returnAddress Address containing the return address of the entry point
 *                        for the current thread.
 * @param p_globalFrameTable Pointer to stack spoof info to build the spoofed
 *                           stack with.
 * @return void* Return value of the called function.
 */
extern void *__callobf_doCall(
    PVOID p_function,
    WORD ssn,
    DWORD isSyscall,
    DWORD argCount,
    PVOID p_args,
    PVOID p_returnAddress,
    PSTACK_SPOOF_INFO p_globalFrameTable);

// ==============================================================================
// ============================ PUBLIC  FUNCTIONS ===============================

/**
 * @brief Main function of the pass. This function can replace tha call to any
 *        native windows x64 function, by giving it an index to the function
 *        table containing the info about the function being replaced. All the
 *        arguments of the function being replaced, must be passed after the index.
 *
 * @param index Index to the function table.
 * @param ... Function call arguments.
 * @return void* Return value of the replaced function.
 */
void *__callobf_callDispatcher(DWORD32 index, ...);

// ==============================================================================
// =========================== PRIVATE  FUNCTIONS ===============================

/**
 * @brief Given a function entry, loads all it non initialized fields.
 *
 * @param p_fEntry Pointer to a function table entry.
 * @return void* Pointer to function, or NULL.
 */
void *__callobf_loadFunction(PFUNCTION_TABLE_ENTRY p_fEntry);

#endif