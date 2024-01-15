# LLVM-YX-CALLOBFUSCATOR

LLVM plugin to transparently apply stack spoofing and indirect syscalls if possible to native windows x64 calls at compile time.

## "Ive 5 mins, what is this?"
This project is a plugin meant to be used with [opt](https://llvm.org/docs/CommandGuide/opt.html), the LLVM optimizer. Opt will use the pass included in this plugin to hook calls to windows functions based on  a config file, and point those calls to a single function, which given an ID identifying the function to be called, will apply dynamic stack obfuscation, and if the function is a syscall stub, will call it throw indirect syscalling.

**Brief**: Set up a config file indicating the functions to be hooked, write your code without caring about windows functions calls, compile with clang to generate .ir files, give them to opt along this plugin, opt hooks functions, llc compiles the .ir to a .obj and ld links it to an executable that automatically obfuscate function calls.

**Disclaimer**: This only works for code that you compile. This means that if you are linking against a compiled standart library, and that library makes a call to a function, it will not be covered by opt (unless the library has an llvm-ir version, but this is uncommon).

## Table of Contents
* [Setup](#setup)
* [Usage and example](#usage-and-example)
* [Developer guide](#developers-guide)
  * [File distribution](#file-distribution)
  * [How the pass works](#how-the-pass-works)
  *  [How the dispatching system works](#how-the-dispatching-system-works)

* [TODO](#todo)

## Setup
This setup is written for Windows, but it should be possible to setup this enviroment in Linux easily.

* **Dependencies**:
  
  To be able to compile this project, we mainly need 2 things: LLVM and CMAKE.


  LLVM can be either compiled from source, downloaded from the LLVM releases or installed throw MSYS2. For a guide on how to compile LLVM from source and compile a basic plugin, [see this]().
  We also need to set up CMAKE and our build tools. Clang is required to compile the helpers library. For the linker we dont care too much. Lastly, as a generator, I prefer Ninja, but make, nmake or msbuild will work.
  
  Everything listed above can be installed with pacman by using MSYS2, so lets do that.
  * Download and install [MSYS2](https://www.msys2.org/).
  * Launch an MSYS2 mingw64 terminal, and install the following packages:
  * Install LLVM : ```pacman -S mingw-w64-x86_64-llvm```
  * Install Clang: ```pacman -S mingw-w64-x86_64-clang```
  * Install Cmake: ```pacman -S mingw-w64-x86_64-cmake```
  * Install Ninja: ```pacman -S mingw-w64-x86_64-ninja```

* **Building**:
  
  First, clone this project, pretty obvious:

        git clone https://github.com/janoglezcampos/llvm-yx-callobfuscator

  To build this project, we will be using CMAKE. Because I found it convenient, I use vscode with the cmake extension. You will find my CMAKE config at ```.vscode_conf/setting.json```

  In any other case, launch an MSYS2 mingw64 terminal and do exactly what I say (without checking what any of the commands Im giving to you will do to your beloved machine):

  Go to the root directory of this repo:
  
        cd <whatever>/llvm-yx-obfuscator

  Create a build directory change dir to it:
  
        mkdir build; cd build

  Choose an installation location, I recommend doing this so it will be easier to either get the files on the rigth place, or just to be able to pick them easily. Also, the [usage](#usage-and-example) section later will use this folder as the relative folder for accessing this files, so if you add it to the path, the commands will work rigth away. I use  ```C:/Users/<my-user>/llvm-plugins```, but creating folder in this project directory called ```install```, side by side with ```build``` will do. This folder will not be edited if you dont run the install command, but you will have to go get the files to the build folder.
  
  Configure the project, here you specify the installation folder. ```DCMAKE_BUILD_TYPE``` will set the default mode: Debug, Release or MinSizeRel. It depends on wich generator you are using, if you are gonna be able to change this later or not. I use Nija as the generator, but you can use any other.
        
        cmake -G Ninja -DCMAKE_INSTALL_PREFIX="<path_to_install_folder>" -DCMAKE_BUILD_TYPE=Release ./..
        
  Build the project, optionally choose mode with ```—-config Release``` (if your generator lets you):
        
        cmake --build .
  Move the generated files to the install directory you chose before. If you dont want to use the install feature of cmake, just get the files (```libCallObfuscatorHelpers.a``` and ```CallObfuscatorPlugin.dll```) from the build directory and put them in a place you remember, you will need them.
  
        cmake --build . --target install

  At this point you should have two files: 
  * ```libCallObfuscatorHelpers.a```: A C library that includes all the logic that needs to be executed at runtime.  
  * ```CallObfuscatorPlugin.dll```:  The actual plugin, written in C++, that will be compiled and linked to a dll.
## Usage and example

First of all we need to setup our configuration file. In this section, we will be building the project found in the example folder, so the config file is already made. You can add any number of functions to the file, and the functions doesnt need to appear in the program.

The plugin will get the path to the config from an enviroment variable called ```LLVM_OBF_FUNCTIONS```. You can either add it along all the other env variables for the user, the system, or just set it up for the current terminal. You can also set it in the makefile, so it is only set for the compilation.

Now is time to run the pass, remember that there is a makefile already set up inside the example project. To better understand the process shown here, [see this](https://github.com/janoglezcampos/llvm-pass-plugin-skeleton?tab=readme-ov-file#running-you-pass).

* Go inside the example folder, create a build folder; inside, create 2 folders: irs and objs. 
    
        cd example; mkdir build; mkdir build/irs; mkdifr build/objs

* Set ```LLVM_OBF_FUNCTIONS``` to point to the full path of ```callobfuscator.conf```, only for this terminal:
    * MSYS2/Linux: ```export LLVM_OBF_FUNCTIONS=<absolute path to callobfuscator.conf>```
    * Windows PowerShell: ```env:LLVM_OBF_FUNCTIONS=<absolute path to callobfuscator.conf>```
* Compile the C files to LLVM-IR:
  
        clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ./source/utils.c -Iheaders -o ./build/irs/utils.ll

        clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm ./source/main.c -Iheaders -o ./build/irs/main.ll
* Merge all files:
  
        llvm-link ./build/irs/main.ll ./build/irs/utils.ll -S -o ./build/irs/example.ll
* Run obfusction pass:

        opt -load-pass-plugin="<path to the pass dll>" -passes="base-plugin-pass" ./build/irs/example.ll -o ./build/irs/example.obf.ll
* Run optimization passes:
  
        opt -O3 ./build/irs/example.obf.ll -o ./build/irs/example.op.ll
* Compile to windows x86_64 assembly:
    
        llc --mtriple=x86_64-pc-windows-msvc -filetype=obj  ./build/irs/example.op.ll -o ./build/objs/example.obj

* Link:
        
        clang ./build/objs/example.obj -o ./build/example.exe

Now you should have ```./build/example.exe```, the final executable.

## Developer guide
* ### File distribution
  The code is always divided in 2 folders, one called headers, for definitions and macros mainly, and the other called source, containing the actual source code. For every source code file, the is a header file matching the relative path to source, in the headers folder. Documentation for functions is always found at headers files.
  You will find two source codebases in this project:

  * **CallObfuscatorPlugin**: The actual plugin, written in C++, that will be compiled and linked to a dll.
    * **CallObfuscator**: Includes the logic to transparently apply call obfucation at compile time.
    * **CallObfuscatorPass**: Initalization and managment of the obfuscator pass.
    * **CallObfuscatorPluginRegister**: Plugin registration.

  * **CallObfuscatorHelpers**: A C library that includes all the logic that needs to be executed at runtime.
    * **common**: Common functionality that is used across the project.
    * **pe**: Utilities to manipulate and work with in-memory PEs.
    * **callDispatcher**: Funcionality to invoke windows native functions applying obfuscation.
    * **stackSpoof**: Functionality to apply dynamic stack spoofing in windows x64 enviroments.
    * **syscalls**: Utilities to work with windows x64 syscalls.

* ### How the pass works

* ### How the dispatching system works

## TODO:
### Docs/formatting:
* Rewrite this readme 5/10
* Document functions
* Add thanks
* Mby rename from FRAME_TABLE_ENTRY to something like FRAME_INFO
* Rename everything realted to pushRbp to saveRbp
* Somehow improve stackSpoofHelper.x64.asm readability
  
### Opsec:
* EAF bypass
* Opsec library loads on the call dispatcher

### General quality:
* Group all globals, or somehow make it clear in the code were all globals are declared
* Add checks if its safe to delete intructions from parent (the instruction that was replaced by a call)
* Put MIN_ADD_RSP_FRAME_SIZE to work

### Functionality:
* Handle invoke instructions and exception stuff (should not happen in C but...)
* Handle indirect calls
* Handle function address reads
