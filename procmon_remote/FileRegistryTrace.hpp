//
// Created by Tudor on 19.06.2025.
//

#ifndef PROCMON_REMOTE_FILEREGISTRYTRACE_HPP
#define PROCMON_REMOTE_FILEREGISTRYTRACE_HPP

#include <QObject>
#include <QString>
#include <windows.h>
#include <thread>
#include <atomic>

class FileRegistryTrace : public QObject
{
    Q_OBJECT
public:
    explicit FileRegistryTrace(QObject *parent = nullptr);
    ~FileRegistryTrace();

    bool startFileTrace(const QString &directory);
    bool startRegistryTrace(HKEY rootKey, const QString &subKey);
    void stopTrace();

signals:
    void fileEvent(const QString &action, const QString &path);
    void registryEvent(const QString &action, const QString &key, const QString &value);

private:
    void runFileMonitor(const QString &directory);
    void runRegistryMonitor(HKEY rootKey, const QString &subKey);

    HANDLE m_hDir{INVALID_HANDLE_VALUE};
    HKEY m_hRegKey{nullptr};
    std::atomic<bool> m_running{false};
    std::thread m_fileThread;
    std::thread m_regThread;
};

#endif //PROCMON_REMOTE_FILEREGISTRYTRACE_HPP
