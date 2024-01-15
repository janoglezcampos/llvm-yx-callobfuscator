TODO:
*  Fix create directories in makefile
*  Mby rename from FRAME_TABLE_ENTRY to something like FRAME_INFO
*  Rename everything realted to pushRbp to saveRbp
*  Group all globals, or somehow make it clear in the code were all globals are declared
*  EAF bypass
*  Opsec library loads on the call dispatcher
*  Put MIN_ADD_RSP_FRAME_SIZE to work
*  Somehow improve stackSpoofHelper.x64.asm readability
*  Add checks if its safe to delete intructions from parent
*  Handle invoke instructions and exception stuff
*  Handle indirect calls
*  Handle value reads