/**
 * @file CallObfucatorPass.cpp
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

#include "CallObfuscatorPass.h"
#include "CallObfuscator.h"

#include <typeinfo>

#include "llvm/Support/CommandLine.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace std;
using namespace llvm;
using namespace json;

namespace callobfuscatorpass
{
    bool CallObfuscatorPass::readConfig(Object &jsonObj)
    {

        StringRef config = getenv(LLVM_CALL_OBF_CONFIG_PATH);

        if (config.empty())
        {
            errs() << "[ERROR] " LLVM_CALL_OBF_CONFIG_PATH " env variable not set"
                   << "\n";
            return false;
        }

        config = config.trim(" \t\n\v\f\r\"");

        ErrorOr<unique_ptr<MemoryBuffer>> result =
            MemoryBuffer::getFile(config);

        error_code ec = result.getError();

        if (ec)
        {
            errs() << "[ERROR] Config file could not be read: " << ec.message() << "\n";
            return false;
        }

        Expected<json::Value> ParseResult =
            parse(result.get().get()->getBuffer());

        if (Error E = ParseResult.takeError())
        {
            errs() << "[ERROR] Config file could not be parsed\n";
            errs() << E << "\n";
            consumeError(std::move(E));
            return false;
        }

        outs() << "[INFO] : Using config: " << config << "\n";

        jsonObj = *ParseResult.get().getAsObject();
        return true;
    }

    bool CallObfuscatorPass::isFunctionHooked(StringRef argFunctionName, StringRef &argDllName)
    {
        if (fileMalformed)
            return false;

        const json::Array *dllHooks = jsonObj.getArray(DLL_HOOKS_KEY);

        if (!dllHooks)
        {
            errs() << "[ERROR] Config file malformed, cant find \"" DLL_HOOKS_KEY "\" key\n";
            fileMalformed = true;
            return false;
        }
        int entryCount = 0; // Used for error reporting
        for (auto &entry : *dllHooks)
        {
            const Object *dllInfo = entry.getAsObject();

            if (!dllInfo)
            {
                errs() << "[ERROR] Config file malformed at entry " << entryCount << ", not an object\n";
                fileMalformed = true;
                return false;
            }

            std::optional<llvm::StringRef> dllName = (*dllInfo).getString(DLL_NAME_KEY);
            if (!dllName.has_value())
            {
                errs() << "[ERROR] Config file malformed at entry " << entryCount << ", cant find \"" DLL_NAME_KEY "\" key, or is not a string\n";
                fileMalformed = true;
                return false;
            }

            const json::Array *functionNames = (*dllInfo).getArray(FUNCTION_HOOKS_KEY);
            if (!functionNames)
            {
                errs() << "[ERROR] Config file malformed at entry " << entryCount << ", cant find \"" FUNCTION_HOOKS_KEY "\" key, or is not a list\n";
                fileMalformed = true;
                return false;
            }

            for (auto &functionEntry : *functionNames)
            {
                std::optional<llvm::StringRef> functionName = functionEntry.getAsString();
                if (!functionName.has_value())
                {
                    errs() << "[ERROR] Config file malformed at entry " << entryCount << ", function list contains errors\n";
                    fileMalformed = true;
                    return false;
                }

                if (functionName.value().equals(argFunctionName))
                {

                    argDllName = dllName.value();
                    return true;
                }
            }

            entryCount++;
        }
        return false;
    }

    // CallObfuscatorPass implementations:
    PreservedAnalyses CallObfuscatorPass::run(Module &M,
                                              ModuleAnalysisManager &AM)
    {
        auto &Ctx = M.getContext();
        if (!configLoaded)
        {
            configLoaded = true; // No matter the result, try only once
            bool result = readConfig(jsonObj);
            if (!result)
                return PreservedAnalyses::all();
        }

        outs() << "[INFO] : Analyzing module: " << M.getName() << "\n";

        callobfuscator::CallObfuscator obf = callobfuscator::CallObfuscator(M);
        for (Function &F : M)
        {
            StringRef dllName;
            if (isFunctionHooked(F.getName(), dllName))
            {
                if (!obf.addHook({F, dllName, false, 0}))
                {
                    outs() << "[INFO] : Something went wrong while preparing the hooks... "
                           << "\n";
                    return PreservedAnalyses::all();
                }
            }
        }

        if (!obf.finalize())
            outs() << "[ERROR] Something went wrong\n";

        // Im not really sure wich passes should invalidate, so better to rerun all,
        // but since we run it as a single pass with opt, doesnt really matter.
        if (obf.changedModule())
            return PreservedAnalyses::none();

        outs() << "[INFO] : Module not modified"
               << "\n";

        return PreservedAnalyses::all();
    }
}