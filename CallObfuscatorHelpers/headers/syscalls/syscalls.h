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

/**
 * @brief Given the address of ntdll, initializes a syscall iterator context.
 *
 * @param p_ctx Pointer to the iterator context.
 * @param p_ntdll Pointer to ntdll.
 * @return BOOL Success.
 */
BOOL __callobf_initSyscallIter(
    PSYSCALL_ITER_CTX p_ctx,
    PVOID p_ntdll);

/**
 * @brief Given a syscall iterator context, gets the next syscalls address and stub
 *        name, and modifies the ctx so the next call to __callobf_iterateSyscalls
 *        returns the next stub. The function names are always the Zw version of the
 *        function. If there is no more stubs remaining or an error
 *        occurs, return FALSE.
 *
 * @param p_ctx Pointer to the iterator context.
 * @param pp_name Double pointer to return the function name.
 * @param pp_functionAddr Double pointer to return function address.
 * @return BOOL TRUE if next stub found, FALSE in any other case.
 */
BOOL __callobf_iterateSyscalls(
    PSYSCALL_ITER_CTX p_ctx,
    PCHAR *pp_name,
    PVOID *pp_functionAddr);

/**
 * @brief Given a function name hash and a pointer to ntdll, checks if the function is
 *        a syscall, if it is, returns its ssn and syscall pointer.
 *
 * @param nameHash 32 bit hash of the function name.
 * @param p_ntdll Pointer to ntdll.
 * @param p_ssn Pointer to return SSN if found.
 * @param pp_function Pointer to return the syscall address if found.
 * @return BOOL TRUE if is a syscall and both SSN and syscall address was found.
 */
BOOL __callobf_loadSyscall(
    DWORD32 nameHash,
    PVOID p_ntdll,
    PUSHORT p_ssn,
    PVOID *pp_function);

/**
 * @brief Given a pointer, searches for the closest syscall ret instruction pair
 *        between p_lowBoundary and p_highBoundary.
 *
 * @param p_function Pointer to start search from.
 * @param p_lowBoundary Lowest possible address to search.
 * @param p_highBoundary Highest possible address to search.
 * @return PVOID Pointer to closest syscall ret, or NULL if not found.
 */
PVOID __callobf_getSyscallAddr(
    PVOID p_function,
    PVOID p_lowBoundary,
    PVOID p_highBoundary);

/**
 * @brief Check if the given function names matches the given hash in any of its
 *        Zw or Nt forms.
 *
 * @param p_functionName Pointer to function name to check.
 * @param hash Hash of the function we are comparing to.
 * @return BOOL TRUE if the hash matches in any of its Zw or Nt form
 */
BOOL __callobf_checkHashSyscallA(
    PCHAR p_functionName,
    DWORD32 hash);

#endif