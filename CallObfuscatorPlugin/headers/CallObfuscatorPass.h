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
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

    private:
        bool configLoaded;
        bool fileMalformed;
        Object jsonObj;

        // bool runOnBasicBlock(BasicBlock &B);
        bool readConfig(Object &jsonObj);
        bool isFunctionHooked(StringRef functionName, StringRef &dllName);
    };

}

#endif
