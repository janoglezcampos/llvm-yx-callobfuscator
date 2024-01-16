#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "common/common.h"

#ifdef DEBUG

#ifndef printf
extern int printf(const char *format, ...);
#endif

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
