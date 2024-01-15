set(CLANG_RELEASE_OPTIONS "-O3;-nostdlib;-ffunction-sections;-Wall;-Wextra;-fno-ident")
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

#llvm_map_components_to_libnames(llvm_libs core linker)
#target_link_libraries(LLVMCallDispatcher ${llvm_libs})
target_include_directories(CallObfuscatorHelpers PRIVATE headers)

set_target_properties(CallObfuscatorHelpers PROPERTIES PREFIX "lib")
set_target_properties(CallObfuscatorHelpers PROPERTIES CXX_STANDARD 17)

target_compile_options(CallObfuscatorHelpers PRIVATE "$<$<CONFIG:RELEASE>:$<$<COMPILE_LANGUAGE:C>:${CLANG_RELEASE_OPTIONS}>>")
target_compile_options(CallObfuscatorHelpers PRIVATE "$<$<CONFIG:RELEASE>:$<$<COMPILE_LANGUAGE:ASM_NASM>:${NASM_RELEASE_OPTIONS}>>")

install(TARGETS CallObfuscatorHelpers DESTINATION ${PROJECT_NAME}/plugin-helpers)