# LLVM-YX-CALLOBFUSCATOR

Transparently integrate stack spoofing and indirect syscalls if possible at compile time

## TODO:
* Rewrite this readme
* Document functions
* Add thanks
* Mby rename from FRAME_TABLE_ENTRY to something like FRAME_INFO
* Rename everything realted to pushRbp to saveRbp
* Group all globals, or somehow make it clear in the code were all globals are declared
* EAF bypass
* Opsec library loads on the call dispatcher
* Put MIN_ADD_RSP_FRAME_SIZE to work
* Somehow improve stackSpoofHelper.x64.asm readability
* Add checks if its safe to delete intructions from parent (the instruction that was replaced by a call)
* Handle invoke instructions and exception stuff (should not happen in C but...)
* Handle indirect calls
* Handle function address reads
