//
// Created by Tudor on 19.06.2025.
//

#ifndef PROCMON_REMOTE_HANDLEDLLINSPECTION_HPP
#define PROCMON_REMOTE_HANDLEDLLINSPECTION_HPP

#include <Windows.h>
#include <QString>
#include <vector>

struct HandleInfo {
    DWORD pid;
    ULONG handle;
    QString type;
    QString name;
};

struct ModuleInfo {
    DWORD pid;
    HMODULE hModule;
    QString moduleName;
    QString modulePath;
};

class HandleDLLInspection {
public:
    static bool getHandles(DWORD pid, std::vector<HandleInfo>& handles);
    static bool getModules(DWORD pid, std::vector<ModuleInfo>& modules);
};

#endif //PROCMON_REMOTE_HANDLEDLLINSPECTION_HPP
