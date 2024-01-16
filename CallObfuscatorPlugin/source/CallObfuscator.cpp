/**
 * @file CallObfuscator.cpp
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

#include "CallObfuscator.h"

#include <typeinfo>

#include "llvm/Support/CommandLine.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Linker/Linker.h"

#include <ostream>
#include <iostream>
#include <format>
#include <string>
#include <iomanip>

using namespace std;
using namespace llvm;

namespace callobfuscator
{
    CallObfuscator::CallObfuscator(Module &module) : mod(module)
    {
        __changedModule = false;
        __locked = false;
        functionList = std::vector<callobfuscator::FunctionInfo>();

        tmpMod = new Module("__callobf_tmpMod", module.getContext());
        tmpMod->setDataLayout(module.getDataLayout());
    }

    unsigned long long hashStr(StringRef str)
    {
        unsigned long long h;
        char c;

        h = 0;
        for (char p : str)
        {
            c = (p >= 65 && p <= 90) ? p + 32 : p;
            h = 37 * h + c;
        }
        return h;
    }

    bool CallObfuscator::addHook(const FunctionInfo &functionInfo)
    {
        // TODO: Add checks <- What I was thinking about when wrote this? Ive no idea
        if (__locked)
            return false;

        FunctionInfo info = functionInfo;
        bool found = false;
        for (auto entry : functionList) // This hole looks so bad
        {
            if (entry.function.getName().equals(info.function.getName()))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            unsigned long dllHash = hashStr(info.dllName);
            unsigned long functionHash = hashStr(info.dllName);

            info.modIndex = find(dllHashes.begin(), dllHashes.end(), dllHash) - dllHashes.begin();

            if (info.modIndex >= dllHashes.size())
            {
                info.modIndex = dllHashes.size();
                dllNames.push_back(info.dllName);
                dllHashes.push_back(dllHash);
            }

            info.argCount = info.function.arg_size();

            functionList.push_back(info);
        }

        return true;
    }

    Constant *CallObfuscator::createFunctionTableArray(LLVMContext &ctx, ArrayType **pp_functionTableEntryStruct, vector<FunctionInfo> functionInfo)
    {

        // _FUNCTION_TABLE_ENTRY (24 bytes -> 64bits; 20 bytes -> 32bits)
        // > u_int32 hash
        // > u_int32 moduleIndex
        // > u_int32 argCount
        // > u_int32 ssn
        // > void *functionPtr
        StructType *p_functionTableEntryStruct = StructType::create(ctx, "_FUNCTION_TABLE_ENTRY");

        p_functionTableEntryStruct->setBody(
            {IntegerType::get(ctx, 32),
             IntegerType::get(ctx, 32),
             IntegerType::get(ctx, 32),
             IntegerType::get(ctx, 32),
             PointerType::get(ctx, 0)},
            true);

        vector<Constant *> functionTableEntries;
        for (FunctionInfo info : functionInfo)
        {
            functionTableEntries.push_back(ConstantStruct::get(
                p_functionTableEntryStruct,
                {ConstantInt::get(IntegerType::get(ctx, 32), hashStr(info.function.getName())),
                 ConstantInt::get(IntegerType::get(ctx, 32), info.modIndex),
                 ConstantInt::get(IntegerType::get(ctx, 32), info.argCount),
                 ConstantInt::get(IntegerType::get(ctx, 32), info.ssn),
                 ConstantPointerNull::get(PointerType::get(ctx, 0))}));
        }

        *pp_functionTableEntryStruct = ArrayType::get(p_functionTableEntryStruct, functionTableEntries.size());

        return ConstantArray::get(*pp_functionTableEntryStruct,
                                  ArrayRef(functionTableEntries)); // Im assuming get makes a copy, of the array, or at least of its contents, if not, this is passing a reference to a vector defined in the current scope :/
        // TODO: check this by modifying the vector after a call to get
    }

    Constant *CallObfuscator::createFunctionTable(LLVMContext &ctx, StructType **pp_functionTableStruct, vector<FunctionInfo> functionInfo)
    {
        // _FUNCTION_TABLE
        // > u_int32 entryCount
        // > u_int32 padding
        // > _FUNCTION_TABLE_ENTRY[] entries
        StructType *p_functionTableStruct = StructType::create(ctx, "_FUNCTION_TABLE");

        ArrayType *p_functionTableArrayDef;
        Constant *p_functionTableArray = createFunctionTableArray(ctx, &p_functionTableArrayDef, functionInfo);

        p_functionTableStruct->setBody(
            {IntegerType::get(ctx, 32),
             IntegerType::get(ctx, 32),
             p_functionTableArrayDef},
            true);

        *pp_functionTableStruct = p_functionTableStruct; // This doesnt seem rigth xd, expecting that the lifetime of p_functionTableStruct is enough :P

        return ConstantStruct::get(p_functionTableStruct,
                                   {ConstantInt::get(IntegerType::get(ctx, 32), p_functionTableArrayDef->getNumElements()),
                                    ConstantInt::get(IntegerType::get(ctx, 32), 0),
                                    p_functionTableArray});
    }

    Constant *CallObfuscator::createDllTableArray(LLVMContext &ctx, ArrayType **pp_dllTableEntryStruct, vector<Constant *> dlls)
    {
        vector<Constant *> dllTableEntries;
        // _DLL_TABLE_ENTRY  (16 bytes -> 64bits; 6 bytes-> 32bits)
        // > char* name
        // > void* handle
        StructType *p_dllTableEntryStruct = StructType::create(ctx, "_DLL_TABLE_ENTRY");

        p_dllTableEntryStruct->setBody(
            {PointerType::get(ctx, 0),
             PointerType::get(ctx, 0)},
            true);

        for (Constant *dllName : dlls)
        {
            dllTableEntries.push_back(ConstantStruct::get(
                p_dllTableEntryStruct,
                {dllName,
                 ConstantPointerNull::get(PointerType::get(ctx, 0))}));
        }

        *pp_dllTableEntryStruct = ArrayType::get(p_dllTableEntryStruct, dllTableEntries.size());

        return ConstantArray::get(*pp_dllTableEntryStruct,
                                  ArrayRef(dllTableEntries));
    }

    Constant *CallObfuscator::createDllTable(LLVMContext &ctx, StructType **pp_dllTableEntryStruct, vector<Constant *> dlls)
    {
        // _DLL_TABLE
        // > u_int32 entryCount
        // > u_int32 padding
        // > _DLL_TABLE_ENTRY[] entries
        StructType *p_dllTableStruct = StructType::create(ctx, "_DLL_TABLE");

        ArrayType *p_dllTableArrayDef;
        Constant *p_dllTableArray = createDllTableArray(ctx, &p_dllTableArrayDef, dlls);

        p_dllTableStruct->setBody(
            {IntegerType::get(ctx, 32),
             IntegerType::get(ctx, 32),
             p_dllTableArrayDef},
            true);

        *pp_dllTableEntryStruct = p_dllTableStruct; // This doesnt seem rigth xd, expecting that the lifetime of p_dllTableStruct is enough :P

        return ConstantStruct::get(p_dllTableStruct,
                                   {ConstantInt::get(IntegerType::get(ctx, 32), p_dllTableArrayDef->getNumElements()),
                                    ConstantInt::get(IntegerType::get(ctx, 32), 0),
                                    p_dllTableArray});
    }

    vector<Constant *> createDllNames(Module &M, vector<StringRef> dllNames)
    {
        LLVMContext &ctx = M.getContext();
        vector<Constant *> dllNamesAsCt;
        for (StringRef name : dllNames)
        {
            Constant *stringAsCt = ConstantDataArray::getString(ctx, name, true);
            Constant *stringGlobalAsCt = M.getOrInsertGlobal((".str.__callobfuscator." + name).str(), stringAsCt->getType()); // TODO: Check size
            GlobalVariable *stringGlobalAsGv = cast<GlobalVariable>(stringGlobalAsCt);

            stringGlobalAsGv->setConstant(true);
            stringGlobalAsGv->setLinkage(GlobalValue::InternalLinkage);
            stringGlobalAsGv->setInitializer(stringAsCt);
            dllNamesAsCt.push_back(stringGlobalAsCt);
        }

        return dllNamesAsCt;
    }

    bool CallObfuscator::insertTables(vector<StringRef> dllNames, vector<FunctionInfo> functionInfo)
    {
        LLVMContext &ctx = tmpMod->getContext();

        // ============================= Declare tables =============================
        StructType *p_functionTableDef;
        StructType *p_dllTableDef;

        if (!dllNames.size())
        {
            outs() << "[ERROR] No dlls to insert to tables, aborting";
            return false;
        }
        if (!functionInfo.size())
        {
            outs() << "[ERROR] No functions to insert to tables, aborting";
            return false;
        }

        vector<Constant *> dllNamesAsCt = createDllNames(*tmpMod, dllNames);

        Constant *p_functionTable = CallObfuscator::createFunctionTable(ctx, &p_functionTableDef, functionInfo);
        Constant *p_dllTable = CallObfuscator::createDllTable(ctx, &p_dllTableDef, dllNamesAsCt);

        // ================= Create global tables ===============

        GlobalVariable *functionTableGlobal = cast<GlobalVariable>(tmpMod->getOrInsertGlobal("__callobf_functionTable", p_functionTableDef));
        GlobalVariable *dllTableGlobal = cast<GlobalVariable>(tmpMod->getOrInsertGlobal("__callobf_dllTable", p_dllTableDef));

        functionTableGlobal->setConstant(false);
        functionTableGlobal->setLinkage(GlobalValue::ExternalLinkage);
        functionTableGlobal->setInitializer(p_functionTable);

        dllTableGlobal->setConstant(false);
        dllTableGlobal->setLinkage(GlobalValue::ExternalLinkage);
        dllTableGlobal->setInitializer(p_dllTable);

        return true;
    }

    bool CallObfuscator::replaceCall(CallInst *p_callInstruction, int functionTableIndex)
    {
        LLVMContext &ctx = mod.getContext();
        if (!p_callInstruction)
            return false;

        FunctionType *funcType = callDispatcher.getFunctionType();
        Value *funcValue = callDispatcher.getCallee();

        vector<Value *> funcArgsVec;
        funcArgsVec.push_back(ConstantInt::get(IntegerType::get(ctx, 32), functionTableIndex));
        p_callInstruction->getReturnedArgOperand();

        for (int i = 0; i < p_callInstruction->arg_size(); i++)
        {
            funcArgsVec.push_back(p_callInstruction->getArgOperand(i));
        }

        CallInst *p_callReplacement = CallInst::Create(funcType, funcValue, ArrayRef(funcArgsVec));
        p_callReplacement->insertBefore(p_callInstruction);

        int originalRetSize = 0;
        if (!p_callInstruction->getType()->isVoidTy())
            originalRetSize = mod.getDataLayout().getTypeSizeInBits(p_callInstruction->getType());

        int newRetSize = mod.getDataLayout().getTypeSizeInBits(p_callReplacement->getType());

        if (originalRetSize != newRetSize && originalRetSize != 0)
        {
            // originalRetSize should never be bigger than newRetSize
            assert(newRetSize > originalRetSize);

            TruncInst *p_truncInstruction = new TruncInst(p_callReplacement, p_callInstruction->getType());
            p_truncInstruction->insertAfter(p_callReplacement);
            p_callInstruction->replaceAllUsesWith(p_truncInstruction);
        }
        else
        {
            p_callInstruction->replaceAllUsesWith(p_callReplacement);
        }

        // TODO: Add checks if its safe to delete (It should ¿?)
        p_callInstruction->eraseFromParent();
        return true;
    }

    bool CallObfuscator::replaceUse(Use &use, int functionTableIndex)
    {
        outs() << "[INFO] Replacing use\n";
        // TODO: Handle invoke instructions and exception stuff (should not happen in C but...)
        // TODO: Handle indirect calls
        // TODO: Handle address reads
        if (Instruction *p_callInstruction = dyn_cast<Instruction>(use.getUser()))
        {
            if (p_callInstruction->getOpcode() == Instruction::OtherOps::Call)
                return replaceCall(cast<CallInst>(p_callInstruction), functionTableIndex);
        }
        return false;
    }

    bool CallObfuscator::insertCallDispatcherDef()
    {
        LLVMContext &ctx = mod.getContext();

        FunctionType *p_dispatcherType = FunctionType::get(
            IntegerType::get(ctx, 64),
            {IntegerType::get(ctx, 32)},
            true);

        if (mod.getFunction("__callobf_callDispatcher"))
            return false;

        callDispatcher = mod.getOrInsertFunction("__callobf_callDispatcher", p_dispatcherType);
        return true;
    }

    bool CallObfuscator::finalize()
    {
        if (__locked)
            return false;

        if (!insertTables(dllNames, functionList))
            return false;

        outs() << "[INFO] Inserted tables\n";
        __locked = true;
        if (Linker::linkModules(mod, std::unique_ptr<Module>(tmpMod)))
            return false;

        outs() << "[INFO] Linked modules\n";

        if (!insertCallDispatcherDef())
            return false;

        outs() << "[INFO] Inserted dispatcher definition\n";

        int functionTableIndex = 0;

        for (FunctionInfo &info : functionList)
        {
            for (auto &use : info.function.uses())
                if (!replaceUse(use, functionTableIndex++))
                {
                    outs() << "[ERROR] Unsuported use, code may break, aborting"
                           << "\n";

                    return false;
                }
        }

        __changedModule = true;
        return true;
    }

    bool CallObfuscator::changedModule()
    {
        return __changedModule;
    }
}