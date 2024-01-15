cmake_minimum_required(VERSION 3.20.0)

# I know this is not a "best practice", but CallDispatcher requires Clang to be compiled because of the use of builtins, also, 
# this only works for windows, and x64, so I dont see a problem on forcing some options.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
#TODO: Generate errors on bad arch/SO

project(llvm-yx-callobfuscator LANGUAGES C CXX ASM_NASM)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Using install prefix ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})



add_subdirectory(CallObfuscatorPlugin)
add_subdirectory(CallObfuscatorHelpers)