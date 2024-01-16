/**
 * @file debug.h
 * @author Alejandro González (@httpyxel)
 * @brief Debug definitions.
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

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "common/common.h"

#ifdef DEBUG

// ==============================================================================
// =========================== EXTERNAL FUNCTIONS ===============================
#ifndef printf
extern int printf(const char *format, ...);
#endif

// ==============================================================================
// ============================= MACRO DEFINITIONS ==============================

#define DEBUG_PRINT(fmt, ...)                                                                                \
    do                                                                                                       \
    {                                                                                                        \
        printf("[+] DEBUG: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__); \
    } while (0);

#define DEBUG_PRINT_PTR(ptr) DEBUG_PRINT(p_agent, "PTR %s: %p", #ptr, ptr);
#define BREAKPOINT      \
    do                  \
    {                   \
        __debugbreak(); \
    } while (0);
#else
#define DEBUG_PRINT(...) \
    do                   \
    {                    \
    } while (0);
#define DEBUG_PRINT_PTR(...) \
    do                       \
    {                        \
    } while (0);
#define BREAKPOINT \
    do             \
    {              \
    } while (0);
#endif

#endif
