#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ProcessInfo.hpp"
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QToolButton>
#include <QStyle>
#include <QMenuBar>
#include <sstream>
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <QStringList>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <unordered_map>
#include <QVector>
#include <QPair>
#include <QChart>
#include <QLineSeries>
#include <QChartView>
#include <QHBoxLayout>
#include <QtCharts/QValueAxis>
#include <QBrush>
#include <QColor>
#include <intrin.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <QFile>
#include <QCryptographicHash>
#include <QMap>
#include <QByteArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , processesVisible(false)
{
    ui->setupUi(this);
    // force initial size from UI geometry to be applied at runtime
    this->resize(900, 900);
    // allow resizing smaller than default minimum
    this->setMinimumSize(0, 0);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    // fix button sizes so text remains visible
    for (auto btn : ui->centralwidget->findChildren<QPushButton*>()) {
        auto hint = btn->sizeHint();
        btn->setMinimumSize(hint.width(), hint.height());
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    ui->tableProcesses->setMinimumHeight(0);
    ui->tableProcesses->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    ui->tabWidget->setMinimumHeight(0);
    ui->tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

    // UI styling enhancements for a modern dark theme
    this->setStyleSheet(
        "QMainWindow { background-color: #2e3440; color: #d8dee9; }"
        " QPushButton { background-color: #4c566a; border: none; border-radius: 4px; padding: 6px 12px; }"
        " QPushButton:hover { background-color: #5e81ac; }"
        " QPushButton:pressed { background-color: #81a1c1; }"
        " QTableWidget { background-color: #3b4252; color: #eceff4; gridline-color: #434c5e; }"
        " QHeaderView::section { background-color: #4c566a; color: #eceff4; }"
    );
    // Increase padding and spacing
    if (auto lay = ui->centralwidget->layout()) {
        lay->setContentsMargins(10, 10, 10, 10);
        lay->setSpacing(10);
        lay->setSizeConstraint(QLayout::SetNoConstraint);
    }

    // auto-stretch columns to fill table width and avoid horizontal scroll
    ui->tableProcesses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // restore column header labels
    ui->tableProcesses->setHorizontalHeaderLabels(QStringList() << "PID" << "Name" << "Memory" << "Threads" << "Status" << "User" << "Priority");
    ui->tableProcesses->hide(); // hide table initially

    // Enable sorting and connect the header click signal
    ui->tableProcesses->horizontalHeader()->setSortIndicatorShown(true);
    connect(ui->tableProcesses->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onHeaderClicked);

    if (!connect(ui->btnProcessDisplay, &QPushButton::clicked, this, &MainWindow::onProcessDisplay)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnProcessDisplay signal.");
    }
    if (!connect(ui->btnProcessLog, &QPushButton::clicked, this, &MainWindow::onProcessLog)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnProcessLog signal.");
    }
    if (!connect(ui->btnProcessSearch, &QPushButton::clicked, this, &MainWindow::onProcessSearch)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnProcessSearch signal.");
    }
    if (!connect(ui->btnKillProcess, &QPushButton::clicked, this, &MainWindow::onKillProcess)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnKillProcess signal.");
    }
    if (!connect(ui->btnOpenProcess, &QPushButton::clicked, this, &MainWindow::onOpenProcess)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnOpenProcess signal.");
    }
    if (!connect(ui->btnDisplayHardwareInfo, &QPushButton::clicked, this, &MainWindow::onDisplayHardwareInfo)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnDisplayHardwareInfo signal.");
    }
    if (!connect(ui->btnGetProcessMemoryUsage, &QPushButton::clicked, this, &MainWindow::onGetProcessMemoryUsage)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnGetProcessMemoryUsage signal.");
    }
    if (!connect(ui->btnGetProcessPath, &QPushButton::clicked, this, &MainWindow::onGetProcessPath)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnGetProcessPath signal.");
    }
    if (!connect(ui->btnFastLimitRAM, &QPushButton::clicked, this, &MainWindow::onFastLimitRAM)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnFastLimitRAM signal.");
    }
    // connect Limit With Job Objects button via findChild
    if (auto btnJob = ui->centralwidget->findChild<QPushButton*>("btnLimitJobObjects")) {
        if (!connect(btnJob, &QPushButton::clicked, this, &MainWindow::onLimitJobObjects)) {
            QMessageBox::critical(this, "Error", "Failed to connect Limit With Job Objects signal.");
        }
    } else {
        QMessageBox::critical(this, "Error", "Button Limit With Job Objects not found");
    }
    // connect Limit Logical Processor button via findChild
    if (auto btnLP = ui->centralwidget->findChild<QPushButton*>("btnLimitLogicalProcessors")) {
        if (!connect(btnLP, &QPushButton::clicked, this, &MainWindow::onLimitLogicalProcessors)) {
            QMessageBox::critical(this, "Error", "Failed to connect Limit Logical Processor signal.");
        }
    } else {
        QMessageBox::critical(this, "Error", "Button Limit Logical Processor not found");
    }
    if (!connect(ui->btnOptimizePerformance, &QPushButton::clicked, this, &MainWindow::onOptimizePerformance)) {
        QMessageBox::critical(this, "Error", "Failed to connect Optimize Process Performance signal.");
    }
    if (!connect(ui->btnResetAffinity, &QPushButton::clicked, this, &MainWindow::onResetAffinity)) {
        QMessageBox::critical(this, "Error", "Failed to connect Reset Affinity signal.");
    }
    if (!connect(ui->btnVerifyIntegrity, &QPushButton::clicked, this, &MainWindow::onVerifyIntegrity)) {
        QMessageBox::critical(this, "Error", "Failed to connect Verify Integrity signal.");
    }
    if (!connect(ui->btnExit, &QPushButton::clicked, this, &MainWindow::close)) {
        QMessageBox::critical(this, "Error", "Failed to connect btnExit signal.");
    }
    if (!connect(ui->tableProcesses, &QTableWidget::itemClicked, this, &MainWindow::onProcessSelected)) {
        QMessageBox::critical(this, "Error", "Failed to connect tableProcesses itemClicked signal.");
    }

    // Add refresh button in top-right corner
    btnRefresh = new QToolButton(this);
    btnRefresh->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    btnRefresh->setToolTip("Refresh Processes");
    connect(btnRefresh, &QToolButton::clicked, this, &MainWindow::refreshProcesses);
    menuBar()->setCornerWidget(btnRefresh, Qt::TopRightCorner);

    // Setup CPU utilization chart
    cpuChart = new QChart();
    cpuChart->setTheme(QChart::ChartThemeDark);
    cpuSeries = new QLineSeries();
    cpuChart->addSeries(cpuSeries);
    cpuChart->createDefaultAxes();
    cpuChart->setTitle("CPU % Optimization");
    {
        auto axesY = cpuChart->axes(Qt::Vertical);
        if (!axesY.isEmpty()) {
            if (auto axisY = qobject_cast<QValueAxis*>(axesY.first())) {
                axisY->setRange(0, 100);
                axisY->setTitleText("CPU %");
            }
        }
        auto axesX = cpuChart->axes(Qt::Horizontal);
        if (!axesX.isEmpty()) {
            if (auto axisX = qobject_cast<QValueAxis*>(axesX.first())) {
                axisX->setTitleText("Time");
            }
        }
    }
    cpuChartView = new QChartView(cpuChart, this);
    cpuChartView->setBackgroundBrush(QBrush(QColor(30,30,30)));
    cpuChartView->setMinimumSize(150, 80);
    cpuChartView->setMinimumHeight(0); // allow vertical shrink
    cpuChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored); // ignore size hint for height
    cpuChartView->show();

    // Stats panel setup
    statsWidget = new QWidget(this);
    { // style and initial visibility
        statsWidget->setStyleSheet("background-color: #3b4252; color: #eceff4; padding: 4px; border-radius:4px;");
        statsWidget->show(); // always visible
    }
    statsWidget->setMinimumHeight(0); // allow vertical shrink
    statsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored); // ignore size hint for height
    QFormLayout *statLayout = new QFormLayout(statsWidget);
    statLayout->setContentsMargins(2,2,2,2);
    statLayout->setSpacing(4);
    lblUtilization = new QLabel("---", statsWidget);
    lblProcesses   = new QLabel("---", statsWidget);
    lblThreads     = new QLabel("---", statsWidget);
    lblHandles     = new QLabel("---", statsWidget);
    lblUptime      = new QLabel("---", statsWidget);
    lblBaseSpeed   = new QLabel("--- MHz", statsWidget);
    lblSockets     = new QLabel("---", statsWidget);
    lblCores       = new QLabel("---", statsWidget);
    lblLogical     = new QLabel("---", statsWidget);
    lblVirtualization = new QLabel("---", statsWidget);
    lblL1Cache     = new QLabel("--- KB", statsWidget);
    lblL2Cache     = new QLabel("--- KB", statsWidget);
    lblL3Cache     = new QLabel("--- KB", statsWidget);
    statLayout->addRow("Utilization:", lblUtilization);
    statLayout->addRow("Processes:", lblProcesses);
    statLayout->addRow("Threads:", lblThreads);
    statLayout->addRow("Handles:", lblHandles);
    statLayout->addRow("Up time:", lblUptime);
    statLayout->addRow("Base speed:", lblBaseSpeed);
    statLayout->addRow("Sockets:", lblSockets);
    statLayout->addRow("Cores:", lblCores);
    statLayout->addRow("Logical processors:", lblLogical);
    statLayout->addRow("Virtualization:", lblVirtualization);
    statLayout->addRow("L1 cache:", lblL1Cache);
    statLayout->addRow("L2 cache:", lblL2Cache);
    statLayout->addRow("L3 cache:", lblL3Cache);

    // Wrap CPU chart and stats panel in a horizontal layout at top of main window
    if (auto mainLay = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout())) {
        QWidget* chartStatsWrapper = new QWidget(this);
        QHBoxLayout* csLayout = new QHBoxLayout(chartStatsWrapper);
        csLayout->setContentsMargins(2,2,2,2);
        csLayout->setSpacing(4);
        csLayout->addWidget(cpuChartView);
        csLayout->addWidget(statsWidget);
        csLayout->setSizeConstraint(QLayout::SetNoConstraint); // allow layout to shrink
        chartStatsWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored); // ignore minimum height
        mainLay->insertWidget(0, chartStatsWrapper);
    }

    // Compute static CPU/hardware info
    int cpuInfo[4] = {0}; __cpuid(cpuInfo, 0x16);
    baseSpeedMHz = cpuInfo[1]; lblBaseSpeed->setText(QString::number(baseSpeedMHz) + " MHz");
    SYSTEM_INFO sysInfo; GetSystemInfo(&sysInfo);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buf = nullptr; DWORD len = 0;
    GetLogicalProcessorInformation(nullptr, &len);
    buf = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(len);
    GetLogicalProcessorInformation(buf, &len);
    int phys=0, sockets=0;
    for (DWORD i=0; i < len/sizeof(*buf); ++i) {
        if (buf[i].Relationship == RelationProcessorCore) phys++;
        if (buf[i].Relationship == RelationProcessorPackage) sockets++;
        if (buf[i].Relationship == RelationCache) {
            auto &c = buf[i].Cache;
            switch(c.Level) {
            case 1: l1Size = c.Size/1024; break;
            case 2: l2Size = c.Size/1024; break;
            case 3: l3Size = c.Size/1024; break;
            }
        }
    }
    free(buf);
    physicalCores = phys; socketCount = sockets;
    lblCores->setText(QString::number(phys));
    lblLogical->setText(QString::number(sysInfo.dwNumberOfProcessors));
    lblSockets->setText(QString::number(sockets));
    lblL1Cache->setText(QString::number(l1Size) + " KB");
    lblL2Cache->setText(QString::number(l2Size) + " KB");
    lblL3Cache->setText(QString::number(l3Size) + " KB");
    virtualizationEnabled = IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED);
    lblVirtualization->setText(virtualizationEnabled?"Enabled":"Disabled");

    // Initialize CPU timer and data
    cpuTimer = new QTimer(this);
    connect(cpuTimer, &QTimer::timeout, this, &MainWindow::updateCpuUsage);
    connect(cpuTimer, &QTimer::timeout, this, &MainWindow::updateStats);
    cpuTimer->start(1000);
    FILETIME ftIdle, ftKernel, ftUser;
    GetSystemTimes(&ftIdle, &ftKernel, &ftUser);
    lastIdleTime = ((quint64)ftIdle.dwHighDateTime << 32) | ftIdle.dwLowDateTime;
    lastKernelTime = ((quint64)ftKernel.dwHighDateTime << 32) | ftKernel.dwLowDateTime;
    lastUserTime = ((quint64)ftUser.dwHighDateTime << 32) | ftUser.dwLowDateTime;
    cpuPointIndex = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProcessDisplay()
{
    if (!processesVisible) {
        ui->tableProcesses->show();
        processesVisible = true;
        ui->btnProcessDisplay->setText("Hide Processes");
        refreshProcesses();
    } else {
        ui->tableProcesses->hide();
        processesVisible = false;
        ui->btnProcessDisplay->setText("Display Processes");
    }
}

void MainWindow::refreshProcesses()
{
    if (!processesVisible) return;
    ProcessInfo processInfo;
    ui->tableProcesses->setRowCount(0);
    if (!Process32First(processInfo.getProcessSnap(), const_cast<LPPROCESSENTRY32>(&processInfo.getProcessEntry()))) {
        CloseHandle(processInfo.getProcessSnap());
        return;
    }
    int row = 0;
    do {
        ui->tableProcesses->insertRow(row);
        QString pid = QString::number(processInfo.getProcessEntry().th32ProcessID);
        QString name = QString::fromLocal8Bit(processInfo.getProcessEntry().szExeFile);
        QString threads = QString::number(processInfo.getProcessEntry().cntThreads);
        QString memory = "N/A";
        QString user = "N/A";
        QString priority = "N/A";
        QString status = "Running";
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processInfo.getProcessEntry().th32ProcessID);
        if (hProc) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                memory = QString::number(pmc.WorkingSetSize / 1024) + " KB";
            }
            HANDLE hToken;
            if (OpenProcessToken(hProc, TOKEN_QUERY, &hToken)) {
                DWORD userLen=0, domainLen=0;
                GetTokenInformation(hToken, TokenUser, nullptr, 0, &userLen);
                PTOKEN_USER ptu = (PTOKEN_USER)malloc(userLen);
                if (GetTokenInformation(hToken, TokenUser, ptu, userLen, &userLen)) {
                    char userName[256], domainName[256]; DWORD un=256, dn=256; SID_NAME_USE sidType;
                    if (LookupAccountSid(nullptr, ptu->User.Sid, userName, &un, domainName, &dn, &sidType)) {
                        user = QString("%1\\%2").arg(domainName).arg(userName);
                    }
                }
                free(ptu);
                CloseHandle(hToken);
            }
            DWORD pr = GetPriorityClass(hProc);
            if (pr) priority = QString::number(pr);
            CloseHandle(hProc);
        }
        ui->tableProcesses->setItem(row, 0, new QTableWidgetItem(pid));
        ui->tableProcesses->setItem(row, 1, new QTableWidgetItem(name));
        ui->tableProcesses->setItem(row, 2, new QTableWidgetItem(memory));
        ui->tableProcesses->setItem(row, 3, new QTableWidgetItem(threads));
        ui->tableProcesses->setItem(row, 4, new QTableWidgetItem(status));
        ui->tableProcesses->setItem(row, 5, new QTableWidgetItem(user));
        ui->tableProcesses->setItem(row, 6, new QTableWidgetItem(priority));
        row++;
    } while (Process32Next(processInfo.getProcessSnap(), const_cast<LPPROCESSENTRY32>(&processInfo.getProcessEntry())));
    CloseHandle(processInfo.getProcessSnap());
    ui->tableProcesses->resizeColumnsToContents();
    ui->tableProcesses->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::onProcessLog()
{
    QString processName = QInputDialog::getText(this, "Process Log", "Enter process name:");
    if (processName.isEmpty()) return;

    ProcessInfo processInfo;
    if (processInfo.ProcessLog(processName.toStdString().c_str())) {
        QMessageBox::information(this, "Process Log", "Process log created successfully.");
    } else {
        QMessageBox::critical(this, "Process Log", "Failed to create process log.");
    }
}

void MainWindow::onProcessSearch()
{
    QString processName = QInputDialog::getText(this, "Process Search", "Enter process name:");
    if (processName.isEmpty()) return;

    ProcessInfo processInfo;
    if (processInfo.ProcessSearch(processName.toStdString().c_str())) {
        QMessageBox::information(this, "Process Search", "Process found.");
    } else {
        QMessageBox::critical(this, "Process Search", "Process not found.");
    }
}

void MainWindow::onKillProcess()
{
    QString processName = QInputDialog::getText(this, "Kill Process", "Enter process name:");
    if (processName.isEmpty()) return;

    ProcessInfo processInfo;
    if (processInfo.KillProcess(processName.toStdString().c_str())) {
        QMessageBox::information(this, "Kill Process", "Process killed successfully.");
    } else {
        QMessageBox::critical(this, "Kill Process", "Failed to kill process.");
    }
}

void MainWindow::onDisplayHardwareInfo()
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    DWORD len = 0;
    GetLogicalProcessorInformation(nullptr, &len);
    auto procInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(len);
    GetLogicalProcessorInformation(procInfo, &len);
    int physicalCores = 0;
    DWORD count = len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (DWORD i = 0; i < count; ++i)
        if (procInfo[i].Relationship == RelationProcessorCore)
            ++physicalCores;
    free(procInfo);

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    // Build display string
    QString info;
    info += QString("Logical processors: %1\n").arg(sysInfo.dwNumberOfProcessors);
    info += QString("Physical cores: %1\n").arg(physicalCores);
    info += QString("Total physical memory: %1 MB\n").arg(mem.ullTotalPhys / (1024*1024));
    info += QString("Available physical memory: %1 MB\n").arg(mem.ullAvailPhys / (1024*1024));
    info += QString("Memory load: %1%\n").arg(mem.dwMemoryLoad);

    ui->textHardwareInfo->setPlainText(info);
}

// Implement stats update slot
void MainWindow::updateStats()
{
    // CPU utilization
    lblUtilization->setText(QString::number(lastCpuPercent, 'f', 1) + "%");

    // Processes, threads, handles
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);
    GetPerformanceInfo(&pi, sizeof(pi));
    lblProcesses->setText(QString::number(pi.ProcessCount));
    lblThreads->setText(QString::number(pi.ThreadCount));
    lblHandles->setText(QString::number(pi.HandleCount));

    // System uptime
    quint64 secs = GetTickCount64() / 1000;
    int days = secs / 86400; secs %= 86400;
    int hrs  = secs / 3600;   secs %= 3600;
    int mins = secs / 60;     secs %= 60;
    lblUptime->setText(QString("%1d %2h %3m %4s").arg(days).arg(hrs).arg(mins).arg(secs));
}

// Add implementations for GUI slots
void MainWindow::onOpenProcess()
{
    QString processPath = QInputDialog::getText(this, "Open Process", "Enter process path or executable name:");
    if (processPath.isEmpty()) return;
    HINSTANCE result = ShellExecuteW(nullptr, L"open", reinterpret_cast<LPCWSTR>(processPath.utf16()), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        QMessageBox::critical(this, "Open Process", "Failed to open process.");
    } else {
        QMessageBox::information(this, "Open Process", "Process opened successfully.");
    }
}

void MainWindow::onGetProcessMemoryUsage()
{
    QString processName = QInputDialog::getText(this, "Process Memory Usage", "Enter process name:");
    if (processName.isEmpty()) return;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    bool found = false;
    SIZE_T memKB = 0;
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                if (hProc) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                        memKB = pmc.WorkingSetSize / 1024;
                        found = true;
                    }
                    CloseHandle(hProc);
                }
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (found) {
        QMessageBox::information(this, "Memory Usage", QString("%1 is using %2 KB").arg(processName).arg(memKB));
    } else {
        QMessageBox::warning(this, "Memory Usage", "Process not found or access denied.");
    }
}

void MainWindow::onGetProcessPath()
{
    QString processName = QInputDialog::getText(this, "Get Process Path", "Enter process name:");
    if (processName.isEmpty()) return;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    bool found = false;
    QString path;
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                if (hProc) {
                    char exePath[MAX_PATH];
                    if (GetModuleFileNameExA(hProc, nullptr, exePath, MAX_PATH)) {
                        path = QString::fromLocal8Bit(exePath);
                        found = true;
                    }
                    CloseHandle(hProc);
                }
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (found) {
        QMessageBox::information(this, "Process Path", path);
    } else {
        QMessageBox::warning(this, "Process Path", "Failed to get process path.");
    }
}

void MainWindow::onFastLimitRAM()
{
    QString processName = QInputDialog::getText(this, "Fast Limit RAM", "Enter process name:");
    if (processName.isEmpty()) return;
    bool ok = false;
    int limitKB = QInputDialog::getInt(this, "Fast Limit RAM", "Enter memory limit (KB):", 1024, 1, INT_MAX, 1, &ok);
    if (!ok) return;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    bool success = false;
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_SET_QUOTA, FALSE, pe.th32ProcessID);
                if (hProc) {
                    SIZE_T bytes = static_cast<SIZE_T>(limitKB) * 1024;
                    if (SetProcessWorkingSetSize(hProc, bytes, bytes)) success = true;
                    CloseHandle(hProc);
                }
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (success) {
        QMessageBox::information(this, "Fast Limit RAM", "Memory limit applied successfully.");
    } else {
        QMessageBox::warning(this, "Fast Limit RAM", "Failed to apply memory limit.");
    }
}

void MainWindow::onLimitJobObjects()
{
    QString processName = QInputDialog::getText(this, "Limit With Job Objects", "Enter process name:");
    if (processName.isEmpty()) return;
    bool ok = false;
    int limitKB = QInputDialog::getInt(this, "Limit With Job Objects", "Enter memory limit (KB):", 1024, 1, INT_MAX, 1, &ok);
    if (!ok) return;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    bool success = false;
    DWORD pid = 0;
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (pid) {
        HANDLE hProc = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, pid);
        if (hProc) {
            HANDLE hJob = CreateJobObject(nullptr, nullptr);
            if (hJob) {
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
                SIZE_T bytes = static_cast<SIZE_T>(limitKB) * 1024;
                info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY | JOB_OBJECT_LIMIT_JOB_MEMORY;
                info.ProcessMemoryLimit = bytes;
                info.JobMemoryLimit = bytes;
                if (SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &info, sizeof(info)) &&
                    AssignProcessToJobObject(hJob, hProc)) {
                    success = true;
                }
                CloseHandle(hJob);
            }
            CloseHandle(hProc);
        }
    }
    if (success) {
        QMessageBox::information(this, "Limit With Job Objects", "Job object memory limit applied.");
    } else {
        QMessageBox::warning(this, "Limit With Job Objects", "Failed to limit memory with job object.");
    }
}

void MainWindow::onLimitLogicalProcessors()
{
    QString processName = QInputDialog::getText(this, "Limit Logical Processors", "Enter process name:");
    if (processName.isEmpty()) return;
    QString maskStr = QInputDialog::getText(this, "Limit Logical Processors", "Enter core mask (hex, e.g., 0xA9):");
    if (maskStr.isEmpty()) return;
    bool ok = false;
    unsigned long mask = maskStr.toULong(&ok, 0);
    if (!ok) return;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    bool success = false;
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProc) {
                    if (SetProcessAffinityMask(hProc, mask)) success = true;
                    CloseHandle(hProc);
                }
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (success) {
        QMessageBox::information(this, "Limit Logical Processors", "Processor affinity mask set.");
    } else {
        QMessageBox::warning(this, "Limit Logical Processors", "Failed to set affinity mask.");
    }
}

void MainWindow::onOptimizePerformance()
{
    // Get processor count and masks
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    int n = si.dwNumberOfProcessors;
    DWORD_PTR fullMask = (n < sizeof(DWORD_PTR)*8) ? ((1ULL<<n)-1) : (DWORD_PTR)-1;
    DWORD_PTR halfMask = fullMask & ~((1ULL<<(n/2))-1); // upper half
    DWORD_PTR oneMask  = 1;                              // core 0

    // Snapshot
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snap==INVALID_HANDLE_VALUE){
        QMessageBox::warning(this,"Optimize Performance","Failed to snapshot processes.");
        return;
    }

    // Skip list
    const QStringList blacklist = {
            "system","smss.exe","csrss.exe","wininit.exe",
            "services.exe","lsass.exe","svchost.exe",
            "explorer.exe","winlogon.exe"
    };

    PROCESSENTRY32 pe{sizeof(pe)};
    int updated=0;
    if(Process32First(snap,&pe)){
        do{
            QString exe = QString::fromLocal8Bit(pe.szExeFile).toLower();
            if(blacklist.contains(exe)) continue;

            HANDLE h = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
            if(!h) continue;

            DWORD pr = GetPriorityClass(h);
            DWORD_PTR mask = oneMask;
            if(pr==IDLE_PRIORITY_CLASS){
                mask = oneMask;
            } else if(pr==BELOW_NORMAL_PRIORITY_CLASS || pr==NORMAL_PRIORITY_CLASS){
                mask = halfMask;
            } else {
                mask = fullMask;
            }

            if(SetProcessAffinityMask(h, mask)) ++updated;
            CloseHandle(h);
        } while(Process32Next(snap,&pe));
    }
    CloseHandle(snap);

    QMessageBox::information(
            this,
            "Optimize Performance",
            QString("%1 processes updated").arg(updated)
    );
}


void MainWindow::onResetAffinity()
{
    // Determine full mask for all logical processors
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD_PTR fullMask = (sysInfo.dwNumberOfProcessors < sizeof(DWORD_PTR)*8)
                         ? ((1ULL << sysInfo.dwNumberOfProcessors) - 1)
                         : static_cast<DWORD_PTR>(-1);

    // Iterate all processes and apply the mask
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(snap, &pe)) {
            do {
                HANDLE proc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (proc) {
                    SetProcessAffinityMask(proc, fullMask);
                    CloseHandle(proc);
                }
            } while (Process32Next(snap, &pe));
        }
        CloseHandle(snap);
    }

    // Notify the user
    QMessageBox::information(this,
                             "Reset Affinity",
                             "Affinity mask reset for all processes.");
}

void MainWindow::onVerifyIntegrity()
{
    QString processName = QInputDialog::getText(this, "Verify Integrity", "Enter process name:");
    if (processName.isEmpty()) return;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    QString path;
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                if (hProc) {
                    char exePath[MAX_PATH];
                    if (GetModuleFileNameExA(hProc, nullptr, exePath, MAX_PATH)) path = QString::fromLocal8Bit(exePath);
                    CloseHandle(hProc);
                }
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Verify Integrity", "Process not found.");
        return;
    }
    static QMap<QString, QByteArray> hashes;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Verify Integrity", "Failed to open file.");
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    if (hashes.contains(processName)) {
        if (hashes[processName] == hash) {
            QMessageBox::information(this, "Verify Integrity", "Integrity verified (hash matches).");
        } else {
            QMessageBox::warning(this, "Verify Integrity", "Integrity check failed (hash mismatch).");
        }
    } else {
        hashes[processName] = hash;
        QMessageBox::information(this, "Verify Integrity", QString("Hash stored: %1").arg(QString(hash.toHex())));
    }
}

void MainWindow::onProcessSelected(QTableWidgetItem* item)
{
    int row = item->row();
    QString pidStr = ui->tableProcesses->item(row, 0)->text();
    DWORD pid = pidStr.toUInt();
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    QString path = "N/A";
    if (hProc) {
        char exe[MAX_PATH];
        if (GetModuleFileNameExA(hProc, nullptr, exe, MAX_PATH))
            path = QString::fromLocal8Bit(exe);
        CloseHandle(hProc);
    }
    QString info = QString("PID: %1\nName: %2\nMemory: %3\nThreads: %4\nUser: %5\nPriority: %6\nPath: %7")
        .arg(pidStr)
        .arg(ui->tableProcesses->item(row, 1)->text())
        .arg(ui->tableProcesses->item(row, 2)->text())
        .arg(ui->tableProcesses->item(row, 3)->text())
        .arg(ui->tableProcesses->item(row, 5)->text())
        .arg(ui->tableProcesses->item(row, 6)->text())
        .arg(path);
    QMessageBox::information(this, "Process Details", info);
}

void MainWindow::onHeaderClicked(int column)
{
    auto header = ui->tableProcesses->horizontalHeader();
    Qt::SortOrder order = header->sortIndicatorOrder() == Qt::AscendingOrder
        ? Qt::DescendingOrder
        : Qt::AscendingOrder;
    ui->tableProcesses->sortItems(column, order);
    header->setSortIndicator(column, order);
}

void MainWindow::updateCpuUsage()
{
    // Get current system times
    FILETIME ftIdle, ftKernel, ftUser;
    GetSystemTimes(&ftIdle, &ftKernel, &ftUser);
    quint64 idle = ((quint64)ftIdle.dwHighDateTime << 32) | ftIdle.dwLowDateTime;
    quint64 kernel = ((quint64)ftKernel.dwHighDateTime << 32) | ftKernel.dwLowDateTime;
    quint64 user = ((quint64)ftUser.dwHighDateTime << 32) | ftUser.dwLowDateTime;

    // Calculate deltas
    quint64 idleDiff = idle - lastIdleTime;
    quint64 kernelDiff = kernel - lastKernelTime;
    quint64 userDiff = user - lastUserTime;
    quint64 total = kernelDiff + userDiff;
    quint64 busy = total - idleDiff;
    double cpuPercent = total > 0 ? (busy * 100.0) / total : 0.0;

    // Update last times
    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;

    // Append new point
    cpuSeries->append(cpuPointIndex, cpuPercent);
    cpuPointIndex++;

    // Keep only latest 60 points
    const int maxPoints = 60;
    if (cpuSeries->count() > maxPoints) {
        cpuSeries->removePoints(0, cpuSeries->count() - maxPoints);
    }

    // update horizontal axis range without deprecated call
    if (cpuSeries->count() > 0) {
        int start = cpuPointIndex - cpuSeries->count();
        auto axesX2 = cpuChart->axes(Qt::Horizontal);
        if (!axesX2.isEmpty()) {
            if (auto axisX2 = qobject_cast<QValueAxis*>(axesX2.first())) {
                axisX2->setRange(start, start + maxPoints);
            }
        }
    }
}
