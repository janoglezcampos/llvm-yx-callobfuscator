#include <windows.h>
#include "utils.h"
#include <stdio.h>

int main()
{
    printf("[INFO] Running call obfuscator test\n");

    PVOID p_kernel32 = LoadLibraryA("kernel32");
    printf("[INFO] kernel32 addr: %p\n", p_kernel32);
    PVOID addr = memoryTest();
    Sleep(1000);
    MessageBoxA(NULL, "Use this time to check the function stack.", "Break", 0);

    printf("[INFO] Ended\n");
}