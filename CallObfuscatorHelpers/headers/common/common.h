/**
 * @file common.h
 * @author Alejandro González (@httpyxel)
 * @brief Common definitions used across the project.
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

#ifndef _COMMON_H_
#define _COMMON_H_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// ==============================================================================
// ============================= MACRO DEFINITIONS ==============================

// Some hashes:

#define NTDLL_HASH 0xE59C2120
#define KERNEL32_HASH 0x70B456D4
#define KERNELBASE_HASH 0xBD145C12

#define MESSAGEBOXA_HASH 0x35FC1883
#define LOADLIBRARYA_HASH 0x15E7E6C2
#define BASETRHEADINITTHUNK_HASH 0xA2EC03A5
#define RTLUSERTHREADSTART_HASH 0x362F51AF

#define PE_MAGIC 0x5A4D

// Constants:
#define RET 0xc3 // One byte, no conversion needed

// Utils:
#define POSITIVE_OR_ZERO(a) ((signed)a > 0 ? a : 0)
#define UNUSED(x) (void)(x)
#define COUNT(x) (sizeof(x) / sizeof(*x))

/** A compile time assertion check.
 *
 *  Validate at compile time that the predicate is true without
 *  generating code. This can be used at any point in a source file
 *  where typedef is legal.
 *
 *  @param predicate The predicate to test. It must evaluate to
 *  something that can be coerced to a normal C boolean.
 */
#define CASSERT(predicate) _impl_CASSERT_LINE(predicate, __LINE__, __FILE__)

#define _impl_PASTE(a, b) a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    NOWARN("-Wunused-local-typedef",              \
           typedef char _impl_PASTE(assertion_failed_##file##_, line)[2 * !!(predicate)-1];)

#define DO_PRAGMA(x) _Pragma(#x)
#define NOWARN(warnoption, ...)                  \
    DO_PRAGMA(GCC diagnostic push)               \
    DO_PRAGMA(GCC diagnostic ignored warnoption) \
    __VA_ARGS__                                  \
    DO_PRAGMA(GCC diagnostic pop)

#define BitVal(data, y) ((data >> y) & 1)

#define BitChainInfo(data) BitVal(data, 2)
#define BitUHandler(data) BitVal(data, 1)
#define BitEHandler(data) BitVal((data), 0)
#define Version(data) BitVal(data, 4) * 2 + BitVal(data, 3)

#define DISABLE_OPTIMIZATIONS __attribute__((optnone))

#endif