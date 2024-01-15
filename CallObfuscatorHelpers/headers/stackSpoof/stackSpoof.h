#ifndef _DYN_STACK_SPOOF_H_
#define _DYN_STACK_SPOOF_H_

#include "common/common.h"

// ==========================================================
// ================= MACRO DEFINITIONS ======================

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

// ==========================================================
// ================= STRUCT DEFINITIONS =====================

// TODO: Mby rename from FRAME_TABLE_ENTRY to something like FRAME_INFO
typedef struct _FRAME_TABLE_ENTRY
{
    SIZE_T frameSize;
    PVOID p_entryAddr;
} FRAME_TABLE_ENTRY, *PFRAME_TABLE_ENTRY;

// TODO: Rename everything realted to pushRbp to saveRbp
typedef struct _PUSH_RBP_FRAME_TABLE_ENTRY
{
    SIZE_T frameSize;
    PVOID p_entryAddr;
    DWORD64 rbpOffset;
} PUSH_RBP_FRAME_TABLE_ENTRY, *PPUSH_RBP_FRAME_TABLE_ENTRY;

#pragma pack(push, 1)
typedef struct _FRAME_TABLE
{
    DWORD64 initialized;
    DWORD64 entryCountPerList;

    DWORD32 nextCiclicValue;
    DWORD32 __seedPadding;

    PVOID *p_entryRetAddr;

    DWORD64 addRspCount;
    FRAME_TABLE_ENTRY addRspList[MAX_ENTRIES_PER_LIST];

    DWORD64 jmpRbxCount;
    FRAME_TABLE_ENTRY jmpRbxList[MAX_ENTRIES_PER_LIST];

    DWORD64 setFpRegCount;
    FRAME_TABLE_ENTRY setFpRegList[MAX_ENTRIES_PER_LIST];

    DWORD64 pushRbpCount;
    PUSH_RBP_FRAME_TABLE_ENTRY pushRbpList[MAX_ENTRIES_PER_LIST];
} STACK_SPOOF_INFO, *PSTACK_SPOOF_INFO;

#pragma pack(pop)

// ==========================================================
// ================ EXTERNAL FUNCTIONS ======================

extern STACK_SPOOF_INFO __callobf_globalFrameTable;
extern PVOID __callobf_buildSpoofedCallStack(PSTACK_SPOOF_INFO);

// ==========================================================
// ================= PUBLIC  FUNCTIONS ======================

DWORD64 __callobf_fillGadgetTable(
    PVOID p_module,
    PFRAME_TABLE_ENTRY p_entry,
    DWORD64 maxEntries,
    PBYTE gadgetBytes,
    PBYTE mask,
    SIZE_T gadgetSize);

DWORD64 __callobf_fillFpRegFrameTable(
    PVOID p_module,
    PFRAME_TABLE_ENTRY p_entry,
    DWORD64 maxEntries);

DWORD64 __callobf_fillPushRbpFrameTable(
    PVOID p_module,
    PPUSH_RBP_FRAME_TABLE_ENTRY p_entry,
    DWORD64 maxEntries);

BOOL __callobf_fillStackSpoofTables(
    PSTACK_SPOOF_INFO p_frameTable,
    PVOID p_module);

BOOL __callobf_initializeSpoofInfo(
    PSTACK_SPOOF_INFO p_frameTable);
#endif