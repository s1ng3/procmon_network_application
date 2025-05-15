//Author: Tudor-Cristian Sîngerean
//Date: 07.12.2024

#ifndef PROCESSINFO_HPP
#define PROCESSINFO_HPP

#include <bits/stdc++.h>
#include <sys/stat.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <io.h>
#include <psapi.h>
#include <ws2tcpip.h>

#include "RaspberryProcesses.hpp"


using namespace std;

typedef struct LogFile
{
    char ProcessName[100];
    unsigned int pid;
    unsigned int ppid;
    unsigned int thread_cnt;
} LOGFILE;

class ThreadInfo
{
private:
    DWORD PID;
    HANDLE hThreadSnap;
    THREADENTRY32 te32;
public:
    ThreadInfo(DWORD);
    BOOL ThreadsDisplay();
};

class DLLInfo
{
private:
    DWORD PID;
    MODULEENTRY32 me32;
    HANDLE hProcessSnap;
public:
    DLLInfo(DWORD);
    BOOL DependentDLLDisplay();
};

class ProcessInfo
{
private:
    DWORD PID;
    DLLInfo* pdobj;
    ThreadInfo* ptobj;
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
public:
    bool ProcessLog(const char *processName);
    bool ProcessSearch(const char *processName);
    bool KillProcess(const char *processName);
    bool GetProcessMemoryUsage(const char *processName);
    bool GetProcessUserName(const char *processName);
    bool GetProcessStatus(const char *processName);
    bool GetProcessDescription(const char *processName);
    bool GetProcessPriority(const char *processName);
    bool GetProcessStartTime(const char *processName);
    bool GetProcessCPUUsage(const char *processName);
    bool GetProcessPath(const char *processName);
    bool VerifyProcessIntegrity(const char *processName);

    void DisplayHelp();

    ProcessInfo();

    bool ProcessDisplay();
    bool DisplayHardwareInfo();
    bool FastLimitRAM();
    bool LimitRAMWithJobObjects();
    bool LimitLogicalProcessors();
    bool OpenGivenProcess();
    bool DisplayRaspberryProcesses();
    bool LimitArduinoPowerForAnalogAndDigital();
    bool OptimizeProcessPerformance();
};

#endif // PROCESSINFO_HPP