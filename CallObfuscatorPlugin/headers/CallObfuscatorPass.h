/**
 * @file CallObfucatorPass.h
 * @author Alejandro González (@httpyxel)
 * @brief Initalization and managment of the obfuscator pass.
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

#ifndef _CALL_OBFUSCATOR_PASS_H_
#define _CALL_OBFUSCATOR_PASS_H_

#include "llvm/IR/PassManager.h"
#include "llvm/Support/JSON.h"

#define LLVM_CALL_OBF_CONFIG_PATH "LLVM_OBF_FUNCTIONS"

#define DLL_HOOKS_KEY "dll_hooks"
#define DLL_NAME_KEY "dll_name"
#define FUNCTION_HOOKS_KEY "hooked_functions"

#define DO_PRAGMA(x) _Pragma(#x)
#define NOWARN(warnoption, ...)                  \
    DO_PRAGMA(GCC diagnostic push)               \
    DO_PRAGMA(GCC diagnostic ignored warnoption) \
    __VA_ARGS__                                  \
    DO_PRAGMA(GCC diagnostic pop)

using namespace llvm;
using namespace json;

namespace callobfuscatorpass
{
    class CallObfuscatorPass : public PassInfoMixin<CallObfuscatorPass>
    {
    public:
        /**
         * @brief Function invoked by opt for each module given.
         *
         * @param M Module being optimized.
         * @param AM Analisys Manager for current optimization
         * @return PreservedAnalyses Wich analyses can be preserved.
         */
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

    private:
        bool configLoaded;
        bool fileMalformed;
        Object jsonConfig;

        /**
         * @brief Check if the given function is indicated as hooked in the configuration file.
         *
         * @param argFunctionName Name of the function to check.
         * @param argDllName [OUT] Returns dll name, if hooked.
         * @return true Function is hooked.
         */
        bool isFunctionHooked(StringRef argFunctionName, StringRef &argDllName);

        /**
         * @brief Reads config file from LLVM_OBF_FUNCTIONS env variable.
         *
         * @param jsonConfig [OUT] Returns json config.
         * @return true Success.
         */
        bool readConfig(Object &jsonConfig);
    };

}

#endif
