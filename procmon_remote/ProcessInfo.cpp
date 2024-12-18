// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

// This file contains the implementation of certain classes, which
// provides various functions to interact with processes on the system.

// The ProcessInfo class uses the Windows API to retrieve information about processes, threads, and DLLs.

#include "ProcessInfo.hpp"

ThreadInfo::ThreadInfo(DWORD no)
{
    PID = no;
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, PID);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Unable to create the snapshot of the current thread pool" << endl;
        return;
    }
    te32.dwSize = sizeof(THREADENTRY32);
}

BOOL ThreadInfo::ThreadsDisplay()
{
    if (!Thread32First(hThreadSnap, &te32))
    {
        cout << "Error in getting the first thread" << endl;
        CloseHandle(hThreadSnap);
        return false;
    }
    cout << endl << "THREAD OF THIS PROCESS: ";
    do
    {
        if (te32.th32OwnerProcessID == PID)
        {
            cout << "\tTHREAD ID : " << te32.th32ThreadID << endl;
        }
    } while (Thread32Next(hThreadSnap, &te32));
    CloseHandle(hThreadSnap);
    return true;
}

DLLInfo::DLLInfo(DWORD no)
{
    PID = no;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Unable to create the snapshot of the current module pool" << endl;
        return;
    }
    me32.dwSize = sizeof(MODULEENTRY32);
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
        mbstowcs_s(NULL, wModule, me32.szModule, 200);
        wcstombs_s(NULL, arr, 200, wModule, 200);
        cout << arr << endl;
    } while (Module32Next(hProcessSnap, &me32));
    CloseHandle(hProcessSnap);
    return true;
}

void ProcessInfo::DisplayHelp()
{
    cout << "==========================================" << endl;
    cout << "              Process Monitor Help        " << endl;
    cout << "==========================================" << endl;
    cout << "Commands:" << endl;
    cout << "  - ProcessDisplay <processName> <option>" << endl;
    cout << "    Display information about a process." << endl;
    cout << "    Options:" << endl;
    cout << "      -a: Display all information (threads and DLLs)." << endl;
    cout << "      -t: Display thread information." << endl;
    cout << "      -d: Display DLL information." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - ProcessLog <processName>" << endl;
    cout << "    Log process information to a file." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - ProcessSearch <processName>" << endl;
    cout << "    Search for a process by name." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - KillProcess <processName>" << endl;
    cout << "    Terminate a process by name." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - DisplayHardwareInfo" << endl;
    cout << "    Display hardware information." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessMemoryUsage <processName>" << endl;
    cout << "    Display memory usage of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessUserName <processName>" << endl;
    cout << "    Display the user name of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessStatus <processName>" << endl;
    cout << "    Display the status of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessDescription <processName>" << endl;
    cout << "    Display the description of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessPriority <processName>" << endl;
    cout << "    Display the priority of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessStartTime <processName>" << endl;
    cout << "    Display the start time of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessCPUUsage <processName>" << endl;
    cout << "    Display the CPU usage of a process." << endl;
    cout << "------------------------------------------" << endl;
    cout << "  - GetProcessPath <processName>" << endl;
    cout << "    Display the path of a process." << endl;
    cout << "==========================================" << endl;
}

ProcessInfo::ProcessInfo()
{
    ptobj = NULL;
    pdobj = NULL;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Unable to create the snapshot of the current running process" << endl;
        return;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
}

BOOL ProcessInfo::ProcessLog(char* processName)
{
    const char* month[] = { "JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC" };
    char FileName[50], arr[512];
    SYSTEMTIME It;
    FILE* fp;

    GetLocalTime(&It);

    sprintf_s(FileName, "F:\\logs_proj\\log_%02d_%02d_%02d_%s.txt", It.wHour, It.wMinute, It.wDay, month[It.wMonth - 1]);
    fp = fopen(FileName, "w");
    if (fp == NULL)
    {
        cout << "Unable to create a Log File" << endl;
        return false;
    }
    else
    {
        cout << "Log File successfully created as " << FileName << endl;
        cout << "Time of Log File creation: " << It.wHour << "." << It.wMinute << "." << It.wDay << "th " << month[It.wMonth - 1] << endl;
    }

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Unable to create the snapshot of the current running process" << endl;
        fclose(fp);
        return false;
    }

    if (!Process32First(hProcessSnap, &pe32))
    {
        cout << "Error in finding the first process" << endl;
        CloseHandle(hProcessSnap);
        fclose(fp);
        return false;
    }

    do
    {
        wchar_t wExeFile[200];
        mbstowcs_s(NULL, wExeFile, pe32.szExeFile, 200);
        wcstombs_s(NULL, arr, 200, wExeFile, 200);
        if (_stricmp(arr, processName) == 0)
        {
            fprintf(fp, "Process Name: %s\n", arr);
            fprintf(fp, "PID: %u\n", pe32.th32ProcessID);
            fprintf(fp, "PPID: %u\n", pe32.th32ParentProcessID);
            fprintf(fp, "Thread Count: %u\n", pe32.cntThreads);
            fprintf(fp, "Priority Base: %d\n", pe32.pcPriClassBase);
            fprintf(fp, "Executable Path: %s\n", arr); // Assuming arr contains the executable path
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    fclose(fp);
    return true;
}

BOOL ProcessInfo::ProcessDisplay(char* processName, char* option)
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
        mbstowcs_s(NULL, wExeFile, pe32.szExeFile, 200);
        wcstombs_s(NULL, arr, 200, wExeFile, 200);

        if (strlen(processName) == 0 || _stricmp(arr, processName) == 0)
        {
            cout << endl << "--------------------------------------";
            cout << endl << "PROCESS NAME: " << arr;
            cout << endl << "PID: " << pe32.th32ProcessID;
            cout << endl << "Parent ID: " << pe32.th32ParentProcessID;
            cout << endl << "No of Thread: " << pe32.cntThreads;

            if ((_stricmp(option, "-a") == 0) || (_stricmp(option, "-d") == 0) || (_stricmp(option, "-t") == 0))
            {
                if ((_stricmp(option, "-t") == 0) || (_stricmp(option, "-a") == 0))
                {
                    ptobj = new ThreadInfo(pe32.th32ProcessID);
                    ptobj->ThreadsDisplay();
                    delete ptobj;
                }
                if ((_stricmp(option, "-d") == 0) || (_stricmp(option, "-a") == 0))
                {
                    pdobj = new DLLInfo(pe32.th32ProcessID);
                    pdobj->DependentDLLDisplay();
                    delete pdobj;
                }
            }
            cout << endl << "--------------------------------------";

            if (strlen(processName) != 0)
            {
                break;
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return true;
}

BOOL ProcessInfo::ProcessSearch(char* processName)
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
        mbstowcs_s(NULL, wExeFile, pe32.szExeFile, 200);
        wcstombs_s(NULL, arr, 200, wExeFile, 200);
        if (_stricmp(arr, processName) == 0)
        {
            cout << "PROCESS FOUND: " << arr << endl;
            cout << "PID: " << pe32.th32ProcessID << endl;
            cout << "Parent ID: " << pe32.th32ParentProcessID << endl;
            cout << "No of Threads: " << pe32.cntThreads << endl;
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));
    cout << "Process not found" << endl;
    CloseHandle(hProcessSnap);
    return false;
}

BOOL ProcessInfo::KillProcess(char* processName)
{
    HANDLE hProcess;
    char arr[200];

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "Unable to create the snapshot of the current running process" << endl;
        return false;
    }

    if (!Process32First(hProcessSnap, &pe32))
    {
        cout << "Error in finding the first process" << endl;
        CloseHandle(hProcessSnap);
        return false;
    }

    do
    {
        wchar_t wExeFile[200];
        mbstowcs_s(NULL, wExeFile, pe32.szExeFile, 200);
        wcstombs_s(NULL, arr, 200, wExeFile, 200);
        if (_stricmp(arr, processName) == 0)
        {
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess == NULL)
            {
                cout << "Unable to open process for termination" << endl;
                CloseHandle(hProcessSnap);
                return false;
            }
            if (!TerminateProcess(hProcess, 0))
            {
                cout << "Unable to terminate process" << endl;
                CloseHandle(hProcess);
                CloseHandle(hProcessSnap);
                return false;
            }
            cout << "Process terminated successfully" << endl;
            CloseHandle(hProcess);
            CloseHandle(hProcessSnap);
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    cout << "Process not found" << endl;
    CloseHandle(hProcessSnap);
    return false;
}

BOOL ProcessInfo::DisplayHardwareInfo()
{
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    cout << "Hardware information: " << endl;
    cout << "  OEM ID: " << siSysInfo.dwOemId << endl;
    cout << "  Number of processors: " << siSysInfo.dwNumberOfProcessors << endl;
    cout << "  Page size: " << siSysInfo.dwPageSize << endl;
    cout << "  Processor type: " << siSysInfo.dwProcessorType << endl;
    cout << "  Minimum application address: " << siSysInfo.lpMinimumApplicationAddress << endl;
    cout << "  Maximum application address: " << siSysInfo.lpMaximumApplicationAddress << endl;
    cout << "  Active processor mask: " << siSysInfo.dwActiveProcessorMask << endl;

    return true;
}

BOOL ProcessInfo::GetProcessMemoryUsage(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        cout << "Memory Usage: " << pmc.WorkingSetSize / 1024 << " KB" << endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessUserName(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    HANDLE hToken;
    if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
    {
        DWORD dwSize = 0;
        GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
        PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(dwSize);

        if (GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize))
        {
            SID_NAME_USE SidType;
            char lpName[256], lpDomain[256];
            DWORD dwNameSize = sizeof(lpName), dwDomainSize = sizeof(lpDomain);

            if (LookupAccountSid(NULL, pTokenUser->User.Sid, lpName, &dwNameSize, lpDomain, &dwDomainSize, &SidType))
            {
                cout << "User Name: " << lpDomain << "\\" << lpName << endl;
            }
        }
        free(pTokenUser);
        CloseHandle(hToken);
    }
    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessStatus(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    DWORD exitCode;
    if (GetExitCodeProcess(hProcess, &exitCode))
    {
        if (exitCode == STILL_ACTIVE)
            cout << "Status: Running" << endl;
        else
            cout << "Status: Not Running" << endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessDescription(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
    {
        GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
    }

    cout << "Description: " << szProcessName << endl;

    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessPriority(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    DWORD priority = GetPriorityClass(hProcess);
    if (priority)
    {
        cout << "Priority: " << priority << endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessStartTime(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        SYSTEMTIME st;
        FileTimeToSystemTime(&creationTime, &st);
        cout << "Start Time: " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessCPUUsage(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    ULARGE_INTEGER lastKernel, lastUser, currentKernel, currentUser;
    SYSTEM_INFO sysInfo;
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;
    ULARGE_INTEGER lastSysKernel, lastSysUser, currentSysKernel, currentSysUser;

    GetSystemInfo(&sysInfo);
    int numProcessors = sysInfo.dwNumberOfProcessors;

    if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser))
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    memcpy(&lastKernel, &ftKernel, sizeof(FILETIME));
    memcpy(&lastUser, &ftUser, sizeof(FILETIME));

    GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser);
    memcpy(&lastSysKernel, &ftSysKernel, sizeof(FILETIME));
    memcpy(&lastSysUser, &ftSysUser, sizeof(FILETIME));

    Sleep(1000); // Wait for 1 second

    if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser))
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    memcpy(&currentKernel, &ftKernel, sizeof(FILETIME));
    memcpy(&currentUser, &ftUser, sizeof(FILETIME));

    GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser);
    memcpy(&currentSysKernel, &ftSysKernel, sizeof(FILETIME));
    memcpy(&currentSysUser, &ftSysUser, sizeof(FILETIME));

    ULONGLONG sysKernelDiff = (currentSysKernel.QuadPart - lastSysKernel.QuadPart) / numProcessors;
    ULONGLONG sysUserDiff = (currentSysUser.QuadPart - lastSysUser.QuadPart) / numProcessors;
    ULONGLONG procKernelDiff = currentKernel.QuadPart - lastKernel.QuadPart;
    ULONGLONG procUserDiff = currentUser.QuadPart - lastUser.QuadPart;

    double cpuUsage = (double)(procKernelDiff + procUserDiff) * 100.0 / (sysKernelDiff + sysUserDiff);

    cout << "CPU Usage: " << cpuUsage << "%" << endl;

    CloseHandle(hProcess);
    return TRUE;
}

BOOL ProcessInfo::GetProcessPath(char* processName)
{
    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return FALSE;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (_stricmp(pe32.szExeFile, processName) == 0)
            {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);

    if (processID == 0)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess)
        return FALSE;

    char processPath[MAX_PATH];
    if (GetModuleFileNameEx(hProcess, NULL, processPath, MAX_PATH))
    {
        cout << "Process Path: " << processPath << endl;
    }

    CloseHandle(hProcess);
    return TRUE;
}