/**
 * @file stackSpoof.c
 * @author Alejandro González (@httpyxel)
 * @brief Functionality to apply dynamic stack spoofing in windows x64 enviroments.
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

// Many thanks to the project showing this idea: https://github.com/klezVirus/klezVirus.github.io

#include "stackSpoof/stackSpoof.h"

#include "common/commonUtils.h"
#include "pe/peUtils.h"
#include "pe/unwind/unwindUtils.h"

STACK_SPOOF_INFO __callobf_globalFrameTable = {
    .entryCountPerList = MAX_ENTRIES_PER_LIST,
    .nextCiclicValue = 0x89235482,
    .initialized = 0,
    .p_entryRetAddr = NULL,

    .addRspCount = 0,
    .addRspList = {0},

    .jmpRbxCount = 0,
    .jmpRbxList = {0},

    .setFpRegList = {0},
    .saveRbpList = {0}};

DWORD64 __callobf_fillGadgetTable(
    PVOID p_module,
    PFRAME_INFO p_entryList,
    DWORD64 maxEntries,
    PBYTE p_gadgetBytes,
    PBYTE p_mask,
    SIZE_T gadgetSize)
{
    UWOP_ITERATOR_CONTEXT uwopCtx = {0};

    PUNWIND_INFO p_unwindInfo = NULL;
    PUNWIND_CODE p_uwop = NULL;
    PVOID p_gadget = NULL;

    SIZE_T frameSize = 0;

    PVOID gadgetLowerSearch = NULL;
    PVOID gadgetHigherSearch = NULL;

    DWORD64 entryCount = 0;
    BOOL valid = TRUE;

    if (!p_module || !p_entryList || !maxEntries || !p_gadgetBytes || !p_mask || !gadgetSize)
        return entryCount;

    if (!__callobf_getCodeBoundaries(p_module, &gadgetLowerSearch, &gadgetHigherSearch))
        return entryCount;

    while ((p_gadget = __callobf_findBytes(gadgetLowerSearch, gadgetHigherSearch, p_gadgetBytes, p_mask, gadgetSize)))
    {
        gadgetLowerSearch = (PVOID)((DWORD_PTR)p_gadget + gadgetSize);

        if (entryCount >= maxEntries)
            break;

        p_unwindInfo = __callobf_getUnwindInfoForCodePtr(p_module, p_gadget, NULL, NULL);
        if (!p_unwindInfo)
            continue;

        if (!__callobf_createOrResetUwopIterator(&uwopCtx, p_module, p_unwindInfo))
            return entryCount;

        frameSize = 0;
        valid = TRUE;

        while ((p_uwop = __callobf_getNextUwop(&uwopCtx)))
        {
            switch (p_uwop->UnwindOp)
            {
            case UWOP_PUSH_NONVOL:
            case UWOP_SAVE_NONVOL:
            case UWOP_SAVE_NONVOL_BIG:
                if (p_uwop->OpInfo == RSP)
                    valid = FALSE;
                break;
            case UWOP_SET_FPREG:
                valid = FALSE;
                break;
            default:
                break;
            }
            if (!valid)
                break;
            frameSize += __callobf_getFrameSizeModification(p_unwindInfo, p_uwop);
        }
        if (valid)
        {
            p_entryList[entryCount].p_entryAddr = p_gadget;
            p_entryList[entryCount].frameSize = frameSize;
            entryCount++;
        }
    }
    return entryCount;
}

DWORD64 __callobf_fillFpRegFrameTable(
    PVOID p_module,
    PFRAME_INFO p_entryList,
    DWORD64 maxEntries)
{
    UWOP_ITERATOR_CONTEXT uwopCtx = {0};
    UNWIND_INFO_ITERATOR_CONTEXT unwindInfoCtx = {0};

    PUNWIND_INFO p_unwindInfo = NULL;
    PUNWIND_CODE p_uwop = NULL;

    SIZE_T frameSize = 0;

    DWORD64 entryCount = 0;
    BOOL valid = TRUE;
    BOOL foundFpRegOp = FALSE;

    if (!p_module || !p_entryList || !maxEntries)
        return entryCount;

    if (!__callobf_createOrResetUnwindInfoIterator(&unwindInfoCtx, p_module))
        return entryCount;

    while ((p_unwindInfo = __callobf_getNextUnwindInfo(&unwindInfoCtx)))
    {
        if (entryCount >= maxEntries)
            break;

        if (!__callobf_createOrResetUwopIterator(&uwopCtx, p_module, p_unwindInfo))
            return entryCount;

        frameSize = 0;
        valid = TRUE;
        foundFpRegOp = FALSE;

        while ((p_uwop = __callobf_getNextUwop(&uwopCtx)))
        {
            switch (p_uwop->UnwindOp)
            {
            case UWOP_PUSH_NONVOL: // 0
                if (p_uwop->OpInfo == RSP && !foundFpRegOp)
                    valid = FALSE;
                break;
            case UWOP_SAVE_NONVOL:     // 0
            case UWOP_SAVE_NONVOL_BIG: // 0
                if (p_uwop->OpInfo == RSP || p_uwop->OpInfo == RBP)
                    valid = FALSE;
                break;
            case UWOP_SET_FPREG:
                foundFpRegOp = TRUE;
                break;
            default:
                break;
            }
            if (!valid)
                break;
            frameSize += __callobf_getFrameSizeModification(p_unwindInfo, p_uwop);
        }
        if (valid && foundFpRegOp)
        {
            PVOID p_begin = NULL;
            PVOID p_end = NULL;

            if (!__callobf_getCodeBoundariesLastUnwindInfo(&unwindInfoCtx, &p_begin, &p_end))
                return entryCount;

            p_entryList[entryCount].p_entryAddr = MID_POINT_ADDR(p_begin, p_end); // random ptr inside function
            p_entryList[entryCount].frameSize = frameSize;
            entryCount++;
        }
    }

    return entryCount;
}

DWORD64 __callobf_fillSaveRbpFrameTable(
    PVOID p_module,
    PSAVE_RBP_FRAME_INFO p_entryList,
    DWORD64 maxEntries)
{
    UWOP_ITERATOR_CONTEXT uwopCtx = {0};
    UNWIND_INFO_ITERATOR_CONTEXT unwindInfoCtx = {0};

    PUNWIND_INFO p_unwindInfo = NULL;
    PUNWIND_CODE p_uwop = NULL;

    SIZE_T frameSize = 0;

    DWORD64 entryCount = 0;
    BOOL valid = TRUE;
    BOOL foundSaveRbpOp = FALSE;

    if (!p_module || !p_entryList || !maxEntries)
        return entryCount;

    if (!__callobf_createOrResetUnwindInfoIterator(&unwindInfoCtx, p_module))
        return FALSE;

    while ((p_unwindInfo = __callobf_getNextUnwindInfo(&unwindInfoCtx)))
    {
        if (entryCount >= maxEntries)
            break;

        if (!__callobf_createOrResetUwopIterator(&uwopCtx, p_module, p_unwindInfo))
            return FALSE;

        frameSize = 0;
        valid = TRUE;
        foundSaveRbpOp = FALSE;
        while ((p_uwop = __callobf_getNextUwop(&uwopCtx)))
        {
            switch (p_uwop->UnwindOp)
            {
            case UWOP_PUSH_NONVOL:     // 0
            case UWOP_SAVE_NONVOL:     // 0
            case UWOP_SAVE_NONVOL_BIG: // 0

                if (p_uwop->OpInfo == RSP)
                    valid = FALSE;

                if (p_uwop->OpInfo == RBP)
                {
                    if (foundSaveRbpOp)
                    {
                        valid = FALSE;
                        break;
                    }
                    foundSaveRbpOp = TRUE;
                }
                break;
            case UWOP_SET_FPREG:
                valid = FALSE;
                break;
            default:
                break;
            }
            if (!valid)
                break;
            frameSize += __callobf_getFrameSizeModification(p_unwindInfo, p_uwop);
        }
        if (valid && foundSaveRbpOp)
        {
            PVOID p_begin = NULL;
            PVOID p_end = NULL;
            LONG64 saveOffset = -1;

            if (!__callobf_getCodeBoundariesLastUnwindInfo(&unwindInfoCtx, &p_begin, &p_end))
                return entryCount;

            if (!(saveOffset = __callobf_getOffsetWhereRegSaved(p_module, p_unwindInfo, RBP)))
                return entryCount;

            p_entryList[entryCount].p_entryAddr = MID_POINT_ADDR(p_begin, p_end); // random ptr inside function
            p_entryList[entryCount].frameSize = frameSize;
            p_entryList[entryCount].rbpOffset = saveOffset;

            entryCount++;
        }
    }
    return entryCount;
}

BOOL __callobf_fillStackSpoofTables(PSTACK_SPOOF_INFO p_stackSpoofInfo, PVOID p_module)
{

    CASSERT((JUMP_RBX_GADGET_SIZE != 0));
    CASSERT((ADD_RSP_GADGET_SIZE != 0));

    CASSERT((sizeof(JUMP_RBX_GADGET) == sizeof(JUMP_RBX_GADGET_MASK)));
    CASSERT((sizeof(ADD_RSP_GADGET) == sizeof(ADD_RSP_GADGET_MASK)));

    if (!p_stackSpoofInfo || !p_module)
        return FALSE;

    if (!(p_stackSpoofInfo->addRspCount = __callobf_fillGadgetTable(
              p_module,
              p_stackSpoofInfo->addRspList,
              p_stackSpoofInfo->entryCountPerList,
              (PBYTE)ADD_RSP_GADGET,
              (PBYTE)ADD_RSP_GADGET_MASK,
              ADD_RSP_GADGET_SIZE)))
        return FALSE;

    if (!(p_stackSpoofInfo->jmpRbxCount = __callobf_fillGadgetTable(
              p_module,
              p_stackSpoofInfo->jmpRbxList,
              p_stackSpoofInfo->entryCountPerList,
              (PBYTE)JUMP_RBX_GADGET,
              (PBYTE)JUMP_RBX_GADGET_MASK,
              JUMP_RBX_GADGET_SIZE)))
        return FALSE;

    if (!(p_stackSpoofInfo->setFpRegCount = __callobf_fillFpRegFrameTable(
              p_module,
              p_stackSpoofInfo->setFpRegList,
              p_stackSpoofInfo->entryCountPerList)))
        return FALSE;

    if (!(p_stackSpoofInfo->saveRbpCount = __callobf_fillSaveRbpFrameTable(
              p_module,
              p_stackSpoofInfo->saveRbpList,
              p_stackSpoofInfo->entryCountPerList)))
        return FALSE;
    return TRUE;
}

BOOL __callobf_initializeSpoofInfo(PSTACK_SPOOF_INFO p_stackSpoofInfo)
{
    PVOID p_kernelBase = __callobf_getModuleAddrH(KERNELBASE_HASH);
    if (!p_kernelBase)
        return FALSE;

    PVOID p_kernel32 = __callobf_getModuleAddrA("kernel32.dll");
    if (!p_kernelBase)
        return FALSE;

    // iterateRuntimeFunctionTableNew(p_kernel32);
    PVOID p_ntdll = __callobf_getModuleAddrA("ntdll.dll");
    if (!p_ntdll)
        return FALSE;

    if (!__callobf_fillStackSpoofTables(p_stackSpoofInfo, p_kernelBase))
        return FALSE;

    if (!(p_stackSpoofInfo->p_entryRetAddr = __callobf_findEntryAddressOfReturnAddress(p_ntdll, p_kernel32)))
        return FALSE;

    p_stackSpoofInfo->initialized = TRUE;

    return TRUE;
}