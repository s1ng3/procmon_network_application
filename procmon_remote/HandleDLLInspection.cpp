//
// Created by Tudor on 19.06.2025.
//

#include "HandleDLLInspection.hpp"
#include <tlhelp32.h>
// winternl.h provides SYSTEM_HANDLE_INFORMATION and SYSTEM_HANDLE_ENTRY
#include <winternl.h>

// Function pointers for NT APIs
using NtQuerySystemInformation_t = NTSTATUS(NTAPI*)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
using NtQueryObject_t = NTSTATUS(NTAPI*)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);

static NtQuerySystemInformation_t pNtQuerySystemInformation =
    (NtQuerySystemInformation_t)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");
static NtQueryObject_t pNtQueryObject =
    (NtQueryObject_t)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryObject");

bool HandleDLLInspection::getHandles(DWORD pid, std::vector<HandleInfo>& handles) {
    if (!pNtQuerySystemInformation) return false;
    ULONG size = 0;
    NTSTATUS status = pNtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)16, nullptr, 0, &size);
    if (size == 0) return false;
    PBYTE buffer = (PBYTE)malloc(size);
    if (!buffer) return false;
    status = pNtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)16, buffer, size, &size);
    if (!NT_SUCCESS(status)) { free(buffer); return false; }
    auto info = (SYSTEM_HANDLE_INFORMATION*)buffer;
    for (ULONG i = 0; i < info->Count; ++i) {
        auto& e = info->Handle[i];
        if ((DWORD)e.OwnerPid != pid) continue;
        HANDLE source = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
        if (!source) continue;
        HANDLE dup = nullptr;
        if (!DuplicateHandle(source, (HANDLE)(ULONG_PTR)e.HandleValue, GetCurrentProcess(), &dup, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            CloseHandle(source);
            continue;
        }
        HandleInfo hi;
        hi.pid = pid;
        hi.handle = e.HandleValue;
        // Query type name
        BYTE typeBuffer[1024];
        ULONG returnLen = 0;
        auto oti = (PUBLIC_OBJECT_TYPE_INFORMATION*)typeBuffer;
        status = pNtQueryObject(dup, ObjectTypeInformation, oti, sizeof(typeBuffer), &returnLen);
        if (NT_SUCCESS(status)) {
            hi.type = QString::fromWCharArray(oti->TypeName.Buffer, oti->TypeName.Length/sizeof(WCHAR));
        }
        // Query object name
        ULONG nameSize = 0x1000;
        PBYTE nameBuf = (PBYTE)malloc(nameSize);
        if (nameBuf) {
            status = pNtQueryObject(dup, ObjectNameInformation, nameBuf, nameSize, &returnLen);
            if (NT_SUCCESS(status)) {
                auto uni = (UNICODE_STRING*)nameBuf;
                hi.name = QString::fromWCharArray(uni->Buffer, uni->Length/sizeof(WCHAR));
            }
            free(nameBuf);
        }
        handles.push_back(hi);
        CloseHandle(dup);
        CloseHandle(source);
    }
    free(buffer);
    return true;
}

bool HandleDLLInspection::getModules(DWORD pid, std::vector<ModuleInfo>& modules) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (hSnap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32 me{ sizeof(me) };
    if (Module32First(hSnap, &me)) {
        do {
            ModuleInfo mi;
            mi.pid = pid;
            mi.hModule = me.hModule;
            mi.moduleName = QString::fromLocal8Bit(me.szModule);
            mi.modulePath = QString::fromLocal8Bit(me.szExePath);
            modules.push_back(mi);
        } while (Module32Next(hSnap, &me));
    }
    CloseHandle(hSnap);
    return true;
}
