#ifndef _COMMON_H_
#define _COMMON_H_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// ==========================================================
// ================= MACRO DEFINITIONS ======================

// Hashes:
#define KERNELBASE_HASH 0xBD145C12
#define MESSAGEBOXA_HASH 0x35FC1883
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
 *  \param predicate The predicate to test. It must evaluate to
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

// Stolen from SilentMoonwalk project
#define BitVal(data, y) ((data >> y) & 1)

#define BitChainInfo(data) BitVal(data, 2)
#define BitUHandler(data) BitVal(data, 1)
#define BitEHandler(data) BitVal((data), 0)
#define Version(data) BitVal(data, 4) * 2 + BitVal(data, 3)

// ==========================================================
// ================ EXTERNAL FUNCTIONS ======================

extern int printf(const char *format, ...);

// ==========================================================

#endif