#ifndef _CALL_DISPATCHER_H_
#define _CALL_DISPATCHER_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"
#include "stackSpoof/stackSpoof.h"

// ==========================================================
// ================= STRUCT DEFINITIONS =====================

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
    DWORD ssn; // As any other time that ssn is defined as a 4byte value, means that the to lower bytes are either 0xFF or 0x00, and means if is actually a valid ssn
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

// ==========================================================
// ====================== GLOBALS ===========================

unsigned long __callobf_lastError = 0;

// ==========================================================
// ================= EXTERNAL GLOBALS =======================

extern DLL_TABLE __callobf_dllTable;
extern FUNCTION_TABLE __callobf_functionTable;

// ==========================================================
// ================ EXTERNAL FUNCTIONS ======================

extern void *__callobf_doCall(
    PVOID p_function,
    WORD ssn,
    DWORD isSyscall,
    DWORD argCount,
    PVOID p_args,
    PVOID p_returnAddress,
    PSTACK_SPOOF_INFO p_globalFrameTable);

// ==========================================================
// ================= PUBLIC  FUNCTIONS ======================

void *__callobf_callDispatcher(DWORD32 index, ...);

// ==========================================================
// ================= PRIVATE FUNCTIONS ======================

unsigned long __callobf_getLastError();
void *__callobf_loadFunction(PFUNCTION_TABLE_ENTRY p_fEntry);

#endif