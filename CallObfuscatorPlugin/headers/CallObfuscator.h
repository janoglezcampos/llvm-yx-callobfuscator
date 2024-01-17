/**
 * @file CallObfuscator.h
 * @author Alejandro González (@httpyxel)
 * @brief Includes the logic needed to transparently apply call obfucation at compile time.
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

#ifndef _CALL_OBFUSCATOR_H_
#define _CALL_OBFUSCATOR_H_

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace callobfuscator
{
    struct FunctionInfo
    {
        Function &function;
        StringRef dllName;
        bool isSyscall;
        unsigned int ssn;
        unsigned long modIndex = 0;
        unsigned long argCount = 0;
    };

    class CallObfuscator
    {
    private:
        Module &mod;    // We are ensured the ref (Module given by the pass manager) outlives any object of this class... probably, i guess O_o
        Module *tmpMod; // We are ensured the ref (Module given by the pass manager) outlives any object of this class... probably, i guess O_o

        bool __changedModule;
        bool __locked; // Once finalized, cant get more hooks
        vector<FunctionInfo> functionList;

        vector<StringRef> dllNames;
        vector<unsigned long> dllHashes;

        FunctionCallee callDispatcher;

    public:
        CallObfuscator(Module &module);

        /**
         * @brief Inserts given function to list of functions that will be hooked on finalize.
         *
         * @param functionInfo Information about the function to be hooked.
         * @return true Success.
         */
        bool addHook(const FunctionInfo &functionInfo);

        /**
         * @brief Applies applies hooks, inserts definitions, inserts tables, and briefly
         *        makes every change to the module. Prior to the execution of this function,
         *        the module stays unmodified.
         *
         * @return true Success.
         */
        bool finalize();

        /**
         * @return true Module changed.
         * @return false Module didnt change.
         */
        bool changedModule();

    private:
        bool insertTables(vector<StringRef> dllNames, vector<FunctionInfo> functionInfo);
        /**
         * @brief Inserts the definition of the call dispatcher, need to
         *        replace hooked functions.
         *
         * @return true Success.
         */
        bool insertCallDispatcherDef();

        /**
         * @brief Replaces the use of a function, by an equivalent use to the call dispatcher.
         *        NOTE: In LLVM, Uses represents reads, calls...
         *
         * @param use Use to be replaced.
         * @param functionTableIndex Index in the function table for the function to be replaced.
         * @return true Success.
         */
        bool replaceUse(Use &use, int functionTableIndex);

        /**
         * @brief Replaces a call instruction, by a call istruction to the call dispatcher.
         *
         * @param callInstruction Instruction to be replaced.
         * @param functionTableIndex Index in the function table for the function to be replaced.
         * @return true Success.
         */
        bool replaceCall(CallInst *callInstruction, int functionTableIndex);

        /**
         * @brief Creates an array of objects of type _FUNCTION_TABLE_ENTRY, and partially initializes it.
         *
         * @param ctx Module context.
         * @param pp_functionTableEntryStruct [OUT] Returns the array definition indicating entry type and size.
         * @param functionInfo Function information to partially initialize array.
         * @return Constant* Value containing the array.
         *         NOTE: In LLVM, Values represents functions, variables...
         */
        static Constant *createFunctionTableArray(LLVMContext &ctx, ArrayType **pp_functionTableEntryStruct, vector<FunctionInfo> functionInfo);

        /**
         * @brief Creates an array of objects of type _DLL_TABLE_ENTRY, and partially initializes it.
         *
         * @param ctx Module context.
         * @param pp_dllTableEntryStruct [OUT] Returns the array definition indicating entry type and size.
         * @param dlls Dll information to partially initialize table
         * @return Constant* Value containing the array.
         *         NOTE: In LLVM, Values represents functions, variables...
         */
        static Constant *createDllTableArray(LLVMContext &ctx, ArrayType **pp_dllTableEntryStruct, vector<Constant *> dlls);

        /**
         * @brief Creates an objects of type _FUNCTION_TABLE, and partially initializes it.
         *
         * @param ctx Module context.
         * @param pp_functionTableStruct [OUT] Returns object definition.
         * @param functionInfo Function information to partially initialize table.
         * @return Constant* Value containing the object.
         *         NOTE: In LLVM, Values represents functions, variables...
         */
        static Constant *createFunctionTable(LLVMContext &ctx, StructType **pp_functionTableStruct, vector<FunctionInfo> functionInfo);

        /**
         * @brief Creates an objects of type _DLL_TABLE, and partially initializes it.
         *
         * @param ctx Module context.
         * @param pp_dllTableEntryStruct [OUT] Returns object definition.
         * @param dlls Dll information to partially initialize table.
         * @return Constant* Value containing the object.
         *         NOTE: In LLVM, Values represents functions, variables...
         */
        static Constant *createDllTable(LLVMContext &ctx, StructType **pp_dllTableEntryStruct, vector<Constant *> dlls);
    };

}

#endif
