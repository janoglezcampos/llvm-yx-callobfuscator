add_library(CallObfuscatorPlugin MODULE 
            source/CallObfuscatorPass.cpp
            source/CallObfuscatorPluginRegister.cpp
            source/CallObfuscator.cpp)

llvm_map_components_to_libnames(llvm_libs core linker)
target_link_libraries(CallObfuscatorPlugin ${llvm_libs})
target_include_directories(CallObfuscatorPlugin PRIVATE headers)

set_target_properties(CallObfuscatorPlugin PROPERTIES PREFIX "")
set_target_properties(CallObfuscatorPlugin PROPERTIES CXX_STANDARD 17)

install(TARGETS CallObfuscatorPlugin DESTINATION ${PROJECT_NAME})
