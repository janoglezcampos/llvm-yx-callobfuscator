set(CLANG_DEBUG_OPTIONS "-D__CALLOBF_DEBUG")
# This migth be wrong, I only want to apply this options to the target defined here, 
# but Im applying them to the hole project...
set(CLANG_RELEASE_OPTIONS "-O3 -nostdlib -ffunction-sections -Wall -Wextra -fno-ident")

set(NASM_DEBUG_OPTIONS "")
set(NASM_RELEASE_OPTIONS "")

#find ./source -type f -name "*.c"
#find ./source -type f -name "*.asm"
add_library(CallObfuscatorHelpers STATIC 
            source/callDispatcher/callDispatcher.c
            source/common/commonUtils.c
            source/pe/peUtils.c
            source/pe/unwind/unwindUtils.c
            source/stackSpoof/stackSpoof.c
            source/syscalls/syscalls.c
            source/callDispatcher/asm/callDispatcher.x64.asm
            source/stackSpoof/asm/stackSpoofHelper.x64.asm
            )

target_include_directories(CallObfuscatorHelpers PRIVATE headers)

set_target_properties(CallObfuscatorHelpers PROPERTIES PREFIX "lib")
set_target_properties(CallObfuscatorHelpers PROPERTIES CXX_STANDARD 17)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CLANG_DEBUG_OPTIONS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CLANG_RELEASE_OPTIONS}")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${CLANG_RELEASE_OPTIONS}")

set(CMAKE_NASM_FLAGS_DEBUG "${CMAKE_NASM_FLAGS_DEBUG} ${NASM_DEBUG_OPTIONS}")
set(CMAKE_NASM_FLAGS_RELEASE "${CMAKE_NASM_FLAGS_RELEASE} ${NASM_RELEASE_OPTIONS}")
set(CMAKE_NASM_FLAGS_MINSIZEREL "${CMAKE_NASM_FLAGS_MINSIZEREL} ${NASM_RELEASE_OPTIONS}")

install(TARGETS CallObfuscatorHelpers DESTINATION ${PROJECT_NAME}/plugin-helpers)