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

        bool addHook(const FunctionInfo &functionInfo);
        bool finalize();
        bool changedModule();

    private:
        bool insertTables(vector<StringRef> dllNames, vector<FunctionInfo> functionInfo);
        bool insertCallDispatcherDef();

        bool replaceUse(Use &use, int functionTableIndex);
        bool replaceCall(CallInst *callInstruction, int functionTableIndex);

        static Constant *createFunctionTableArray(LLVMContext &ctx, ArrayType **pp_functionTableEntryStruct, vector<FunctionInfo> functionInfo);
        static Constant *createFunctionTable(LLVMContext &ctx, StructType **pp_functionTableStruct, vector<FunctionInfo> functionInfo);

        static Constant *createDllTableArray(LLVMContext &ctx, ArrayType **pp_dllTableEntryStruct, vector<Constant *> dlls);
        static Constant *createDllTable(LLVMContext &ctx, StructType **pp_dllTableEntryStruct, vector<Constant *> dlls);
    };

}

#endif
