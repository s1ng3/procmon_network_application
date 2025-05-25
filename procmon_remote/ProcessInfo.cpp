// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

// This file contains the implementation of certain classes, which
// provides various functions to interact with processes on the system.

// The ProcessInfo class uses the Windows API to retrieve information about processes, threads, and DLLs.
#include <fstream>
#include "bits/stdc++.h"
#include <sstream>
#include <vector>
#include <limits>
#include <unordered_map>
#include "iostream"
#include "ProcessInfo.hpp"
#include "RaspberryProcesses.hpp"

ThreadInfo::ThreadInfo(DWORD no)
{
    PID=no;
    hThreadSnap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,PID);
    if (hThreadSnap==INVALID_HANDLE_VALUE)
    {
        cout<<"Unable to create the snapshot of the current thread pool"<<endl;
        return;
    }
    te32.dwSize=sizeof(THREADENTRY32);
}

BOOL ThreadInfo::ThreadsDisplay()
{
    if (!Thread32First(hThreadSnap,&te32))
    {
        cout<<"Error in getting the first thread"<<endl;
        CloseHandle(hThreadSnap);
        return false;
    }
    cout<<endl<<"THREAD OF THIS PROCESS: ";
    do
    {
        if (te32.th32OwnerProcessID==PID)
        {
            cout<<"THREAD ID : "<<te32.th32ThreadID<<endl;
        }
    } while (Thread32Next(hThreadSnap,&te32));
    CloseHandle(hThreadSnap);
    return true;
}

DLLInfo::DLLInfo(DWORD no)
{
    PID=no;
    hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,PID);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
    {
        cout<<"Unable to create the snapshot of the current module pool"<<endl;
        return;
    }
    me32.dwSize=sizeof(MODULEENTRY32);
}

BOOL DLLInfo::DependentDLLDisplay()
{
    char arr[200];
    if (!Module32First(hProcessSnap, &me32))
    {
        cout << "FAILED to get DLL Information" << endl;
        CloseHandle(hProcessSnap);
        return false;
    }
    cout << "DEPENDENT DLL OF THIS PROCESS: " << endl;
    do
    {
        wchar_t wModule[200];
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wModule, 200, me32.szModule, _TRUNCATE);
        wcstombs_s(&convertedChars, arr, 200, wModule, _TRUNCATE);
        cout << arr << endl;
    } while (Module32Next(hProcessSnap, &me32));
    CloseHandle(hProcessSnap);
    return true;
}

void ProcessInfo::DisplayHelp()
{
    cout<<"=========================================="<<endl;
    cout<<"              Process Monitor Help        "<<endl;
    cout<<"=========================================="<<endl;
    cout<<"Commands:"<<endl;
    cout<<"  - ProcessDisplay <processName> <option>"<<endl;
    cout<<"    Display information about a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - ProcessLog <processName>"<<endl;
    cout<<"    Log process information to a file."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - ProcessSearch <processName>"<<endl;
    cout<<"    Search for a process by name."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - KillProcess <processName>"<<endl;
    cout<<"    Terminate a process by name."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - DisplayHardwareInfo"<<endl;
    cout<<"    Display hardware information."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessMemoryUsage <processName>"<<endl;
    cout<<"    Display memory usage of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessUserName <processName>"<<endl;
    cout<<"    Display the user name of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessStatus <processName>"<<endl;
    cout<<"    Display the status of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessDescription <processName>"<<endl;
    cout<<"    Display the description of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessPriority <processName>"<<endl;
    cout<<"    Display the priority of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessStartTime <processName>"<<endl;
    cout<<"    Display the start time of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessCPUUsage <processName>"<<endl;
    cout<<"    Display the CPU usage of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - GetProcessPath <processName>"<<endl;
    cout<<"    Display the path of a process."<<endl;
    cout<<"=========================================="<<endl;
    cout<<"  - DisplayHelp"<<endl;
    cout<<"    Display this help message."<<endl;
    cout<<"=========================================="<<endl;
    cout<<"  - FastLimitRAM"<<endl;
    cout<<"    Limit RAM usage using SetProcessWorkingSetSize."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - LimitRAMWithJobObjects"<<endl;
    cout<<"    Limit RAM usage using Job Objects."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - LimitLogicalProcessors"<<endl;
    cout<<"    Limit the number of logical processors."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - OpenGivenProcess"<<endl;
    cout<<"    Open a given process."<<endl;
    cout<<"=========================================="<<endl;
    cout<<"  - DisplayRaspberryProcesses"<<endl;
    cout<<"    Display processes on a Raspberry Pi."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - LimitArduinoPowerForAnalogAndDigital"<<endl;
    cout<<"    Limit Arduino power for analog and digital."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - OptimizeProcessPerformance"<<endl;
    cout<<"    Optimize process performance."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - VerifyProcessIntegrity <processName>"<<endl;
    cout<<"    Verify the integrity of a process."<<endl;
    cout<<"------------------------------------------"<<endl;
    cout<<"  - Exit"<<endl;
    cout<<"    Exit the program."<<endl;
    cout<<"=========================================="<<endl;
}

ProcessInfo::ProcessInfo()
{
    ptobj=nullptr;
    pdobj=nullptr;

    hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
    {
        cout<<"Unable to create the snapshot of the current running process"<<endl;
        return;
    }
    pe32.dwSize=sizeof(PROCESSENTRY32);
}

void encryptAndLog(const string& logData, const string& filePath) {
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;
    HCRYPTHASH hHash;

    // Acquire a cryptographic context
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        throw runtime_error("Failed to acquire cryptographic context");
    }

    // Create a hash object
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to create hash object");
    }

    // Hash a password or key (for simplicity, using a hardcoded key here)
    const string key = "secure_key";
    if (!CryptHashData(hHash, reinterpret_cast<const BYTE*>(key.c_str()), key.size(), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to hash key");
    }

    // Generate an AES key from the hash
    if (!CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to derive AES key");
    }

    CryptDestroyHash(hHash);

    // Encrypt the data
    vector<BYTE> buffer(logData.begin(), logData.end());
    DWORD bufferSize = buffer.size();
    DWORD bufferCapacity = bufferSize + 16; // Add padding for encryption
    buffer.resize(bufferCapacity);

    if (!CryptEncrypt(hKey, 0, TRUE, 0, buffer.data(), &bufferSize, bufferCapacity)) {
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to encrypt data");
    }

    // Write the encrypted data to a file
    ofstream outFile(filePath, ios::binary);
    if (!outFile) {
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to open log file for writing");
    }
    outFile.write(reinterpret_cast<const char*>(buffer.data()), bufferSize);
    outFile.close();

    // Clean up
    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv, 0);
}

bool ProcessInfo::ProcessLog(const char* processName) {
    const char* month[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
    char FileName[50];
    SYSTEMTIME It;

    GetLocalTime(&It);

    sprintf_s(FileName, "F:\\logs_proj\\log_%02d_%02d_%02d_%s.enc", It.wHour, It.wMinute, It.wDay, month[It.wMonth - 1]);

    ostringstream logData;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cerr << "Unable to create the snapshot of the current running process" << endl;
        return false;
    }

    if (!Process32First(hProcessSnap, &pe32)) {
        cerr << "Error in finding the first process" << endl;
        CloseHandle(hProcessSnap);
        return false;
    }

    do {
        wchar_t wExeFile[200];
        char arr[200];
        mbstowcs_s(nullptr, wExeFile, pe32.szExeFile, 200);
        wcstombs_s(nullptr, arr, 200, wExeFile, 200);
        if (_stricmp(arr, processName) == 0) {
            logData << "Process Name: " << arr << "\n";
            logData << "PID: " << pe32.th32ProcessID << "\n";
            logData << "PPID: " << pe32.th32ParentProcessID << "\n";
            logData << "Thread Count: " << pe32.cntThreads << "\n";
            logData << "Priority Base: " << pe32.pcPriClassBase << "\n";
            logData << "Executable Path: " << arr << "\n";
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    try {
        encryptAndLog(logData.str(), FileName);
        cout << "Log data encrypted and saved to " << FileName << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }

    return true;
}

bool ProcessInfo::ProcessDisplay()
{
    char arr[200];
    if (!Process32First(hProcessSnap, &pe32))
    {
        cout << "Error in finding the first process" << endl;
        CloseHandle(hProcessSnap);
        return false;
    }
    do
    {
        wchar_t wExeFile[200];
        mbstowcs_s(nullptr, wExeFile, pe32.szExeFile, 200);
        wcstombs_s(nullptr, arr, 200, wExeFile, 200);

        cout << endl << "--------------------------------------";
        cout << endl << "PROCESS NAME: " << arr;
        cout << endl << "PID: " << pe32.th32ProcessID;
        cout << endl << "Parent ID: " << pe32.th32ParentProcessID;
        cout << endl << "No of Thread: " << pe32.cntThreads;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess != nullptr)
        {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                cout << endl << "Memory Usage: " << pmc.WorkingSetSize / 1024 << " KB";
            }

            DWORD_PTR processAffinityMask, systemAffinityMask;
            if (GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask))
            {
                cout << endl << "Running on cores: ";
                for (DWORD_PTR mask = 1, core = 0; mask != 0; mask <<= 1, ++core)
                {
                    if (processAffinityMask & mask)
                    {
                        cout << core << " ";
                    }
                }
            }
            else
            {
                cout << endl << "Failed to retrieve process affinity mask.";
            }
            CloseHandle(hProcess);
        }
        else
        {
            cout << endl << "Failed to open process.";
        }

        cout << endl << "--------------------------------------\n\n";

    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return true;
}

bool ProcessInfo::ProcessSearch(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess,(PROCESS_MEMORY_COUNTERS*)&pmc,sizeof(pmc)))
    {
        cout<<"Memory Usage: "<<pmc.PrivateUsage / 1024<<" KB"<<endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::KillProcess(const char* processName)
{
    HANDLE hProcess;
    char arr[200];

    hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
    {
        cout<<"Unable to create the snapshot of the current running process"<<endl;
        return false;
    }

    if (!Process32First(hProcessSnap,&pe32))
    {
        cout<<"Error in finding the first process"<<endl;
        CloseHandle(hProcessSnap);
        return false;
    }

    do
    {
        wchar_t wExeFile[200];
        mbstowcs_s(nullptr,wExeFile,pe32.szExeFile,200);
        wcstombs_s(nullptr,arr,200,wExeFile,200);
        if (_stricmp(arr,processName)==0)
        {
            hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,pe32.th32ProcessID);
            if (hProcess==nullptr)
            {
                cout<<"Unable to open process for termination"<<endl;
                CloseHandle(hProcessSnap);
                return false;
            }
            if (!TerminateProcess(hProcess,0))
            {
                cout<<"Unable to terminate process"<<endl;
                CloseHandle(hProcess);
                CloseHandle(hProcessSnap);
                return false;
            }
            cout<<"Process terminated successfully"<<endl;
            CloseHandle(hProcess);
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap,&pe32));

    cout<<"Process not found"<<endl;
    CloseHandle(hProcessSnap);
    return false;
}

bool ProcessInfo::OpenGivenProcess()
{
    string processPath;
    cout<<"Enter the name of the process to open: ";
    cin.ignore(); // Ignore any leftover newline character in the input buffer
    getline(cin,processPath);

    HINSTANCE result=ShellExecute(nullptr,"open",processPath.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        DWORD error=GetLastError();
        cout<<"Failed to open process: "<<processPath<<endl;
        cout<<"Error code: "<<error<<endl;
        return false;
    }
    cout<<"Process opened successfully: "<<processPath<<endl;
    return true;
}

bool ProcessInfo::DisplayHardwareInfo()
{
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    // Get the number of logical processors
    DWORD length=0;
    GetLogicalProcessorInformation(nullptr,&length);
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer=(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(length);
    GetLogicalProcessorInformation(buffer,&length);

    int coreCount=0;
    DWORD count=length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (DWORD i=0; i < count; ++i)
    {
        if (buffer[i].Relationship==RelationProcessorCore)
        {
            coreCount++;
        }
    }
    free(buffer);

    cout<<"Hardware information: "<<endl;
    cout<<"  Number of logical processors: "<<coreCount<<endl;
    cout<<"  Number of processors: "<<siSysInfo.dwNumberOfProcessors<<endl;
    cout<<"  OEM ID: "<<siSysInfo.dwOemId<<endl;
    cout<<"  Page size: "<<siSysInfo.dwPageSize<<endl;
    cout<<"  Processor type: "<<siSysInfo.dwProcessorType<<endl;
    cout<<"  Minimum application address: "<<siSysInfo.lpMinimumApplicationAddress<<endl;
    cout<<"  Maximum application address: "<<siSysInfo.lpMaximumApplicationAddress<<endl;
    cout<<"  Active processor mask: "<<siSysInfo.dwActiveProcessorMask<<endl;

    // Get total physical memory
    MEMORYSTATUSEX statex;
    statex.dwLength=sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        cout<<"  Total physical memory: "<<statex.ullTotalPhys / (1024 * 1024)<<" MB"<<endl;
    }

    return TRUE;
}

bool ProcessInfo::GetProcessMemoryUsage(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess,(PROCESS_MEMORY_COUNTERS*)&pmc,sizeof(pmc)))
    {
        cout<<"Memory Usage: "<<pmc.PrivateUsage / 1024<<" KB"<<endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessUserName(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    HANDLE hToken;
    if (OpenProcessToken(hProcess,TOKEN_QUERY,&hToken))
    {
        DWORD dwSize=0;
        GetTokenInformation(hToken,TokenUser,nullptr,0,&dwSize);
        PTOKEN_USER pTokenUser=(PTOKEN_USER)malloc(dwSize);

        if (GetTokenInformation(hToken,TokenUser,pTokenUser,dwSize,&dwSize))
        {
            SID_NAME_USE SidType;
            char lpName[256],lpDomain[256];
            DWORD dwNameSize=sizeof(lpName),dwDomainSize=sizeof(lpDomain);

            if (LookupAccountSid(nullptr,pTokenUser->User.Sid,lpName,&dwNameSize,lpDomain,&dwDomainSize,&SidType))
            {
                cout<<"User Name: "<<lpDomain<<"\\"<<lpName<<endl;
            }
        }
        free(pTokenUser);
        CloseHandle(hToken);
    }
    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessStatus(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    DWORD exitCode;
    if (GetExitCodeProcess(hProcess,&exitCode))
    {
        if (exitCode==STILL_ACTIVE)
            cout<<"Status: Running"<<endl;
        else
            cout<<"Status: Not Running"<<endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessDescription(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    TCHAR szProcessName[MAX_PATH]=TEXT("<unknown>");
    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess,&hMod,sizeof(hMod),&cbNeeded))
    {
        GetModuleBaseName(hProcess,hMod,szProcessName,sizeof(szProcessName) / sizeof(TCHAR));
    }

    cout<<"Description: "<<szProcessName<<endl;

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessPriority(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    DWORD priority=GetPriorityClass(hProcess);
    if (priority)
    {
        cout<<"Priority: "<<priority<<endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessStartTime(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    FILETIME creationTime,exitTime,kernelTime,userTime;
    if (GetProcessTimes(hProcess,&creationTime,&exitTime,&kernelTime,&userTime))
    {
        SYSTEMTIME st;
        FileTimeToSystemTime(&creationTime,&st);
        cout<<"Start Time: "<<st.wHour<<":"<<st.wMinute<<":"<<st.wSecond<<endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessCPUUsage(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    FILETIME ftCreation,ftExit,ftKernel,ftUser;
    ULARGE_INTEGER lastKernel,lastUser,currentKernel,currentUser;
    SYSTEM_INFO sysInfo;
    FILETIME ftSysIdle,ftSysKernel,ftSysUser;
    ULARGE_INTEGER lastSysKernel,lastSysUser,currentSysKernel,currentSysUser;

    GetSystemInfo(&sysInfo);
    int numProcessors=sysInfo.dwNumberOfProcessors;

    if (!GetProcessTimes(hProcess,&ftCreation,&ftExit,&ftKernel,&ftUser))
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    memcpy(&lastKernel,&ftKernel,sizeof(FILETIME));
    memcpy(&lastUser,&ftUser,sizeof(FILETIME));

    GetSystemTimes(&ftSysIdle,&ftSysKernel,&ftSysUser);
    memcpy(&lastSysKernel,&ftSysKernel,sizeof(FILETIME));
    memcpy(&lastSysUser,&ftSysUser,sizeof(FILETIME));

    Sleep(1000); // Wait for 1 second

    if (!GetProcessTimes(hProcess,&ftCreation,&ftExit,&ftKernel,&ftUser))
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    memcpy(&currentKernel,&ftKernel,sizeof(FILETIME));
    memcpy(&currentUser,&ftUser,sizeof(FILETIME));

    GetSystemTimes(&ftSysIdle,&ftSysKernel,&ftSysUser);
    memcpy(&currentSysKernel,&ftSysKernel,sizeof(FILETIME));
    memcpy(&currentSysUser,&ftSysUser,sizeof(FILETIME));

    ULONGLONG sysKernelDiff=(currentSysKernel.QuadPart - lastSysKernel.QuadPart) / numProcessors;
    ULONGLONG sysUserDiff=(currentSysUser.QuadPart - lastSysUser.QuadPart) / numProcessors;
    ULONGLONG procKernelDiff=currentKernel.QuadPart - lastKernel.QuadPart;
    ULONGLONG procUserDiff=currentUser.QuadPart - lastUser.QuadPart;

    double cpuUsage=(double)(procKernelDiff+procUserDiff) * 100.0 / (sysKernelDiff+sysUserDiff);

    cout<<"CPU Usage: "<<cpuUsage<<"%"<<endl;

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::GetProcessPath(const char* processName)
{
    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
        return FALSE;

    HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,processID);
    if (nullptr==hProcess)
        return FALSE;

    char processPath[MAX_PATH];
    if (GetModuleFileNameEx(hProcess,nullptr,processPath,MAX_PATH))
    {
        cout<<"Process Path: "<<processPath<<endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::FastLimitRAM()
{
    char processName[200];
    SIZE_T memoryLimit;

    cout<<"Enter process name: ";
    cin>>processName;
    cout<<"Enter memory limit (in KB): ";
    cin>>memoryLimit;

    memoryLimit *= 1024; // Convert to bytes

    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
    {
        cout<<"Process not found"<<endl;
        return FALSE;
    }

    HANDLE hProcess=OpenProcess(PROCESS_SET_QUOTA,FALSE,processID);
    if (hProcess==nullptr)
    {
        cout<<"Unable to open process"<<endl;
        return FALSE;
    }

    if (!SetProcessWorkingSetSize(hProcess,memoryLimit,memoryLimit))
    {
        cout<<"Failed to set process working set size"<<endl;
        CloseHandle(hProcess);
        return FALSE;
    }

    cout<<"Successfully set process working set size"<<endl;
    CloseHandle(hProcess);
    return TRUE;
}

/// LIMIT RAM WITH JOBS

bool ProcessInfo::LimitRAMWithJobObjects()
{
    char processName[200];
    SIZE_T memoryLimit;

    cout<<"Enter process name: ";
    cin>>processName;
    cout<<"Enter memory limit (in KB): ";
    cin>>memoryLimit;

    memoryLimit *= 1024; // Convert to bytes

    DWORD processID=0;
    HANDLE hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnap==INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize=sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap,&pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile,processName)==0)
            {
                processID=pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap,&pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID==0)
    {
        cout<<"Process not found"<<endl;
        return FALSE;
    }

    HANDLE hProcess=OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE,FALSE,processID);
    if (hProcess==nullptr)
    {
        cout<<"Unable to open process"<<endl;
        return FALSE;
    }

    HANDLE hJob=CreateJobObject(nullptr,nullptr);
    if (hJob==nullptr)
    {
        cout<<"Failed to create job object"<<endl;
        CloseHandle(hProcess);
        return FALSE;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobLimit={0};
    jobLimit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    jobLimit.ProcessMemoryLimit=memoryLimit;
    jobLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
    jobLimit.JobMemoryLimit=memoryLimit;

    if (!SetInformationJobObject(hJob,JobObjectExtendedLimitInformation,&jobLimit,sizeof(jobLimit)))
    {
        cout<<"Failed to set job object information"<<endl;
        CloseHandle(hJob);
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!AssignProcessToJobObject(hJob,hProcess))
    {
        cout<<"Failed to assign process to job object"<<endl;
        CloseHandle(hJob);
        CloseHandle(hProcess);
        return FALSE;
    }

    cout<<"Successfully set process memory limit"<<endl;
    CloseHandle(hJob);
    CloseHandle(hProcess);
    return TRUE;
}

bool isBlackListed(const string& processName) {
    static const set<string> blackListedProcesses = {
            "System", "smss.exe", "csrss.exe", "wininit.exe", "services.exe",
            "lsass.exe", "svchost.exe", "explorer.exe", "winlogon.exe"
    };

    string lowerProcessName = processName;
    transform(lowerProcessName.begin(), lowerProcessName.end(), lowerProcessName.begin(), ::tolower);

    return blackListedProcesses.find(lowerProcessName) != blackListedProcesses.end();
}


bool ProcessInfo::LimitLogicalProcessors() {
    string processName;
    DWORD_PTR coreMask;

    cout << "Enter process name: ";
    cin >> processName;
    cout << "Enter core mask (in hexadecimal, e.g., 0xA9): ";
    cin >> hex >> coreMask;

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            string exeFile(pe32.szExeFile);
            transform(exeFile.begin(), exeFile.end(), exeFile.begin(), ::tolower);

            // Skip blacklisted processes
            if (isBlackListed(exeFile)) {
                cout << "Skipping blacklisted process: " << exeFile << endl;
                continue;
            }

            if (exeFile == processName) {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0) {
        cout << "Process not found" << endl;
        return FALSE;
    }

    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processID);
    if (hProcess == nullptr) {
        cout << "Unable to open process" << endl;
        return FALSE;
    }

    if (!SetProcessAffinityMask(hProcess, coreMask)) {
        cout << "Failed to set process affinity mask" << endl;
        CloseHandle(hProcess);
        return FALSE;
    }

    cout << "Successfully set process affinity mask" << endl;
    CloseHandle(hProcess);
    return TRUE;
}

bool ProcessInfo::DisplayRaspberryProcesses() {
    string hostname,username,password;
    cout<<"Enter hostname: ";
    cin>>hostname;
    cout<<"Enter username: ";
    cin>>username;
    cout<<"Enter password: ";
    cin>>password;

    try {
        string processes=GetRaspberryProcesses(hostname,username,password);
        cout<<"Running processes "
                "on Raspberry Pi:"<<endl;
        cout<<processes<<endl;
    } catch (const exception& e) {
        cerr<<"Error: "<<e.what()<<endl;
        return false;
    }
    return true;
}

bool ProcessInfo::LimitArduinoPowerForAnalogAndDigital() {
    ssh_session my_ssh_session;
    int rc;
    string hostname,username,password;
    const char *commandTemplate="python3 /home/kali/snap/arduino/current/Arduino/arduino_control.py \"%s\"";

    // Read hostname,username,and password from the user
    cout<<"Enter hostname: ";
    cin>>hostname;
    cout<<"Enter username: ";
    cin>>username;
    cout<<"Enter password: ";
    char ch;
    while ((ch=_getch()) != '\r') { // '\r' is the Enter key
        if (ch=='\b') { // Handle backspace
            if (!password.empty()) {
                cout<<"\b \b";
                password.pop_back();
            }
        } else {
            password.push_back(ch);
            cout<<'*';
        }
    }
    cout<<endl;

    // Initialize SSH session
    my_ssh_session=ssh_new();
    if (my_ssh_session==nullptr) {
        cerr<<"Error: Unable to create SSH session."<<endl;
        return false;
    }

    // Set SSH options
    ssh_options_set(my_ssh_session,SSH_OPTIONS_HOST,hostname.c_str());
    ssh_options_set(my_ssh_session,SSH_OPTIONS_USER,username.c_str());

    // Connect to the server
    rc=ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        cerr<<"Error: Unable to connect to Raspberry Pi: "<<ssh_get_error(my_ssh_session)<<endl;
        ssh_free(my_ssh_session);
        return false;
    }

    // Authenticate with password
    rc=ssh_userauth_password(my_ssh_session,nullptr,password.c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        cerr<<"Error: Authentication failed: "<<ssh_get_error(my_ssh_session)<<endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return false;
    }

    // Ports to control
    int analogPorts[]={3,5,6,9,10,11};
    int digitalPorts[]={4,7,8,12,13};
    string powerValues;

    for (int port : analogPorts) {
        int powerValue;
        cout<<"Enter power value for analog port "<<port<<" (0-255): ";
        cin>>powerValue;
        if (powerValue < 0 || powerValue > 255) {
            cerr<<"Invalid power value for analog port. Must be between 0 and 255."<<endl;
            return false;
        }
        powerValues += to_string(port)+","+to_string(powerValue)+" ";
    }

    for (int port : digitalPorts) {
        int powerValue;
        cout<<"Enter power value for digital port "<<port<<" (0 or 1): ";
        cin>>powerValue;
        if (powerValue != 0 && powerValue != 1) {
            cerr<<"Invalid power value for digital port. Must be 0 or 1."<<endl;
            return false;
        }
        powerValues += to_string(port)+","+to_string(powerValue)+" ";
    }

    // Format the command with the power values
    char command[512];
    snprintf(command,sizeof(command),commandTemplate,powerValues.c_str());

    // Execute the command
    ssh_channel channel=ssh_channel_new(my_ssh_session);
    if (channel==nullptr) {
        cerr<<"Error: Unable to create SSH channel."<<endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return false;
    }

    rc=ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        cerr<<"Error: Unable to open SSH channel: "<<ssh_get_error(my_ssh_session)<<endl;
        ssh_channel_free(channel);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return false;
    }

    rc=ssh_channel_request_exec(channel,command);
    if (rc != SSH_OK) {
        cerr<<"Error: Unable to execute command: "<<ssh_get_error(my_ssh_session)<<endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return false;
    }

    // Read the command output
    char buffer[256];
    int nbytes;
    while ((nbytes=ssh_channel_read(channel,buffer,sizeof(buffer),0)) > 0) {
        cout.write(buffer,nbytes);
    }

    // Close the channel
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    // Close the session
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);

    return true;
}

bool ProcessInfo::OptimizeProcessPerformance() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD numLogicalProcessors = sysInfo.dwNumberOfProcessors;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cerr << "Unable to create the snapshot of the current running processes" << endl;
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        cerr << "Error in finding the first process" << endl;
        CloseHandle(hProcessSnap);
        return false;
    }

    DWORD_PTR coreMask = 0x01; // Start with the first core
    do {
        string processName(pe32.szExeFile);

        // Skip blacklisted processes
        if (isBlackListed(processName)) {
            cout << "Skipping blacklisted process: " << processName << endl;
            continue;
        }

        HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe32.th32ProcessID);
        if (hProcess != nullptr) {
            if (!SetProcessAffinityMask(hProcess, coreMask)) {
                cerr << "Failed to set process affinity mask for: " << processName << endl;
            } else {
                cout << "Set affinity mask for process: " << processName
                     << " to core mask: " << hex << coreMask << endl;
            }
            CloseHandle(hProcess);
        } else {
            cerr << "Unable to open process: " << processName << endl;
        }

        coreMask <<= 1; // Shift to the next core
        if (coreMask >= (1ULL << numLogicalProcessors)) {
            coreMask = 0x01; // Reset to the first core if all cores are used
        }

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return true;
}

string GenerateFileHash(const char* filePath) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    BYTE hash[32];
    DWORD hashSize = sizeof(hash);

    // Acquire a cryptographic context
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        throw runtime_error("Failed to acquire cryptographic context");
    }

    // Create a hash object
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to create hash object");
    }

    // Open the file
    ifstream file(filePath, ios::binary);
    if (!file) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to open file");
    }

    // Read and hash the file data
    vector<char> buffer(4096);
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        if (!CryptHashData(hHash, reinterpret_cast<BYTE*>(buffer.data()), file.gcount(), 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            throw runtime_error("Failed to hash file data");
        }
    }

    // Get the hash value
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashSize, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        throw runtime_error("Failed to get hash value");
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Convert hash to a string
    ostringstream hashString;
    for (DWORD i = 0; i < hashSize; ++i) {
        hashString << hex << setw(2) << setfill('0') << (int)hash[i];
    }

    return hashString.str();
}


bool ProcessInfo::VerifyProcessIntegrity(const char* processName) {
    static unordered_map<string, string> processHashes;

    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName) == 0) {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0) {
        cerr << "Process not found" << endl;
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess == nullptr) {
        cerr << "Unable to open process" << endl;
        return false;
    }

    char processPath[MAX_PATH];
    if (!GetModuleFileNameEx(hProcess, nullptr, processPath, MAX_PATH)) {
        cerr << "Failed to get process path" << endl;
        CloseHandle(hProcess);
        return false;
    }
    CloseHandle(hProcess);

    try {
        string computedHash = GenerateFileHash(processPath);

        // Check if the hash is already stored
        if (processHashes.find(processName) != processHashes.end()) {
            if (processHashes[processName] == computedHash) {
                cout << "Process integrity verified successfully" << endl;
                return true;
            } else {
                cerr << "Process integrity check failed (hash mismatch)" << endl;
                return false;
            }
        }

        // Store the hash for future comparisons
        processHashes[processName] = computedHash;
        cout << "Generated and stored hash for process: " << processName << endl;
        return true;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}

// added prometheus and node exporter

