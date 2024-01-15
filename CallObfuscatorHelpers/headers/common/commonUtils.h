#ifndef _UTILS_H_
#define _UTILS_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"

// ==========================================================
// ================= MACRO DEFINITIONS ======================

// Constants:
#define HASH_MULTIPLIER 37
#define RAND_SEED 123456

// Utils:
#define MID_POINT_ADDR(begin, end) ((end >= begin) ? ((PVOID)((DWORD_PTR)begin + (((DWORD_PTR)end - (DWORD_PTR)begin) / 2))) : 0)

// ==========================================================
// ================= PUBLIC  FUNCTIONS ======================

PVOID __callobf_memset(
    const PVOID p_bytes,
    const BYTE c,
    size_t len);

DWORD __callobf_rand(PDWORD next);
VOID __callobf_srand(PDWORD next, DWORD seed);
DWORD32 __callobf_lfsrXorShift32(DWORD32 prevVal);

UINT32 __callobf_hashA(const PCHAR p_str);
UINT32 __callobf_hashW(const PWCHAR p_str);
UINT32 __callobf_hashU(const PUNICODE_STRING p_str);

PVOID __callobf_findBytes(
    const PVOID startAddr,
    const PVOID endAddr,
    const PBYTE bytes,
    const PBYTE mask,
    const DWORD byteCount);

PVOID __callobf_getStackBase();
PVOID __callobf_getStackLimit();

#endif