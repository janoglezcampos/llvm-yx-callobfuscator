# LLVM-YX-CALLOBFUSCATOR


LLVM plugin to transparently apply stack spoofing and indirect syscalls to Windows x64 native calls at compile time.


## "I've 5 mins, what is this?"
This project is a plugin meant to be used with [opt](https://llvm.org/docs/CommandGuide/opt.html), the LLVM optimizer. Opt will use the pass included in this plugin to hook calls to Windows functions based on a config file and point those calls to a single function, which, given an ID identifying the function to be called, will apply dynamic stack obfuscation, and if the function is a syscall stub, will invoke it using "indirect syscalling".


**Usage brief**: Set up a config file indicating the functions to be hooked, write your C code without caring about Windows function calls, compile with clang to generate .ir files, give them to opt along with this plugin, opt hooks functions, llc compiles the .ir to a.obj, and ld links it to an executable that automatically obfuscates function calls.

## Table of Contents
> * [Setup](#setup)
> * [Usage and example](#usage-and-example)
> * [Developer guide](#developer-guide)
>   * [File distribution](#file-distribution)
>   * [How the pass works](#how-the-pass-works)
>   * [How the dispatching system works](#how-the-dispatching-system-works)
> * [Thanks](#thanks)
> * [TODO](#todo)

## Setup
This instructions are written for Windows, but it should be possible to setup this environment in Linux easily (still output executables can only be built for Windows x64). Tested with LLVM 16.x and 17.x.

All the commands described in the following steps are supossed to be used in an MSYS2 terminal ie they have Linux format.

* **Dependencies**:

    To be able to compile this project, we mainly need 2 things: LLVM (libraries and headers, along with some tools) and CMAKE.

    LLVM libraries and tools can be either compiled from source, downloaded from the LLVM releases, or installed through MSYS2 (using pacman). To make it easier, here we will use MSYS2. We also need to set up CMAKE and our build tools. Clang is required to compile the helpers library. For the linker, we don't care too much. Lastly, as a generator, I prefer Ninja, but make, nmake or msbuild will work.

    Everything listed above can be installed with pacman by using MSYS2.

    * Download and install [MSYS2](https://www.msys2.org/).
    * Launch an MSYS2 Mingw64 terminal and install the following packages:
    * Install LLVM: ```pacman -S mingw-w64-x86_64-llvm```
    * Install Clang: ```pacman -S mingw-w64-x86_64-clang```
    * Install Nasm: ```pacman -S mingw-w64-x86_64-nasm```
    * Install Cmake: ```pacman -S mingw-w64-x86_64-cmake```
    * Install Ninja: ```pacman -S mingw-w64-x86_64-ninja```
    * Install Git: ```pacman -S git```

    Restart the MSYS2 terminal, It may help with env variables, and gives luck for the following building ritual.

* **Building**:
  
    First, clone this project, pretty obvious:

        git clone https://github.com/janoglezcampos/llvm-yx-callobfuscator


    To build this project, we will be using CMAKE. Because I find it convenient, I use VScode with the CMake extension. You can find my VSCODE config at ```.vscode_conf/setting.json```

    In any other case, launch an MSYS2 Mingw64 terminal and do exactly what I say (without checking what any of the commands Im giving you will do to your beloved machine):

    Go to the root directory of this repo:

        cd <whatever>/llvm-yx-obfuscator

    Create a build directory and change the directory to it:

        mkdir build; cd build

    Choose an install location. I recommend doing this so it will be easier to either get the files in the right place or just be able to pick them up easily. Also, the [usage](#usage-and-example) section will use this folder as the relative folder for accessing these files when needed, so if you add it to the PATH, the commands will work right away. I use  ```C:/Users/<my-user>/llvm-plugins```, but creating a folder in the project directory called ```install```, side by side with ```build``` will do. This folder will not be edited if you don't run the install command, but you will have to go get the files in the build folder.

    Configure the project; here you specify the installation folder. ```DCMAKE_BUILD_TYPE``` will set the default mode: Debug, Release or MinSizeRel. Depending on which generator you are using, you are going to be able to change this later or not (means you will have to reconfigure or not). I use Nija as generator, but you can use any other.

        cmake -G Ninja -DCMAKE_INSTALL_PREFIX="<path_to_install_folder>" -DCMAKE_BUILD_TYPE=Release ./..

    Build the project; optionally choose mode with ```—-config Release``` (if your generator lets you):

        cmake --build .

    Move the generated files to the install directory you chose before. If you don't want to use the install "feature" of CMake, just get the files (```libCallObfuscatorHelpers.a``` and ```CallObfuscatorPlugin.dll```) from the build directory and put them in a place you remember; you will need them.

        cmake --build . --target install

    At this point, you should have two files:
    * ```libCallObfuscatorHelpers.a```: A C library that includes all the logic that needs to be executed at runtime.  
    * ```CallObfuscatorPlugin.dll```:  The actual plugin containing the pass, will be given as an offer to opt.

    Once all this is done I like to add the path I used to install the plugin to the user path, so it is easier to import it after.
    To do this, add the following line to ```~/.bash_profile``` if exists, if not, add it to ```~/.profile```. Also, modify the library path, so you wont need to specify the path to helpers everytime you link.

        export PATH=$PATH:"<path_to_install_folder>"
        export LIBRARY_PATH=$ LIBRARY_PATH:"<path_to_install_folder>/llvm-yx-callobfuscator/plugin-helpers"

    Remember the path format changes from Windows, where C: becomes /c/ (because PATH separator is ```:```), for example, in my case would:

        export PATH=$PATH:"/c/Users/<user>/llvm-plugins"
        export LIBRARY_PATH=$LIBRARY_PATH:"/c/Users/<user>/llvm-plugins/llvm-yx-callobfuscator/plugin-helpers"
     

## Usage and example
First of all, we need to set up our configuration file. In this section, we will be using the config file found in the ```./example``` folder. You can add any number of functions to the file, and the functions do not need to appear in the program.

The plugin will get the path to the config file from an environment variable called ```LLVM_OBF_FUNCTIONS```. You can either add it along with all the other environment variables for the user, the system, or just set it up for the current terminal. You can also set it from a makefile, if using one. To set the example config for the current terminal:

        export LLVM_OBF_FUNCTIONS=<absolute path to callobfuscator.conf>

Now it is time to run the pass. A more detailed explanation about every step can be found [here](https://github.com/janoglezcampos/llvm-pass-plugin-skeleton?tab=readme-ov-file#running-you-pass).

* Go inside the example folder and create a build folder; inside, create 2 folders: irs and objs.

        cd example; mkdir build; mkdir build/irs; mkdir build/objs

### NOTE: Any path in MYSYS2 must be written using / and not \

* Compile the C files to LLVM-IR:

        clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ./source/utils.c -Iheaders -o ./build/irs/utils.ll

        clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ./source/main.c -Iheaders -o ./build/irs/main.ll

* Merge all files:

        llvm-link ./build/irs/main.ll ./build/irs/utils.ll -S -o ./build/irs/example.ll

* Run obfusction pass:

        opt -S -load-pass-plugin="<path to the pass dll>" -passes="callobfuscator-pass" ./build/irs/example.ll -o ./build/irs/example.obf.ll
    
    If added install path to user or system path, then:

        opt -S -load-pass-plugin="llvm-yx-callobfuscator/CallObfuscatorPlugin.dll" -passes="callobfuscator-pass" ./build/irs/example.ll -o ./build/irs/example.obf.ll

* Run optimization passes:

        opt -S -O3 ./build/irs/example.obf.ll -o ./build/irs/example.op.ll

* Compile to windows x86_64 assembly:
  
        llc --mtriple=x86_64-pc-windows-msvc -filetype=obj ./build/irs/example.op.ll -o ./build/objs/example.obj

* Link:

        clang ./build/objs/example.obj -o ./build/example.exe -L"<path to folder containing libCallObfuscatorHelpers.a> -lCallObfuscatorHelpers 

    If added helpers path to LIBRARY_PATH then you can ommit the ```-L``` option.

        clang ./build/objs/example.obj -o ./build/example.exe -lCallObfuscatorHelpers

Now you should have ```./build/example.exe```, the final executable.

In case you are thinking that those are a lot of commands, well, they are always "the same", so writing makefiles helps, Im leaving a makefile example inside the example folder to compile the same code as before.

## Developer guide
* ### File distribution
    ---
    The code is always divided into two folders, one called headers, for definitions and macros mainly, and the other called source, containing the actual source code. For every source code file, there is a header file matching the relative path to the source folder. Documentation for functions is always found at headers files.

    You will find two source codebases in this project:

  * **CallObfuscatorPlugin**: The actual plugin, written in C++, that will be compiled and linked to a dll.
      * **CallObfuscator**: Includes the logic to transparently apply call obfucation at compile time.
    * **CallObfuscatorPass**: Initalization and management of the obfuscator pass.
    * **CallObfuscatorPluginRegister**: Plugin registration.


  * **CallObfuscatorHelpers**: A C library that includes all the logic that needs to be executed at runtime.
    * **common**: Common functionality that is used across the project.
    * **pe**: Utilities to manipulate and work with in-memory PEs.
    * **callDispatcher**: Functionality to invoke Windows native functions applying obfuscation.
    * **stackSpoof**: Functionality to apply dynamic stack spoofing in Windows x64 environments.
    * **syscalls**: Utilities to work with Windows x64 syscalls.


* ### How the pass works
    ---
    Knoledge about common terms like hooks, register, stack... is assumed.
    This is not an in-depth guide, just enough to get you throw the execution flow.

    First, we go through every defined function in the code; if any of them is found in the config file, we store it. Once we find all the functions that will be obfuscated, we create two tables:
    * ```__callobf_dllTable```: This contains all required dlls for obfuscated functions; each dll has an ID, which is its index in the table.
    * ```__callobf_functionTable```: This contains all obfuscated functions and information about which dll contains them, the number of arguments of the function, if it is a syscall, etc.

    At compile time, this tables will be partially initialized, but the only value we need at this moment is the function ID (its index in the function table).

    After building the tables, we find every call to the obfuscated functions; for each of them, replace the call by a call to ```__callobf_callDispatcher```, and pass the ID as the first argument, then pass all the other function arguments.

    ```__callobf_callDispatcher``` is defined as ```PVOID __callobf_callDispatcher(DWORD32 index, ...)```. It will get all the info it needs from the function table by using the ID (index) in the first argument.

    A function has its entry partially initialized until it is called; at that moment, ```__callobf_callDispatcher``` will store all the required information to call and obfuscate the function and pass the other arguments to the function being called.

* ### How the dispatching system works
    ---
    The dispatching system starts by initializing the entry in the function table.
    * Get the dll from the dll table and load it if needed.
    * Find the function in the IAT and store the address in the function table.
    * Find if the call is a syscall; if it is, get the ssn and store it in the function table.
  
    If not already done, initialize the frame table (```__callobf_globalFrameTable```), used to cache posible frames and gadgets that will be used to build the obfuscated stack. The obfuscation method is the same as explained [here](https://klezvirus.github.io/RedTeaming/AV_Evasion/StackSpoofing/), still an outstanding job.

    After the the function is loaded and the frame table is initialized:

    * Build a fake stack using the values stored in the frame table and store it over the current stack pointer.
    * Update the ciclic value used to pick which values are used for building the stack.
    * Move arguments to their right place.
    * Change rsp to match the start of the fake stack.
    * If syscall, set ssn in rax.
    * If syscall, set r10 to hold the first argument.
    * Jump to the function or syscall instruction.

## Thanks
To Arash Parsa, aka [waldoirc](https://twitter.com/waldoirc), Athanasios Tserpelis, aka [trickster0](https://twitter.com/trickster012) and Alessandro Magnosi, aka [klezVirus](https://twitter.com/klezVirus) because of [SilentMoonwalk](https://klezvirus.github.io/RedTeaming/AV_Evasion/StackSpoofing/)

---
> ## TODO:
> This includes things that I really dont want to forget, but more stuff could be added here. Not by now
>### Docs/formatting:
>* Somehow improve stackSpoofHelper.x64.asm readability.
>  
>### Opsec:
>* EAF bypass.
>* Encript strings that cant be hashed.
>
>### General quality:
>* Group all globals, or somehow make it clear in the code where all globals are declared.
>* Put MIN_ADD_RSP_FRAME_SIZE to work.
>* Optionally validate config file entries against local dlls.
>* Optionally return load errors through messagebox pop ups, similarly to what ms does with CRTs.
>
>### Functionality:
>* Handle invoke instructions and exception stuff (should not happen in C but...)
>* Handle indirect calls
>* Handle function address reads (In fact indirect calls always start with a value read, so they are the same case)
