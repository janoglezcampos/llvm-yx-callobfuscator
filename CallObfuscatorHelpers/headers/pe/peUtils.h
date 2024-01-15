#ifndef _PE_UTILS_H_
#define _PE_UTILS_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"

#ifndef NtCurrentTeb
extern PTEB NtCurrentTeb(void);
#endif

// ==========================================================
// ================= PUBLIC  FUNCTIONS ======================

PVOID __callobf_getModuleAddrA(const PCHAR p_moduleName);
PVOID __callobf_getModuleAddrW(const PWCHAR p_moduleName);
PVOID __callobf_getModuleAddrH(const UINT32 moduleHash);

PVOID __callobf_getFunctionAddrA(
    const PVOID p_module,
    const PCHAR p_functionName);

PVOID __callobf_getFunctionAddrU(
    const PVOID p_module,
    const PWCHAR p_functionName);

PVOID __callobf_getFunctionAddrH(
    const PVOID p_module,
    const UINT32 funtionHash);

PVOID __callobf_getExceptionDirectoryAddress(
    const PVOID p_module,
    PDWORD tSize);

BOOL __callobf_getCodeBoundaries(
    PVOID p_module,
    PVOID *pp_base,
    PVOID *pp_top);

#endif