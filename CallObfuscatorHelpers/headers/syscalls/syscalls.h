/**
 * @file syscalls.h
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

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "common/common.h"

// ==============================================================================
// ============================ STRUCT DEFINITIONS ==============================

typedef struct _SYSCALL_ITER_CTX
{
    PVOID p_ntdll;
    PIMAGE_EXPORT_DIRECTORY p_expDir;
    DWORD64 lastEntry;
} SYSCALL_ITER_CTX, *PSYSCALL_ITER_CTX;

// ==============================================================================
// ============================ PUBLIC  FUNCTIONS ===============================

/**
 * @brief Given the address of ntdll, initializes a syscall iterator context.
 *
 * @param p_ctx Pointer to the iterator context.
 * @param p_ntdll Pointer to ntdll.
 * @return BOOL Success.
 */
BOOL __callobf_initSyscallIter(
    PSYSCALL_ITER_CTX p_ctx,
    PVOID p_ntdll);

/**
 * @brief Given a syscall iterator context, gets the next syscalls address and stub
 *        name, and modifies the ctx so the next call to this function returns the
 *        following stub. The function names are always the Zw version of the function.
 *        If there is no more stubs remaining or an error occurs, return FALSE.
 *
 * @param p_ctx Pointer to the iterator context.
 * @param pp_name Double pointer to return the function name.
 * @param pp_functionAddr Double pointer to return function address.
 * @return BOOL TRUE if next stub found, FALSE in any other case.
 */
BOOL __callobf_getNextSyscall(
    PSYSCALL_ITER_CTX p_ctx,
    PCHAR *pp_name,
    PVOID *pp_functionAddr);

/**
 * @brief Given a function name hash and a pointer to ntdll, checks if the function
 *        is a syscall, if it is, returns its ssn and syscall pointer.
 *
 * @param nameHash 32 bit hash of the function name.
 * @param p_ntdll Pointer to ntdll.
 * @param p_ssn Pointer to return SSN if found.
 * @param pp_function Pointer to return the syscall address if found.
 * @return BOOL TRUE if is a syscall and both SSN and syscall address was found.
 */
BOOL __callobf_loadSyscall(
    DWORD32 nameHash,
    PVOID p_ntdll,
    PUSHORT p_ssn,
    PVOID *pp_function);

/**
 * @brief Given a pointer, searches for the closest syscall ret instruction pair
 *        between p_lowBoundary and p_highBoundary.
 *
 * @param p_function Pointer to start search from.
 * @param p_lowBoundary Lowest possible address to search.
 * @param p_highBoundary Highest possible address to search.
 * @return PVOID Pointer to closest syscall ret, or NULL if not found.
 */
PVOID __callobf_getSyscallAddr(
    PVOID p_function,
    PVOID p_lowBoundary,
    PVOID p_highBoundary);

/**
 * @brief Given a function names, calculates its 32 bit representation using
 *        the first two chars as "Zw"
 *
 * @param p_str Pointer to function name.
 * @return UINT32 32 bit representation of function name.
 */
UINT32 __callobf_hashSyscallAsZw(PCHAR p_str);

/**
 * @brief Given a function names, calculates its 32 bit representation using
 *        the first two chars as "Nt"
 *
 * @param p_str Pointer to function name.
 * @return UINT32 32 bit representation of function name.
 */
UINT32 __callobf_hashSyscallAsNt(PCHAR p_str);

/**
 * @brief Check if the given function name matches the given hash in any of its
 *        Zw or Nt forms. So ZwClose would match with the hash of NtClose.
 *
 * @param p_functionName Pointer to function name to check.
 * @param hash Hash of the function we are comparing to.
 * @return BOOL TRUE if the hash matches in any of its Zw or Nt form
 */
BOOL __callobf_checkHashSyscallA(
    PCHAR p_functionName,
    DWORD32 hash);

#endif