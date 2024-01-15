/**
 * @file CallObfuscatorPluginRegister.cpp
 * @author Alejandro González (@httpyxel)
 * @brief Plugin registration.
 * @version 0.1
 * @date 2024-01-14
 *
 * @copyright
 *   Copyright (C) 2024  Alejandro González
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

#include "CallObfuscatorPass.h"

__declspec(dllexport) extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
    llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION,
        "CallObfuscatorPlugin", "v0.1",
        [](PassBuilder &PB)
        {
            // Allows to run the pass alone, only works with opt.exe
            // Enables: opt -load-pass-plugin="<whatever>/LLVMBasePlugin.dll" -passes="base-plugin-pass"
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if(Name == "callobfuscator-pass"){
                        MPM.addPass(callobfuscatorpass::CallObfuscatorPass());
                        return true;
                    }
                    return false; });

            // This is on test...
            // Allows to run the pass as part of the defaults optimization passes
            // Enables: clang -O0 -Xclang -disable-O0-optnone -fpass-plugin="<whatever>/LLVMBasePlugin.dll" ...
            // Enables: opt -O0 -load-pass-plugin="<whatever>\LLVMBasePlugin.dll" ...

            // This extension point allows adding optimization passes after most of the
            // main optimizations, but before the last cleanup-ish optimizations.

            /*PB.registerScalarOptimizerLateEPCallback(
                [](FunctionPassManager &FPM, OptimizationLevel opt)
                {
                    FPM.addPass(baseplugin::CallObfuscatorPass());
                }); */
            PB.registerFullLinkTimeOptimizationEarlyEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel opt)
                {
                    MPM.addPass(callobfuscatorpass::CallObfuscatorPass());
                });
        }};
}