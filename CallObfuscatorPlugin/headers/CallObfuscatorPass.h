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
