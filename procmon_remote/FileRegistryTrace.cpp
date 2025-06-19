//
// Created by Tudor on 19.06.2025.
//

#include "FileRegistryTrace.hpp"
#include <QDir>
#include <chrono>

FileRegistryTrace::FileRegistryTrace(QObject *parent) : QObject(parent) {}

FileRegistryTrace::~FileRegistryTrace() {
    stopTrace();
}

bool FileRegistryTrace::startFileTrace(const QString &directory) {
    if (m_running) return false;
    m_hDir = CreateFileW((LPCWSTR)QDir(directory).absolutePath().utf16(),
                         FILE_LIST_DIRECTORY,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         nullptr,
                         OPEN_EXISTING,
                         FILE_FLAG_BACKUP_SEMANTICS,
                         nullptr);
    if (m_hDir == INVALID_HANDLE_VALUE) return false;
    m_running = true;
    m_fileThread = std::thread(&FileRegistryTrace::runFileMonitor, this, directory);
    return true;
}

bool FileRegistryTrace::startRegistryTrace(HKEY rootKey, const QString &subKey) {
    if (m_running) return false;
    if (RegOpenKeyExW(rootKey, (LPCWSTR)QString(subKey).utf16(), 0, KEY_NOTIFY, &m_hRegKey) != ERROR_SUCCESS)
        return false;
    m_running = true;
    m_regThread = std::thread(&FileRegistryTrace::runRegistryMonitor, this, rootKey, subKey);
    return true;
}

void FileRegistryTrace::stopTrace() {
    m_running = false;
    // close directory handle to unblock ReadDirectoryChangesW
    if (m_hDir != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDir);
        m_hDir = INVALID_HANDLE_VALUE;
    }
    // close registry key to unblock RegQueryInfoKey
    if (m_hRegKey) {
        RegCloseKey(m_hRegKey);
        m_hRegKey = nullptr;
    }
    // join threads
    if (m_fileThread.joinable()) m_fileThread.join();
    if (m_regThread.joinable()) m_regThread.join();
}

void FileRegistryTrace::runFileMonitor(const QString &directory) {
    const DWORD bufSize = 1024 * 10;
    std::vector<BYTE> buffer(bufSize);
    while (m_running) {
        DWORD bytes;
        BOOL res = ReadDirectoryChangesW(m_hDir, buffer.data(), bufSize, TRUE,
                                         FILE_NOTIFY_CHANGE_FILE_NAME |
                                         FILE_NOTIFY_CHANGE_DIR_NAME |
                                         FILE_NOTIFY_CHANGE_ATTRIBUTES |
                                         FILE_NOTIFY_CHANGE_SIZE |
                                         FILE_NOTIFY_CHANGE_LAST_WRITE,
                                         &bytes, nullptr, nullptr);
        if (!res) break;
        BYTE *ptr = buffer.data();
        while (true) {
            FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION *)ptr;
            QString name = QString::fromWCharArray(info->FileName, info->FileNameLength/2);
            QString action;
            switch (info->Action) {
                case FILE_ACTION_ADDED: action = "Added"; break;
                case FILE_ACTION_REMOVED: action = "Removed"; break;
                case FILE_ACTION_MODIFIED: action = "Modified"; break;
                case FILE_ACTION_RENAMED_OLD_NAME: action = "Renamed From"; break;
                case FILE_ACTION_RENAMED_NEW_NAME: action = "Renamed To"; break;
                default: action = "Unknown"; break;
            }
            emit fileEvent(action, QDir(directory).filePath(name));
            if (!info->NextEntryOffset) break;
            ptr += info->NextEntryOffset;
        }
    }
}

void FileRegistryTrace::runRegistryMonitor(HKEY /*rootKey*/, const QString &subKey) {
    while (m_running) {
        // wait for any change
        LONG notify = RegNotifyChangeKeyValue(m_hRegKey, TRUE,
            REG_NOTIFY_CHANGE_NAME |
            REG_NOTIFY_CHANGE_ATTRIBUTES |
            REG_NOTIFY_CHANGE_LAST_SET,
            nullptr, FALSE);
        if (notify != ERROR_SUCCESS) break;
        // enumerate values on change
        DWORD index = 0;
        WCHAR name[256]; DWORD nameLen;
        BYTE data[1024]; DWORD dataLen; DWORD type;
        while (m_running) {
            nameLen = 256; dataLen = sizeof(data);
            LONG res = RegEnumValueW(m_hRegKey, index, name, &nameLen,
                                     nullptr, &type, data, &dataLen);
            if (res == ERROR_NO_MORE_ITEMS) break;
            if (res == ERROR_SUCCESS) {
                QString valName = QString::fromWCharArray(name, nameLen);
                QString valData;
                if (type == REG_SZ) valData = QString::fromWCharArray((WCHAR*)data);
                emit registryEvent("ValueChanged", subKey + "/" + valName, valData);
            }
            ++index;
        }
    }
}
