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
