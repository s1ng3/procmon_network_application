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
    ProcessInfo();
    BOOL ProcessDisplay();
    BOOL ProcessLog(char*);
    BOOL ProcessSearch(char*);
    BOOL KillProcess(char*);
    BOOL OpenGivenProcess();
    BOOL DisplayHardwareInfo();
    BOOL GetProcessMemoryUsage(char*);
    BOOL GetProcessUserName(char*);
    BOOL GetProcessStatus(char*);
    BOOL GetProcessDescription(char*);
    BOOL GetProcessPriority(char*);
    BOOL GetProcessStartTime(char*);
    BOOL GetProcessCPUUsage(char*);
    BOOL GetProcessPath(char*);
    void DisplayHelp();

};

#endif // PROCESSINFO_HPP