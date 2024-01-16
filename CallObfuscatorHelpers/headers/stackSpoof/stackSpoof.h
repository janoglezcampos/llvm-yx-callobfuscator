/**
 * @file stackSpoof.h
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

#ifndef _DYN_STACK_SPOOF_H_
#define _DYN_STACK_SPOOF_H_

#include "common/common.h"

// ==============================================================================
// ============================= MACRO DEFINITIONS ==============================

// Constants:
#define MAX_ENTRIES_PER_LIST 10

#define JUMP_RBX_GADGET "\xff\x23"
#define JUMP_RBX_GADGET_MASK "\xff\xff"
#define JUMP_RBX_GADGET_SIZE sizeof(JUMP_RBX_GADGET) - 1

#define ADD_RSP_GADGET "\x48\x83\xC4\x90\xC3"
#define ADD_RSP_GADGET_MASK "\xff\xff\xff\x00\xff"
#define ADD_RSP_GADGET_SIZE sizeof(ADD_RSP_GADGET) - 1

// TODO: Allow to search for multiple ranges, like, 50% of the entries with minimun stack size of <whatever>
// TODO: and the other 50% with minimun of <whatever>, so we have multiple options to choose from
#define MIN_ADD_RSP_FRAME_SIZE ((0x8 * 15) + 0x20) // This sets how many args we will be able to store, in this case 15 args

// Utils:
#define REQUIRED_STACK(NARGS) (((POSITIVE_OR_ZERO(NARGS - 4) * 8) + 0x20) | 8)

// ==============================================================================
// ============================ STRUCT DEFINITIONS ==============================

typedef struct _FRAME_INFO
{
    SIZE_T frameSize;
    PVOID p_entryAddr;
} FRAME_INFO, *PFRAME_INFO;

typedef struct _SAVE_RBP_FRAME_INFO
{
    SIZE_T frameSize;
    PVOID p_entryAddr;
    DWORD64 rbpOffset;
} SAVE_RBP_FRAME_INFO, *PSAVE_RBP_FRAME_INFO;

#pragma pack(push, 1)
typedef struct _FRAME_TABLE
{
    DWORD64 initialized;
    DWORD64 entryCountPerList;

    DWORD32 nextCiclicValue;
    DWORD32 __seedPadding;

    PVOID *p_entryRetAddr;

    DWORD64 addRspCount;
    FRAME_INFO addRspList[MAX_ENTRIES_PER_LIST];

    DWORD64 jmpRbxCount;
    FRAME_INFO jmpRbxList[MAX_ENTRIES_PER_LIST];

    DWORD64 setFpRegCount;
    FRAME_INFO setFpRegList[MAX_ENTRIES_PER_LIST];

    DWORD64 saveRbpCount;
    SAVE_RBP_FRAME_INFO saveRbpList[MAX_ENTRIES_PER_LIST];
} STACK_SPOOF_INFO, *PSTACK_SPOOF_INFO;

#pragma pack(pop)

// ==============================================================================
// =========================== EXTERNAL GLOBALS =================================

extern STACK_SPOOF_INFO __callobf_globalFrameTable;

// ==============================================================================
// =========================== EXTERNAL FUNCTIONS ===============================

/**
 * @brief Given stack spoof info, build a new spoofed stack picking different
 *        entries each time. It will build the spoofed stach after its stack
 *        frame ended, that means that is save to use the stack after a call
 *        __callobf_buildSpoofedCallStack, if only we dont call more functions.
 *        Even if its possible, this function should not be used from C.
 *
 * @return PVOID Pointer to start of the spoofed stack.
 */
extern PVOID __callobf_buildSpoofedCallStack(PSTACK_SPOOF_INFO);

// ==============================================================================
// ============================ PUBLIC  FUNCTIONS ===============================

/**
 * @brief Given a module, finds the gadgets specified by the convination
 *        of p_gadgetBytes, p_mask and gadgetSize. For every found gadget,
 *        writes its frame info into the list given in p_entryList, until
 *        no more gagets or the number of gadgets found equals maxEntries.
 *
 * @param p_module Module to search gadgets from.
 * @param p_entryList List of frame info to add the gadgets to.
 * @param maxEntries Maximun number of entries to add.
 * @param p_gadgetBytes Byte pattern of the gadget.
 * @param p_mask Mask to apply to read bytes before checking.
 * @param gadgetSize Size of p_gadgetBytes buffer.
 * @return DWORD64 Number of added entries to the list.
 */
DWORD64 __callobf_fillGadgetTable(
    PVOID p_module,
    PFRAME_INFO p_entryList,
    DWORD64 maxEntries,
    PBYTE p_gadgetBytes,
    PBYTE p_mask,
    SIZE_T gadgetSize);

/**
 * @brief Given a module, finds frames containing UWOP_SET_FPREG. For every found
 *        gadget, writes its frame info into the list given in p_entryList, until
 *        no more frames are or the number of frames found equals maxEntries.
 *
 * @param p_module Module to search frames from.
 * @param p_entryList List of frame info to add the frames to.
 * @param maxEntries Maximun number of entries to add.
 * @return DWORD64 Number of added entries to the list.
 */
DWORD64 __callobf_fillFpRegFrameTable(
    PVOID p_module,
    PFRAME_INFO p_entryList,
    DWORD64 maxEntries);

/**
 * @brief Given a module, finds frames containing a save of rsp to the stack. For every
 *        found gadget, writes its frame info (including the offset where rbp is save)
 *        into the list given in p_entryList, until no more frames are or the number of
 *        frames found equals maxEntries.
 *
 * @param p_module Module to search frames from.
 * @param p_entryList List of frame info to add the frames to.
 * @param maxEntries Maximun number of entries to add.
 * @return DWORD64 Number of added entries to the list.
 */
DWORD64 __callobf_fillSaveRbpFrameTable(
    PVOID p_module,
    PSAVE_RBP_FRAME_INFO p_entryList,
    DWORD64 maxEntries);

/**
 * @brief Given a partially initialized stack spoof info, fills the four lists of
 *        frames needed at runtime.
 *
 * @param p_stackSpoofInfo Pointer to spoof info containing the lists to be fille.
 * @param p_module Module to search frames from.
 * @return BOOL Success.
 */
BOOL __callobf_fillStackSpoofTables(
    PSTACK_SPOOF_INFO p_stackSpoofInfo,
    PVOID p_module);

/**
 * @brief Given an unitialized stack spoof info, initialized its basic fields.
 *
 * @param p_stackSpoofInfo Pointer to spoof to initialize.
 * @return BOOL Success.
 */
BOOL __callobf_initializeSpoofInfo(
    PSTACK_SPOOF_INFO p_stackSpoofInfo);
#endif