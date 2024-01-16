/**
 * @file commonUtils.h
 * @author Alejandro González (@httpyxel)
 * @brief Common functionality used across the project.
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

#ifndef _UTILS_H_
#define _UTILS_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"

// ==============================================================================
// ============================= MACRO DEFINITIONS ==============================

// Constants:
#define HASH_MULTIPLIER 37
#define RAND_SEED 123456

// Utils:
#define MID_POINT_ADDR(begin, end) ((end >= begin) ? ((PVOID)((DWORD_PTR)begin + (((DWORD_PTR)end - (DWORD_PTR)begin) / 2))) : 0)

// ==============================================================================
// ============================ PUBLIC  FUNCTIONS ===============================

/**
 * @brief Initializes the context using seed.
 *
 * @param p_ctx Context to be used for pseudo rng.
 * @param seed Seed to set for the context.
 * @return BOOL Success.
 */
BOOL __callobf_srand(PDWORD p_ctx, DWORD seed);

/**
 * @brief Generates next pesudo random number. Updates context for next call.
 *
 * @param p_ctx Context to be used for pseudo rng.
 * @return DWORD Next pseudo random number.
 */
DWORD __callobf_rand(PDWORD p_ctx);

/**
 * @brief Generates a ciclic sequence using lfsr.
 *
 * @param prevVal The last value used.
 * @return DWORD32 Next ciclic value.
 */
DWORD32 __callobf_lfsrXorShift32(DWORD32 prevVal);

/**
 * @brief Generates 32 bit value representing the string in ascii form.
 *
 * @param p_str String to be hashed.
 * @return UINT32 32 bit value representing the string.
 */
UINT32 __callobf_hashA(const PCHAR p_str);

/**
 * @brief Generates 32 bit value representing the string on its wide char form.
 *
 * @param p_str String to be hashed.
 * @return UINT32 32 bit value representing the string.
 */
UINT32 __callobf_hashW(const PWCHAR p_str);

/**
 * @brief Generates 32 bit value representing the string on its unicode form.
 *
 * @param p_str String to be hashed.
 * @return UINT32 32 bit value representing the string.
 */
UINT32 __callobf_hashU(const PUNICODE_STRING p_str);

/**
 * @brief Find the given pattern between startAddr and endAddr.
 *
 * @param startAddr Base address of the search space.
 * @param endAddr Top address of the search space.
 * @param bytes Patter to search for.
 * @param mask Mask to apply to the bytes beoing searched.
 * @param byteCount Size of the pattern.
 * @return PVOID Addres containing byteCount bytes matching the patter.
 */
PVOID __callobf_findBytes(
    const PVOID startAddr,
    const PVOID endAddr,
    const PBYTE bytes,
    const PBYTE mask,
    const DWORD byteCount);

/**
 * @brief Return base of the stack for the current thread.
 *
 * @return PVOID Stack base.
 */
PVOID __callobf_getStackBase();

/**
 * @brief Return top of the stack for the current thread.
 *
 * @return PVOID Stack limit.
 */
PVOID __callobf_getStackLimit();

#endif