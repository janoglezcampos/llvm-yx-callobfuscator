#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "common/common.h"

// ==========================================================
// ================= STRUCT DEFINITIONS =====================

typedef struct _SYSCALL_ITER_CTX
{
    PVOID p_ntdll;
    PIMAGE_EXPORT_DIRECTORY p_expDir;
    DWORD64 lastEntry;
} SYSCALL_ITER_CTX, *PSYSCALL_ITER_CTX;

// ==========================================================
// ================= PUBLIC  FUNCTIONS ======================

BOOL __callobf_loadSyscall(
    DWORD32 nameHash,
    PVOID p_ntdll,
    PUSHORT p_ssn,
    PVOID *pp_function);

BOOL __callobf_iterateSyscalls(
    PSYSCALL_ITER_CTX p_ctx,
    PCHAR *pp_name,
    PVOID *pp_functionAddr);

UINT32 __callobf_hashSyscallA(
    PCHAR p_str,
    BOOL asZw);

BOOL __callobf_checkHashSyscallA(
    PCHAR p_functionName,
    DWORD32 hash);

PVOID __callobf_getSyscallAddr(
    PVOID p_function,
    PVOID p_lowBoundary,
    PVOID p_highBoundary);

#endif