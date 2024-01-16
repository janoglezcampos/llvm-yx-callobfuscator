/**
 * @file unwindUtils.h
 * @author Alejandro González (@httpyxel)
 * @brief Utilities to manipulate and work with PE unwind information.
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

#ifndef _UNWIND_UTILS_H_
#define _UNWIND_UTILS_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"
#include "common/debug.h"

// ==============================================================================
// ============================ STRUCT DEFINITIONS ==============================

typedef struct _UWOP_ITERATOR_CONTEXT
{
    HMODULE p_moduleBase;
    PUNWIND_INFO p_unwindInfo;
    DWORD nextIndex;
    BOOL ended;
} UWOP_ITERATOR_CONTEXT, *PUWOP_ITERATOR_CONTEXT;

typedef struct _UNWIND_INFO_ITERATOR_CONTEXT
{
    HMODULE p_moduleBase;
    DWORD nextIndex;
    BOOL ended;
    PIMAGE_RUNTIME_FUNCTION_ENTRY p_runtimeFunctionTable;
    DWORD entryCount;
} UNWIND_INFO_ITERATOR_CONTEXT, *PUNWIND_INFO_ITERATOR_CONTEXT;

// ==============================================================================
// ============================ PUBLIC  FUNCTIONS ===============================

/**
 * @brief Initializes or resets the given iterator context.
 *
 * @param p_ctx Pointer to iterator context.
 * @param p_moduleBase Pointer to module from where unwind info will be read.
 * @return BOOL Success.
 */
BOOL __callobf_createOrResetUnwindInfoIterator(
    _Out_ PUNWIND_INFO_ITERATOR_CONTEXT p_ctx,
    _In_ HMODULE p_moduleBase);

/**
 * @brief Given an unwind info iterator context, gets the next unwind info
 *        and modifies the ctx so the next call to this function returns the
 *        following unwind info. If there is no more unwind entries remaining
 *        or an error occurs, returns NULL.
 *
 * @param p_ctx Pointer to iterator context.
 * @return PUNWIND_INFO Pointer to next unwind info, or NULL.
 */
PUNWIND_INFO __callobf_getNextUnwindInfo(
    PUNWIND_INFO_ITERATOR_CONTEXT p_ctx);

/**
 * @brief Initializes or resets the given iterator context.
 *
 * @param p_ctx Pointer to iterator context.
 * @param p_moduleBase Pointer to module from where unwind operations will be read.
 * @param p_unwindInfoAddress Pointer to unwind info from where unwind operations
 *                            will be read.
 * @return BOOL Success.
 */
BOOL __callobf_createOrResetUwopIterator(
    _Out_ PUWOP_ITERATOR_CONTEXT p_ctx,
    _In_ HMODULE p_moduleBase,
    _In_ PUNWIND_INFO p_unwindInfoAddress);

/**
 * @brief Given an unwind operation iterator context, gets the next uwop and
 *        modifies the ctx so the next call to this function returns the following
 *        uwops. If there is no more uwop entries remaining or an error occurs,
 *        returns NULL.
 *
 * @param p_ctx Pointer to iterator context.
 * @return PUNWIND_CODE Pointer to next uwop, or NULL.
 */
PUNWIND_CODE __callobf_getNextUwop(
    PUWOP_ITERATOR_CONTEXT p_ctx);

/**
 * @brief Return the BeginAddress and EndAddress of the last unwind info read with
 *        the given context. If the context was never used to read a value, this
 *        returns FALSE.
 *
 * @param p_ctx Pointer to an unwind info iterator context.
 * @param pp_beginAddress Returns BeginAddress of the unwind info of the last entry
 *                        read from p_ctx.
 * @param pp_endAddress Returns EndAddress of the unwind info of the last entry read
 *                      from p_ctx.
 * @return BOOL Success.
 */
BOOL __callobf_getCodeBoundariesLastUnwindInfo(
    PUNWIND_INFO_ITERATOR_CONTEXT p_ctx,
    PVOID *pp_beginAddress,
    PVOID *pp_endAddress);

/**
 * @brief Calculates frame size modification applied by the given uwop.
 *
 *        Note: the difference between frame size and stack size,
 *              is that stack size represents the actual modifications
 *              to rsp, while  frame size applies to the frame, but is
 *              not backed by rsp
 *
 * @param p_unwindInfo Unwind info containing the given uwop.
 * @param p_unwindCode Unwind code to get the frame modification from.
 * @return LONG Frame size modification, or 0 on bad arguments.
 */
LONG __callobf_getFrameSizeModification(
    PUNWIND_INFO p_unwindInfo,
    PUNWIND_CODE p_unwindCode);

/**
 * @brief Calculates stack size modification applied by the given uwop.
 *
 *        Note: the difference between frame size and stack size,
 *              is that stack size represents the actual modifications
 *              to rsp, while  frame size applies to the frame, but is
 *              not backed by rsp
 *
 * @param p_unwindCode Unwind code to get the stack modification from.
 * @return LONG Frame size modification, or 0 on bad arguments.
 */
LONG __callobf_getStackSizeModification(
    PUNWIND_CODE p_unwindCode);

/**
 * @brief Find the address containing the return address for the entry
 *        point of thread executing this code.
 *
 * @param p_ntdll Pointer to ntdll.
 * @param p_kernel32 Pointer to kernel32.
 * @return PVOID Pointer containing return addr of current thread, or NULL.
 */
PVOID __callobf_findEntryAddressOfReturnAddress(
    PVOID p_ntdll,
    PVOID p_kernel32);

/**
 * @brief Given a pointer to a code section in a known module, finds the unwind info
 *        for said address. Optionally return the begin address and end address for
 *        the found unwind info.
 *
 * @param p_module Pointer to module containing the code pointer.
 * @param p_code Pointer to code.
 * @param pp_functionStart [optional] Pointer code block start.
 * @param p_functionEnd [optional] Pointer code block end.
 * @return PUNWIND_INFO Pointer to unwind info containing the code pointer, or NULL.
 */
PUNWIND_INFO __callobf_getUnwindInfoForCodePtr(
    PVOID p_module,
    PVOID p_code,
    PVOID *pp_functionStart,
    PVOID *pp_functionEnd);

/**
 * @brief Returns the count of nodes used by a given unwind code.
 *
 * @param p_unwindCode Unwind code to get the node count from.
 * @return DWORD Count of nodes, or 0 on error.
 */
DWORD __callobf_getNodesUsed(
    PUNWIND_CODE p_unwindCode);

/**
 * @brief For a given unwind info and a register, finds the offset where the register is saved.
 *        Returns -1 if the register is not saved or error found.
 *
 * @param p_module Module containing given unwind info.
 * @param p_unwindInfo Unwind info to find offset from.
 * @param reg Register to find ofset for.
 * @return DWORD64 Offset where register is saved or -1.
 */
LONG64 __callobf_getOffsetWhereRegSaved(
    PVOID p_module,
    PUNWIND_INFO p_unwindInfo,
    REGISTERS reg);

#endif