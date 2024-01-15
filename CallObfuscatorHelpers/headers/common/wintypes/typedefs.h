#ifndef _TYPESDEF_H_
#define _TYPESDEF_H_

#define WIN32_LEAN_AND_MEAN
#include <WinDef.h>

typedef HMODULE (*PLOADLIBRARYA)(LPCSTR lpLibFileName);

#ifndef _NTDEF_
typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;
#endif

#ifndef InitializeObjectAttributes
#define InitializeObjectAttributes(p, n, a, r, s) \
    {                                             \
        (p)->Length = sizeof(OBJECT_ATTRIBUTES);  \
        (p)->RootDirectory = r;                   \
        (p)->Attributes = a;                      \
        (p)->ObjectName = n;                      \
        (p)->SecurityDescriptor = s;              \
        (p)->SecurityQualityOfService = NULL;     \
    }
#endif

typedef UCHAR UBYTE;

typedef struct _IO_STATUS_BLOCK
{
    union
    {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef void *PPS_POST_PROCESS_INIT_ROUTINE;

typedef struct _LSA_UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES
{
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING, ANSI_STRING, *PANSI_STRING;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

// PEB defined by rewolf
// http://blog.rewolf.pl/blog/?p=573
typedef struct _PEB_LDR_DATA
{
    ULONG Length;
    BOOL Initialized;
    LPVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    LPVOID DllBase;
    LPVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB
{
    BYTE InheritedAddressSpace;
    BYTE ReadImageFileExecOptions;
    BYTE BeingDebugged;
    BYTE _SYSTEM_DEPENDENT_01;

    LPVOID Mutant;
    LPVOID ImageBaseAddress;

    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    LPVOID SubSystemData;
    LPVOID ProcessHeap;
    LPVOID FastPebLock;
    LPVOID _SYSTEM_DEPENDENT_02;
    LPVOID _SYSTEM_DEPENDENT_03;
    LPVOID _SYSTEM_DEPENDENT_04;
    union
    {
        LPVOID KernelCallbackTable;
        LPVOID UserSharedInfoPtr;
    };
    DWORD SystemReserved;
    DWORD _SYSTEM_DEPENDENT_05;
    LPVOID _SYSTEM_DEPENDENT_06;
    LPVOID TlsExpansionCounter;
    LPVOID TlsBitmap;
    DWORD TlsBitmapBits[2];
    LPVOID ReadOnlySharedMemoryBase;
    LPVOID _SYSTEM_DEPENDENT_07;
    LPVOID ReadOnlyStaticServerData;
    LPVOID AnsiCodePageData;
    LPVOID OemCodePageData;
    LPVOID UnicodeCaseTableData;
    DWORD NumberOfProcessors;
    union
    {
        DWORD NtGlobalFlag;
        LPVOID dummy02;
    };
    LARGE_INTEGER CriticalSectionTimeout;
    LPVOID HeapSegmentReserve;
    LPVOID HeapSegmentCommit;
    LPVOID HeapDeCommitTotalFreeThreshold;
    LPVOID HeapDeCommitFreeBlockThreshold;
    DWORD NumberOfHeaps;
    DWORD MaximumNumberOfHeaps;
    LPVOID ProcessHeaps;
    LPVOID GdiSharedHandleTable;
    LPVOID ProcessStarterHelper;
    LPVOID GdiDCAttributeList;
    LPVOID LoaderLock;
    DWORD OSMajorVersion;
    DWORD OSMinorVersion;
    WORD OSBuildNumber;
    WORD OSCSDVersion;
    DWORD OSPlatformId;
    DWORD ImageSubsystem;
    DWORD ImageSubsystemMajorVersion;
    LPVOID ImageSubsystemMinorVersion;
    union
    {
        LPVOID ImageProcessAffinityMask;
        LPVOID ActiveProcessAffinityMask;
    };
#ifdef _WIN64
    LPVOID GdiHandleBuffer[64];
#else
    LPVOID GdiHandleBuffer[32];
#endif
    LPVOID PostProcessInitRoutine;
    LPVOID TlsExpansionBitmap;
    DWORD TlsExpansionBitmapBits[32];
    LPVOID SessionId;
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    LPVOID pShimData;
    LPVOID AppCompatInfo;
    PUNICODE_STRING CSDVersion;
    LPVOID ActivationContextData;
    LPVOID ProcessAssemblyStorageMap;
    LPVOID SystemDefaultActivationContextData;
    LPVOID SystemAssemblyStorageMap;
    LPVOID MinimumStackCommit;
} PEB, *PPEB;

typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

#define GDI_BATCH_BUFFER_SIZE 310
typedef struct _GDI_TEB_BATCH
{
    ULONG Offset;
    ULONG_PTR HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

typedef struct _TEB_ACTIVE_FRAME_CONTEXT
{
    ULONG Flags;
    PSTR FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct _TEB_ACTIVE_FRAME
{
    ULONG Flags;
    struct _TEB_ACTIVE_FRAME *Previous;
    PTEB_ACTIVE_FRAME_CONTEXT Context;
} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;

typedef struct _TEB
{
    NT_TIB NtTib;

    PVOID EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    PPEB ProcessEnvironmentBlock;

    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PVOID CsrClientThread;
    PVOID Win32ThreadInfo;
    ULONG User32Reserved[26];
    ULONG UserReserved[5];
    PVOID WOW32Reserved;
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;
    PVOID SystemReserved1[54];
    NTSTATUS ExceptionCode;
    PVOID ActivationContextStackPointer;
#ifdef _M_X64
    UCHAR SpareBytes[24];
#else
    UCHAR SpareBytes[36];
#endif
    ULONG TxFsContext;

    GDI_TEB_BATCH GdiTebBatch;
    CLIENT_ID RealClientId;
    HANDLE GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    ULONG_PTR Win32ClientInfo[62];
    PVOID glDispatchTable[233];
    ULONG_PTR glReserved1[29];
    PVOID glReserved2;
    PVOID glSectionInfo;
    PVOID glSection;
    PVOID glTable;
    PVOID glCurrentRC;
    PVOID glContext;

    NTSTATUS LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[261];

    PVOID DeallocationStack;
    PVOID TlsSlots[64];
    LIST_ENTRY TlsLinks;

    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[2];

    ULONG HardErrorMode;
#ifdef _M_X64
    PVOID Instrumentation[11];
#else
    PVOID Instrumentation[9];
#endif
    GUID ActivityId;

    PVOID SubProcessTag;
    PVOID EtwLocalData;
    PVOID EtwTraceData;
    PVOID WinSockData;
    ULONG GdiBatchCount;

    union
    {
        PROCESSOR_NUMBER CurrentIdealProcessor;
        ULONG IdealProcessorValue;
        struct
        {
            UCHAR ReservedPad0;
            UCHAR ReservedPad1;
            UCHAR ReservedPad2;
            UCHAR IdealProcessor;
        };
    };

    ULONG GuaranteedStackBytes;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
    PVOID SavedPriorityState;
    ULONG_PTR SoftPatchPtr1;
    PVOID ThreadPoolData;
    PVOID *TlsExpansionSlots;
#ifdef _M_X64
    PVOID DeallocationBStore;
    PVOID BStoreLimit;
#endif
    ULONG MuiGeneration;
    ULONG IsImpersonating;
    PVOID NlsCache;
    PVOID pShimData;
    ULONG HeapVirtualAffinity;
    HANDLE CurrentTransactionHandle;
    PTEB_ACTIVE_FRAME ActiveFrame;
    PVOID FlsData;

    PVOID PreferredLanguages;
    PVOID UserPrefLanguages;
    PVOID MergedPrefLanguages;
    ULONG MuiImpersonation;

    union
    {
        USHORT CrossTebFlags;
        USHORT SpareCrossTebBits : 16;
    };
    union
    {
        USHORT SameTebFlags;
        struct
        {
            USHORT SafeThunkCall : 1;
            USHORT InDebugPrint : 1;
            USHORT HasFiberData : 1;
            USHORT SkipThreadAttach : 1;
            USHORT WerInShipAssertCode : 1;
            USHORT RanProcessInit : 1;
            USHORT ClonedThread : 1;
            USHORT SuppressDebugMsg : 1;
            USHORT DisableUserStackWalk : 1;
            USHORT RtlExceptionAttached : 1;
            USHORT InitialThread : 1;
            USHORT SessionAware : 1;
            USHORT SpareSameTebBits : 4;
        };
    };

    PVOID TxnScopeEnterCallback;
    PVOID TxnScopeExitCallback;
    PVOID TxnScopeContext;
    ULONG LockCount;
    ULONG SpareUlong0;
    PVOID ResourceRetValue;
    PVOID ReservedForWdf;
} TEB, *PTEB;

typedef enum _UNWIND_OP_CODES
{
    // x86_64. https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64.
    UWOP_PUSH_NONVOL = 0,
    UWOP_ALLOC_LARGE,     // 1
    UWOP_ALLOC_SMALL,     // 2
    UWOP_SET_FPREG,       // 3
    UWOP_SAVE_NONVOL,     // 4
    UWOP_SAVE_NONVOL_BIG, // 5
    UWOP_EPILOG,          // 6
    UWOP_SPARE_CODE,      // 7
    UWOP_SAVE_XMM128,     // 8
    UWOP_SAVE_XMM128BIG,  // 9
    UWOP_PUSH_MACH_FRAME, // 10

    // ARM64. https://docs.microsoft.com/en-us/cpp/build/arm64-exception-handling
    UWOP_ALLOC_MEDIUM,
    UWOP_SAVE_R19R20X,
    UWOP_SAVE_FPLRX,
    UWOP_SAVE_FPLR,
    UWOP_SAVE_REG,
    UWOP_SAVE_REGX,
    UWOP_SAVE_REGP,
    UWOP_SAVE_REGPX,
    UWOP_SAVE_LRPAIR,
    UWOP_SAVE_FREG,
    UWOP_SAVE_FREGX,
    UWOP_SAVE_FREGP,
    UWOP_SAVE_FREGPX,
    UWOP_SET_FP,
    UWOP_ADD_FP,
    UWOP_NOP,
    UWOP_END,
    UWOP_SAVE_NEXT,
    UWOP_TRAP_FRAME,
    UWOP_CONTEXT,
    UWOP_CLEAR_UNWOUND_TO_CALL,
    // ARM: https://docs.microsoft.com/en-us/cpp/build/arm-exception-handling

    UWOP_ALLOC_HUGE,
    UWOP_WIDE_ALLOC_MEDIUM,
    UWOP_WIDE_ALLOC_LARGE,
    UWOP_WIDE_ALLOC_HUGE,

    UWOP_WIDE_SAVE_REG_MASK,
    UWOP_WIDE_SAVE_SP,
    UWOP_SAVE_REGS_R4R7LR,
    UWOP_WIDE_SAVE_REGS_R4R11LR,
    UWOP_SAVE_FREG_D8D15,
    UWOP_SAVE_REG_MASK,
    UWOP_SAVE_LR,
    UWOP_SAVE_FREG_D0D15,
    UWOP_SAVE_FREG_D16D31,
    UWOP_WIDE_NOP, // UWOP_NOP
    UWOP_END_NOP,  // UWOP_END
    UWOP_WIDE_END_NOP,
    // Custom implementation opcodes (implementation specific).
    UWOP_CUSTOM,
} UNWIND_OP_CODES;

typedef union _UNWIND_CODE
{
    struct
    {
        UBYTE CodeOffset;   // 0xFF00
        UBYTE UnwindOp : 4; // 0x000f OPCODE
        UBYTE OpInfo : 4;   // 0x00f0
    };
    USHORT FrameOffset;
} UNWIND_CODE, *PUNWIND_CODE;

typedef struct _UNWIND_INFO
{
    UBYTE Version : 3;
    UBYTE Flags : 5;    // 4 bytes
    UBYTE SizeOfProlog; // 4 bytes
    UBYTE CountOfCodes; // 4 bytes
    UBYTE FrameRegister : 4;
    UBYTE FrameOffset : 4; // 4bytes
    UNWIND_CODE UnwindCode[1];
    union
    {
        OPTIONAL ULONG ExceptionHandler;
        OPTIONAL ULONG FunctionEntry;
    };
    OPTIONAL ULONG ExceptionData[];
} UNWIND_INFO, *PUNWIND_INFO;

typedef enum _REGISTERS
{
    RAX = 0,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15
} REGISTERS;

#endif