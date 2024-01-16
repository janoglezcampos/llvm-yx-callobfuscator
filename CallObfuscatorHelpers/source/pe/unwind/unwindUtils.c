/**
 * @file unwindUtils.c
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

#include "pe/unwind/unwindUtils.h"
#include "common/commonUtils.h"
#include "pe/peUtils.h"

BOOL __callobf_createOrResetUnwindInfoIterator(
    _Out_ PUNWIND_INFO_ITERATOR_CONTEXT p_ctx,
    _In_ HMODULE p_moduleBase)
{
    DWORD runtimeFunctionTableSize = 0;

    if (!p_ctx || !p_moduleBase)
        return FALSE;

    p_ctx->p_moduleBase = p_moduleBase;
    p_ctx->nextIndex = 0;
    p_ctx->ended = FALSE;

    p_ctx->p_runtimeFunctionTable = (PIMAGE_RUNTIME_FUNCTION_ENTRY)(__callobf_getExceptionDirectoryAddress(p_moduleBase, &runtimeFunctionTableSize));
    p_ctx->entryCount = (DWORD)(runtimeFunctionTableSize / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
    return TRUE;
}

PUNWIND_INFO __callobf_getNextUnwindInfo(
    PUNWIND_INFO_ITERATOR_CONTEXT p_ctx)
{
    if (!p_ctx)
        return NULL;

    PIMAGE_RUNTIME_FUNCTION_ENTRY p_runtimeFunctionTable = p_ctx->p_runtimeFunctionTable;
    DWORD currentIndex = p_ctx->nextIndex;

    if (currentIndex >= p_ctx->entryCount)
    {
        p_ctx->ended = TRUE;
        return NULL;
    }

    p_ctx->nextIndex++;
    return (PUNWIND_INFO)((UINT64)p_ctx->p_moduleBase + (DWORD)p_runtimeFunctionTable[currentIndex].UnwindData);
}

BOOL __callobf_createOrResetUwopIterator(
    _Out_ PUWOP_ITERATOR_CONTEXT p_ctx,
    _In_ HMODULE p_moduleBase,
    _In_ PUNWIND_INFO p_unwindInfoAddress)
{
    if (!p_ctx || !p_moduleBase || !p_unwindInfoAddress)
        return FALSE;

    p_ctx->p_moduleBase = p_moduleBase;
    p_ctx->p_unwindInfo = p_unwindInfoAddress;
    p_ctx->nextIndex = 0;
    p_ctx->ended = FALSE;
    return TRUE;
}

PUNWIND_CODE __callobf_getNextUwop(
    _Inout_ PUWOP_ITERATOR_CONTEXT p_ctx)
{
    PRUNTIME_FUNCTION p_chainedFunction;
    PUNWIND_CODE p_unwindCode = (PUNWIND_CODE)p_ctx->p_unwindInfo->UnwindCode;
    PUNWIND_CODE p_nextUnwindCode = NULL;

    DWORD countOfCodes = p_ctx->p_unwindInfo->CountOfCodes;

    if (p_ctx->ended)
        return NULL;

    if (p_ctx->nextIndex >= countOfCodes)
    {
        if (BitChainInfo(p_ctx->p_unwindInfo->Flags))
        {
            PUNWIND_INFO p_unwindInfoBackup = p_ctx->p_unwindInfo;
            PUNWIND_CODE tmpUwop = NULL;

            p_chainedFunction = (PRUNTIME_FUNCTION)(&p_unwindCode[(countOfCodes + 1) & ~1]);

            p_ctx->nextIndex -= countOfCodes;
            p_ctx->p_unwindInfo = (PUNWIND_INFO)((UINT64)p_ctx->p_moduleBase + (DWORD)p_chainedFunction->UnwindData);
            tmpUwop = __callobf_getNextUwop(p_ctx);
            p_ctx->p_unwindInfo = p_unwindInfoBackup;
            p_ctx->nextIndex += countOfCodes;

            return tmpUwop;
        }
        p_ctx->ended = TRUE;
        return NULL;
    }
    p_nextUnwindCode = &p_unwindCode[p_ctx->nextIndex];

    p_ctx->nextIndex += __callobf_getNodesUsed(&p_unwindCode[p_ctx->nextIndex]);

    return p_nextUnwindCode;
}

DWORD __callobf_getNodesUsed(PUNWIND_CODE p_unwindCode)
{
    DWORD nodeCount = 0;

    if (!p_unwindCode)
        return 0;

    switch (p_unwindCode->UnwindOp)
    {
    case UWOP_PUSH_NONVOL:     // 0
    case UWOP_ALLOC_SMALL:     // 2
    case UWOP_SET_FPREG:       // 3
    case UWOP_PUSH_MACH_FRAME: // 10
        break;

    case UWOP_SAVE_NONVOL: // 4
    case UWOP_EPILOG:      // 6
    case UWOP_SAVE_XMM128: // 8
        nodeCount++;
        break;

    case UWOP_ALLOC_LARGE: // 1
        nodeCount++;
        if (p_unwindCode->OpInfo != 0)
            nodeCount++;
        break;

    case UWOP_SAVE_NONVOL_BIG: // 5
    case UWOP_SPARE_CODE:      // 7
    case UWOP_SAVE_XMM128BIG:  // 9
        nodeCount += 2;
        break;
    }
    return (nodeCount + 1);
}

LONG __callobf_getStackSizeModification(PUNWIND_CODE p_unwindCode)
{
    LONG stackSizeMod = 0;

    if (!p_unwindCode)
        return 0;

    switch (p_unwindCode->UnwindOp)
    {
    case UWOP_PUSH_NONVOL: // 0
        stackSizeMod = 8;
        break;
    case UWOP_ALLOC_SMALL: // 2
        stackSizeMod = 8 * (p_unwindCode->OpInfo + 1);
        break;
    case UWOP_SET_FPREG: // 3
        break;
    case UWOP_PUSH_MACH_FRAME: // 10
        if (p_unwindCode->OpInfo == 0)
        {
            stackSizeMod = 0x40;
            break;
        }
        stackSizeMod = 0x48;
        break;

    case UWOP_SAVE_NONVOL: // 4
    case UWOP_EPILOG:      // 6
    case UWOP_SAVE_XMM128: // 8
        break;

    case UWOP_ALLOC_LARGE: // 1
        stackSizeMod = p_unwindCode[1].FrameOffset;

        if (p_unwindCode->OpInfo == 0)
        {
            stackSizeMod *= 8;
        }
        else
        {
            stackSizeMod += p_unwindCode[2].FrameOffset << 16;
        }
        break;

    case UWOP_SAVE_NONVOL_BIG: // 5
    case UWOP_SPARE_CODE:      // 7
    case UWOP_SAVE_XMM128BIG:  // 9
        break;
    }
    return stackSizeMod;
}

LONG __callobf_getFrameSizeModification(PUNWIND_INFO p_unwindInfo, PUNWIND_CODE p_unwindCode)
{
    LONG frameSizeMod = 0;
    if (!p_unwindInfo || !p_unwindCode)
        return 0;

    switch (p_unwindCode->UnwindOp)
    {
    case UWOP_SET_FPREG: // 3
        frameSizeMod = -0x10 * (p_unwindInfo->FrameOffset);
        break;
    default:
        frameSizeMod = __callobf_getStackSizeModification(p_unwindCode);
    }
    return frameSizeMod;
}

PUNWIND_INFO __callobf_getUnwindInfoForCodePtr(PVOID p_module, PVOID p_code, PVOID *pp_functionStart, PVOID *pp_functionEnd)
{
    DWORD_PTR startAddress = 0;
    DWORD_PTR endAddress = 0;

    DWORD runtimeFunctionTableSize = 0;
    DWORD rtEntryCount = 0;
    PIMAGE_RUNTIME_FUNCTION_ENTRY p_runtimeFunctionTable = NULL;

    if (!p_module || !p_code)
        return NULL;

    p_runtimeFunctionTable = (PIMAGE_RUNTIME_FUNCTION_ENTRY)(__callobf_getExceptionDirectoryAddress(p_module, &runtimeFunctionTableSize));

    if (!p_runtimeFunctionTable || !runtimeFunctionTableSize)
        return NULL;

    rtEntryCount = (DWORD)(runtimeFunctionTableSize / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));

    for (DWORD i = 0; i < rtEntryCount; i++)
    {
        startAddress = (DWORD_PTR)p_module + p_runtimeFunctionTable[i].BeginAddress;
        endAddress = (DWORD_PTR)p_module + p_runtimeFunctionTable[i].EndAddress;

        if ((DWORD_PTR)p_code >= startAddress && (DWORD_PTR)p_code < endAddress)
        {
            if (pp_functionStart)
                *pp_functionStart = (PVOID)startAddress;

            if (pp_functionEnd)
                *pp_functionEnd = (PVOID)endAddress;

            return (PUNWIND_INFO)((UINT64)p_module + (DWORD)p_runtimeFunctionTable[i].UnwindData);
        }
    }
    return NULL;
}

PVOID __callobf_findEntryAddressOfReturnAddress(PVOID p_ntdll, PVOID p_kernel32)
{
    PVOID p_BaseThreadInitThunk = NULL;
    PVOID p_RtlUserThreadStart = NULL;

    PVOID p_BaseThreadInitThunkEnd = NULL;
    PVOID p_RtlUserThreadStartEnd = NULL;

    PUNWIND_INFO p_unwindInfoBtit = NULL;
    PUNWIND_INFO p_unwindInfoRuts = NULL;

    SIZE_T frameSizeBtit = 0;
    SIZE_T frameSizeRuts = 0;

    UWOP_ITERATOR_CONTEXT ctx = {0};
    PUNWIND_CODE p_uwop = NULL;

    if (!p_ntdll || !p_kernel32)
        return NULL;

    p_BaseThreadInitThunk = __callobf_getFunctionAddrH(p_kernel32, BASETRHEADINITTHUNK_HASH);
    p_RtlUserThreadStart = __callobf_getFunctionAddrH(p_ntdll, RTLUSERTHREADSTART_HASH);

    if (!p_BaseThreadInitThunk || !p_RtlUserThreadStart)
        return NULL;

    p_unwindInfoBtit = __callobf_getUnwindInfoForCodePtr(p_kernel32, p_BaseThreadInitThunk, NULL, &p_BaseThreadInitThunkEnd);
    p_unwindInfoRuts = __callobf_getUnwindInfoForCodePtr(p_ntdll, p_RtlUserThreadStart, NULL, &p_RtlUserThreadStartEnd);

    if (!p_unwindInfoBtit || !p_unwindInfoRuts)
        return NULL;

    __callobf_createOrResetUwopIterator(&ctx, p_kernel32, p_unwindInfoBtit);
    while ((p_uwop = __callobf_getNextUwop(&ctx)))
        frameSizeBtit += __callobf_getFrameSizeModification(p_unwindInfoBtit, p_uwop);

    __callobf_createOrResetUwopIterator(&ctx, p_ntdll, p_unwindInfoRuts);
    while ((p_uwop = __callobf_getNextUwop(&ctx)))
        frameSizeRuts += __callobf_getFrameSizeModification(p_unwindInfoRuts, p_uwop);

    // We start searching from the stack base and go downwards (or upwards, however u want to see it xd)
    // We search for a configuration like:
    // 0xN                       : value >= p_BaseThreadInitThunk && value < p_BaseThreadInitThunkEnd
    // 0xN + frameSizeBtit + 0x8 : value >= p_RtlUserThreadStart && value < p_RtlUserThreadStartEnd
    // 0xN + frameSizeRuts + 0x8 : value == 0

    PDWORD64 stackBase = __callobf_getStackBase();
    PDWORD64 stackLimit = __callobf_getStackLimit();
    DWORD_PTR value = 0;

    // Rember stack base is the lower limit, but it is not included, so the first readable quadword
    // is found at  stackBase - 0x8

    // Also, we know the block of addresses have a certain size, so we can discard the starting addresses
    // where it can not be, so we dont need to check for access violations

    stackBase = (PDWORD64)((DWORD_PTR)stackBase - 8);
    stackBase = (PDWORD64)((DWORD_PTR)stackBase - (frameSizeRuts + frameSizeBtit + 0x10));
    for (PDWORD64 runner = stackBase; runner >= stackLimit; runner--)
    {
        value = *runner;
        if (value >= (DWORD_PTR)p_BaseThreadInitThunk && value < (DWORD_PTR)p_BaseThreadInitThunkEnd)
        {
            value = *(PDWORD64)((DWORD_PTR)runner + frameSizeBtit + 0x8);
            if (value >= (DWORD_PTR)p_RtlUserThreadStart && value < (DWORD_PTR)p_RtlUserThreadStartEnd)
            {

                value = *(PDWORD64)((DWORD_PTR)runner + frameSizeBtit + 0x8 + frameSizeRuts + 0x8);
                if (value == 0)
                    return (PVOID)runner;
            }
        }
    }

    return NULL;
}

BOOL __callobf_getCodeBoundariesLastUnwindInfo(
    PUNWIND_INFO_ITERATOR_CONTEXT p_ctx,
    PVOID *pp_beginAddress,
    PVOID *pp_endAddress)
{
    PIMAGE_RUNTIME_FUNCTION_ENTRY p_runtimeFunctionTable = p_ctx->p_runtimeFunctionTable;

    if (!p_ctx || !pp_beginAddress || !pp_endAddress)
        return FALSE;

    if (p_ctx->nextIndex <= 0)
        return FALSE;

    DWORD currentIndex = p_ctx->nextIndex - 1;

    if (currentIndex >= p_ctx->entryCount)
        return FALSE;

    if (pp_beginAddress)
        *pp_beginAddress = (PVOID)((UINT64)p_ctx->p_moduleBase + (DWORD)p_runtimeFunctionTable[currentIndex].BeginAddress);

    if (pp_endAddress)
        *pp_endAddress = (PVOID)((UINT64)p_ctx->p_moduleBase + (DWORD)p_runtimeFunctionTable[currentIndex].EndAddress);

    return TRUE;
}

LONG64 __callobf_getOffsetWhereRegSaved(PVOID p_module, PUNWIND_INFO p_unwindInfo, REGISTERS reg)
{
    UWOP_ITERATOR_CONTEXT uwopCtx = {0};
    PUNWIND_CODE p_uwop = NULL;
    LONG stackSize = 0;

    if (!p_module || !p_unwindInfo)
        return -1;

    if (!__callobf_createOrResetUwopIterator(&uwopCtx, p_module, p_unwindInfo))
        return -1;

    while ((p_uwop = __callobf_getNextUwop(&uwopCtx)))
    {
        if (p_uwop->OpInfo == reg)
        {
            switch (p_uwop->UnwindOp)
            {
            case UWOP_PUSH_NONVOL: // 0
                return stackSize;
            case UWOP_SAVE_NONVOL: // 0
                return (p_uwop[1].FrameOffset * 8);

            case UWOP_SAVE_NONVOL_BIG: // 0
                return (p_uwop[2].FrameOffset << 16 | p_uwop[1].FrameOffset);
            default:
                break;
            }
        }
        stackSize += __callobf_getStackSizeModification(p_uwop);
    }
    return -1;
}