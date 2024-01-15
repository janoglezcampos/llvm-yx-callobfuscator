#ifndef _UNWIND_UTILS_H_
#define _UNWIND_UTILS_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"
#include "common/debug.h"

// ==========================================================
// ================= STRUCT DEFINITIONS =====================

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

// ==========================================================
// ================= PUBLIC  FUNCTIONS ======================

BOOL __callobf_createOrResetUnwindInfoIterator(
    _Out_ PUNWIND_INFO_ITERATOR_CONTEXT p_ctx,
    _In_ HMODULE p_moduleBase);

PUNWIND_INFO __callobf_getNextUnwindInfo(
    PUNWIND_INFO_ITERATOR_CONTEXT p_ctx);

BOOL __callobf_getCodeBoundariesLastUnwindInfo(
    PUNWIND_INFO_ITERATOR_CONTEXT p_ctx,
    PVOID *pp_beginAddress,
    PVOID *pp_endAddress);

BOOL __callobf_createOrResetUwopIterator(
    _Out_ PUWOP_ITERATOR_CONTEXT p_ctx,
    _In_ HMODULE p_moduleBase,
    _In_ PUNWIND_INFO p_unwindInfoAddress);

PUNWIND_CODE __callobf_getNextUwop(
    PUWOP_ITERATOR_CONTEXT p_ctx);

LONG __callobf_getFrameSizeModification(
    PUNWIND_INFO p_unwindInfo,
    PUNWIND_CODE p_unwindCode);

LONG __callobf_getStackSizeModification(
    PUNWIND_CODE p_unwindCode);

PVOID __callobf_findEntryAddressOfReturnAddress(
    PVOID p_ntdll,
    PVOID p_kernel32);

PUNWIND_INFO __callobf_getUnwindInfoForCodePtr(
    PVOID p_module,
    PVOID p_code,
    PVOID *p_functionStart,
    PVOID *p_functionEnd);

DWORD __callobf_getNodesUsed(
    PUNWIND_CODE p_unwindCode);

DWORD64 __callobf_getOffsetWhereRegSaved(
    PVOID p_module,
    PUNWIND_INFO p_unwindInfo,
    REGISTERS reg);

#endif