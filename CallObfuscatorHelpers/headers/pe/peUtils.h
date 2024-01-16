/**
 * @file peUtils.h
 * @author Alejandro González (@httpyxel)
 * @brief Utilities to manipulate and work with in-memory PEs.
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

#ifndef _PE_UTILS_H_
#define _PE_UTILS_H_

#include "common/common.h"
#include "common/wintypes/typedefs.h"

#ifndef NtCurrentTeb
extern PTEB NtCurrentTeb(void);
#endif

// ==============================================================================
// ============================ PUBLIC  FUNCTIONS ===============================

/**
 * @brief Finds pointer to a module loaded in memory from its name.
 *
 * @param p_moduleName Module name.
 * @return PVOID Pointer to module base if found, else NULL.
 */
PVOID __callobf_getModuleAddrA(const PCHAR p_moduleName);

/**
 * @brief Finds pointer to a module loaded in memory from its name in wide char form.
 *
 * @param p_moduleName Module name.
 * @return PVOID Pointer to module base if found, else NULL.
 */
PVOID __callobf_getModuleAddrW(const PWCHAR p_moduleName);

/**
 * @brief Finds pointer to a module loaded in memory from its name hash.
 *
 * @param moduleHash Module hash.
 * @return PVOID Pointer to module base if found, else NULL.
 */
PVOID __callobf_getModuleAddrH(const UINT32 moduleHash);

/**
 * @brief Given a module pointer, find the given function name between its
 *        exported functions, then  returns its pointer.
 *
 * @param p_module Pointer to module to search in.
 * @param p_functionName Function name.
 * @return PVOID Pointer to function if found, else NULL.
 */
PVOID __callobf_getFunctionAddrA(
    const PVOID p_module,
    const PCHAR p_functionName);

/**
 * @brief Given a module pointer, find the given function name in wide char form between its
 *        exported functions, then  returns its pointer.
 *
 * @param p_module Pointer to module to search in.
 * @param p_functionName Function name.
 * @return PVOID Pointer to function if found, else NULL.
 */
PVOID __callobf_getFunctionAddrW(
    const PVOID p_module,
    const PWCHAR p_functionName);

/**
 * @brief Given a module pointer, find the given function hash between its exported functions,
 *        then  returns its pointer.
 *
 * @param p_module Pointer to module to search in.
 * @param funtionHash Function hash.
 * @return PVOID Pointer to function if found, else NULL.
 */
PVOID __callobf_getFunctionAddrH(
    const PVOID p_module,
    const UINT32 funtionHash);

/**
 * @brief Returns the exception directory address if any.
 *
 * @param p_module Pointer to module to return the exception dir address from.
 * @param p_size Retruns size of the exception directory.
 * @return PVOID Pointer exception directory, or NULL.
 */
PVOID __callobf_getExceptionDirectoryAddress(
    const PVOID p_module,
    PDWORD p_size);

/**
 * @brief Gets boundaries for the first executable section found in a module.
 *
 * @param p_module Pointer to module to search in.
 * @param pp_base Returns pointer to base of code section.
 * @param pp_top Returns pointer to top of code section.
 * @return BOOL Success.
 */
BOOL __callobf_getCodeBoundaries(
    PVOID p_module,
    PVOID *pp_base,
    PVOID *pp_top);

#endif