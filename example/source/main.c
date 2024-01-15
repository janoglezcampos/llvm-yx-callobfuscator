#include <windows.h>
#include "utils.h"
#include <stdio.h>

int main()
{
    printf("[INFO] Running call obfuscator test\n");
    PVOID addr = memoryTest();
    Sleep(1000);
    MessageBoxA(NULL, "Use this time to check the function stack.", "Break", 0);

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 0);
    printf("[INFO] Ended\n");
}