#include <windows.h>
#include <stdio.h>
#include "utils.h"

PVOID memoryTest()
{
    PVOID p_baseAddress = NULL;
    SIZE_T requiredSize = 0x1100;

    NTSTATUS status = NtAllocateVirtualMemory((HANDLE)-1, &p_baseAddress, 0, &requiredSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (status)
    {
        printf("[ERROR] NtAllocateVirtualMemory status: 0x%lX\n", status);
    }

    printf("[INFO] Allocated 0x%llX bytes at %p\n", requiredSize, p_baseAddress);
    return p_baseAddress;
}