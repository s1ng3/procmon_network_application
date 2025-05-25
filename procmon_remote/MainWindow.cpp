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
#include <QHBoxLayout>  // added for horizontal layout casting
#include <QtCharts/QValueAxis>
#include <QBrush>
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , processesVisible(false) // Initialize processesVisible
{
    ui->setupUi(this);
    // auto-stretch columns to fill table width and avoid horizontal scroll
    ui->tableProcesses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
    // replace deprecated axisX()/axisY() with QValueAxis retrieval
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
    cpuChartView->setMinimumSize(600, 300); // wider and higher for better visibility
    cpuChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cpuChartView->show();  // show chart initially when processes are hidden
    // Insert chart in main horizontal layout, left of buttons, center vertically
    {
        QWidget* page = ui->tabWidget->widget(0); // Processes tab
        if (auto vLayout = qobject_cast<QVBoxLayout*>(page->layout())) {
            if (auto hLayout = qobject_cast<QHBoxLayout*>(vLayout->itemAt(0)->layout())) {
                // Wrap chart in a widget with vertical layout to center it
                QWidget* chartWrapper = new QWidget(this);
                QVBoxLayout* chartVLayout = new QVBoxLayout(chartWrapper);
                chartVLayout->addStretch();
                chartVLayout->addWidget(cpuChartView);
                chartVLayout->addStretch();
                chartWrapper->setLayout(chartVLayout);
                // insert before spacer (index 1) for middle-left positioning
                hLayout->insertWidget(1, chartWrapper);
            }
        }
    }
    // Initialize CPU timer and data
    cpuTimer = new QTimer(this);
    connect(cpuTimer, &QTimer::timeout, this, &MainWindow::updateCpuUsage);
    cpuTimer->start(1000);  // start updating chart immediately
    // get initial system times
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
        // hide chart if visible
        cpuTimer->stop();
        cpuChartView->hide();
        ui->tableProcesses->show(); // show table when displaying
        // Display processes
        ProcessInfo processInfo;
        ui->tableProcesses->setRowCount(0);
        if (!Process32First(processInfo.getProcessSnap(), const_cast<LPPROCESSENTRY32>(&processInfo.getProcessEntry()))) {
            QMessageBox::critical(this, "Process Display", "Failed to retrieve processes.");
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
            // Get memory, user and priority
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processInfo.getProcessEntry().th32ProcessID);
            if (hProc) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProc, &pmc, sizeof(pmc))) {
                    memory = QString::number(pmc.WorkingSetSize / 1024) + " KB";
                }
                // User
                HANDLE hToken;
                if (OpenProcessToken(hProc, TOKEN_QUERY, &hToken)) {
                    DWORD userLen = 0, domainLen = 0;
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
                // Priority
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
        // adjust column widths to contents and stretch last column to fill remaining space
        ui->tableProcesses->resizeColumnsToContents();
        ui->tableProcesses->horizontalHeader()->setStretchLastSection(true);
        processesVisible = true;
        ui->btnProcessDisplay->setText("Hide Processes");
    } else {
        ui->tableProcesses->setRowCount(0);
        ui->tableProcesses->hide(); // hide table when hiding
        // show CPU chart
        cpuChartView->show();
        cpuTimer->start(1000);
        processesVisible = false;
        ui->btnProcessDisplay->setText("Display Processes");
    }
}

void MainWindow::refreshProcesses()
{
    if (!processesVisible) return;
    // same logic as onProcessDisplay display branch
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
    // adjust column widths after refresh
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
    ProcessInfo processInfo;
    std::ostringstream hardwareInfo;

    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    DWORD length = 0;
    GetLogicalProcessorInformation(nullptr, &length);
    auto buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(length));
    GetLogicalProcessorInformation(buffer, &length);

    int coreCount = 0;
    DWORD count = length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (DWORD i = 0; i < count; ++i) {
        if (buffer[i].Relationship == RelationProcessorCore) {
            coreCount++;
        }
    }
    free(buffer);

    hardwareInfo << "Hardware information:\n";
    hardwareInfo << "  Number of logical processors: " << coreCount << "\n";
    hardwareInfo << "  Number of processors: " << siSysInfo.dwNumberOfProcessors << "\n";
    hardwareInfo << "  OEM ID: " << siSysInfo.dwOemId << "\n";
    hardwareInfo << "  Page size: " << siSysInfo.dwPageSize << "\n";
    hardwareInfo << "  Processor type: " << siSysInfo.dwProcessorType << "\n";
    hardwareInfo << "  Minimum application address: " << siSysInfo.lpMinimumApplicationAddress << "\n";
    hardwareInfo << "  Maximum application address: " << siSysInfo.lpMaximumApplicationAddress << "\n";
    hardwareInfo << "  Active processor mask: " << siSysInfo.dwActiveProcessorMask << "\n";

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex)) {
        hardwareInfo << "  Total physical memory: " << statex.ullTotalPhys / (1024 * 1024) << " MB\n";
    }

    ui->textHardwareInfo->setPlainText(QString::fromStdString(hardwareInfo.str()));
}

void MainWindow::onGetProcessMemoryUsage()
{
    QString processName = QInputDialog::getText(this, "Get Process Memory Usage", "Enter process name:");
    if (processName.isEmpty()) return;

    DWORD processID = 0;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Error", "Failed to create process snapshot.");
        return;
    }
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &pe32)) {
        do {
            QString exeFile = QString::fromLocal8Bit(pe32.szExeFile);
            if (exeFile.compare(processName, Qt::CaseInsensitive) == 0) {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }
    CloseHandle(hProcessSnap);
    if (processID == 0) {
        QMessageBox::critical(this, "Process Memory Usage", "Process not found.");
        return;
    }
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) {
        QMessageBox::critical(this, "Process Memory Usage", "Failed to open process.");
        return;
    }
    PROCESS_MEMORY_COUNTERS pmc;
    SIZE_T memUsageKB = 0;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        memUsageKB = pmc.WorkingSetSize / 1024;
    }
    CloseHandle(hProcess);
    QMessageBox::information(this, "Process Memory Usage", QString("Memory Usage: %1 KB").arg(memUsageKB));
}

void MainWindow::onGetProcessPath()
{
    QString processName = QInputDialog::getText(this, "Get Process Path", "Enter process name:");
    if (processName.isEmpty()) return;
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Get Process Path", "Failed to create process snapshot.");
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(hSnap, &pe)) {
        do {
            QString exe = QString::fromLocal8Bit(pe.szExeFile);
            if (exe.compare(processName, Qt::CaseInsensitive) == 0) {
                processID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (processID == 0) {
        QMessageBox::critical(this, "Get Process Path", "Process not found.");
        return;
    }
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProc) {
        QMessageBox::critical(this, "Get Process Path", "Failed to open process.");
        return;
    }
    wchar_t pathBuf[MAX_PATH];
    if (GetModuleFileNameExW(hProc, nullptr, pathBuf, MAX_PATH)) {
        QString path = QString::fromWCharArray(pathBuf);
        QMessageBox::information(this, "Process Path", path);
    } else {
        QMessageBox::critical(this, "Get Process Path", "Failed to retrieve process path.");
    }
    CloseHandle(hProc);
}

void MainWindow::onOpenProcess()
{
    QString processPath = QInputDialog::getText(this, "Open Process", "Enter full path of executable to open:");
    if (processPath.isEmpty())
        return;
    // Attempt to open the executable
    HINSTANCE result = ShellExecuteA(nullptr, "open", processPath.toLocal8Bit().constData(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        QMessageBox::critical(this, "Open Process", "Process failed to open.");
    } else {
        QMessageBox::information(this, "Open Process", "Process successfully opened.");
    }
}

void MainWindow::onProcessSelected(QTableWidgetItem* item)
{
    int row = item->row();
    QString processName = ui->tableProcesses->item(row, 1)->text();
    DWORD processID = ui->tableProcesses->item(row, 0)->text().toUInt();

    ProcessInfo processInfo;
    QString memoryUsage = "N/A"; // Declare memoryUsage
    QString parentID = "N/A";
    QString threadCount = "N/A";
    QString cores = "N/A";

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess != nullptr) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            memoryUsage = QString::number(pmc.WorkingSetSize / 1024) + " KB";
        }

        DWORD_PTR processAffinityMask, systemAffinityMask;
        if (GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask)) {
            cores = "";
            for (DWORD_PTR mask = 1, core = 0; mask != 0; mask <<= 1, ++core) {
                if (processAffinityMask & mask) {
                    cores += QString::number(core) + " ";
                }
            }
        }
        CloseHandle(hProcess);
    }

    if (Process32First(processInfo.getProcessSnap(), const_cast<LPPROCESSENTRY32>(&processInfo.getProcessEntry()))) {
        do {
            if (processInfo.getProcessEntry().th32ProcessID == processID) {
                parentID = QString::number(processInfo.getProcessEntry().th32ParentProcessID);
                threadCount = QString::number(processInfo.getProcessEntry().cntThreads);
                break;
            }
        } while (Process32Next(processInfo.getProcessSnap(), const_cast<LPPROCESSENTRY32>(&processInfo.getProcessEntry())));
    }

    QString processInfoText = QString("PROCESS NAME: %1\nPID: %2\nParent ID: %3\nNo of Thread: %4\nMemory Usage: %5\nRunning on cores: %6")
        .arg(processName)
        .arg(processID)
        .arg(parentID)
        .arg(threadCount)
        .arg(memoryUsage)
        .arg(cores);

    QMessageBox::information(this, "Process Information", processInfoText);
}

void MainWindow::onHeaderClicked(int column)
{
    static Qt::SortOrder currentSortOrder = Qt::AscendingOrder;

    // Toggle sort order
    currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;

    ui->tableProcesses->sortItems(column, currentSortOrder);
    ui->tableProcesses->horizontalHeader()->setSortIndicator(column, currentSortOrder);
}

void MainWindow::onFastLimitRAM()
{
    QString procName = QInputDialog::getText(this, "Fast Limit RAM", "Enter process name:");
    if (procName.isEmpty()) return;
    bool ok = false;
    int limitKb = QInputDialog::getInt(this, "Fast Limit RAM", "Enter memory limit (KB):", 0, 1, INT_MAX, 1, &ok);
    if (!ok) return;
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Fast Limit RAM", "Failed to create process snapshot");
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(procName, Qt::CaseInsensitive) == 0) {
                processID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (processID == 0) {
        QMessageBox::warning(this, "Fast Limit RAM", "Process not found");
        return;
    }
    HANDLE hProc = OpenProcess(PROCESS_SET_QUOTA, FALSE, processID);
    if (!hProc) {
        QMessageBox::critical(this, "Fast Limit RAM", "Unable to open process");
        return;
    }
    SIZE_T memBytes = static_cast<SIZE_T>(limitKb) * 1024;
    if (!SetProcessWorkingSetSize(hProc, memBytes, memBytes)) {
        QMessageBox::critical(this, "Fast Limit RAM", "Failed to set memory limit");
        CloseHandle(hProc);
        return;
    }
    CloseHandle(hProc);
    QMessageBox::information(this, "Fast Limit RAM",
        QString("Process \"") + procName + "\" memory limited to " + QString::number(limitKb) + " KB");
}

void MainWindow::onLimitJobObjects()
{
    QString procName = QInputDialog::getText(this, "Limit With Job Objects", "Enter process name:");
    if (procName.isEmpty()) return;
    bool ok = false;
    int limitKb = QInputDialog::getInt(this, "Limit With Job Objects", "Enter memory limit (KB):", 0, 1, INT_MAX, 1, &ok);
    if (!ok) return;
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Limit With Job Objects", "Failed to create process snapshot");
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (Process32First(hSnap, &pe)) {
        do {
            if (QString::fromLocal8Bit(pe.szExeFile).compare(procName, Qt::CaseInsensitive) == 0) {
                processID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    if (processID == 0) {
        QMessageBox::warning(this, "Limit With Job Objects", "Process not found");
        return;
    }
    HANDLE hProc = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, processID);
    if (!hProc) {
        QMessageBox::critical(this, "Limit With Job Objects", "Unable to open process");
        return;
    }
    HANDLE hJob = CreateJobObject(nullptr, nullptr);
    if (!hJob) {
        QMessageBox::critical(this, "Limit With Job Objects", "Failed to create job object");
        CloseHandle(hProc);
        return;
    }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
    SIZE_T memBytes = static_cast<SIZE_T>(limitKb) * 1024;
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    jeli.ProcessMemoryLimit = memBytes;
    jeli.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
    jeli.JobMemoryLimit = memBytes;
    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
        QMessageBox::critical(this, "Limit With Job Objects", "Failed to set job object limits");
        CloseHandle(hJob);
        CloseHandle(hProc);
        return;
    }
    if (!AssignProcessToJobObject(hJob, hProc)) {
        QMessageBox::critical(this, "Limit With Job Objects", "Failed to assign process to job");
        CloseHandle(hJob);
        CloseHandle(hProc);
        return;
    }
    CloseHandle(hJob);
    CloseHandle(hProc);
    QMessageBox::information(this, "Limit With Job Objects",
        QString("Process \"") + procName + "\" memory limited to " + QString::number(limitKb) + " KB");
}

void MainWindow::onLimitLogicalProcessors()
{
    QString procName = QInputDialog::getText(this, "Limit Logical Processor", "Enter process name:");
    if (procName.isEmpty()) return;
    // Determine number of logical processors
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int nProcs = sysInfo.dwNumberOfProcessors;
    if (nProcs > 16) nProcs = 16; // clamp to 16 cores
    qulonglong maxMask = ((qulonglong)1 << nProcs) - 1;
    // Build mask list with descriptions
    QStringList maskList;
    for (qulonglong i = 1; i <= maxMask; ++i) {
        QString maskHex = QString("0x%1").arg(i, 0, 16).toUpper();
        QStringList coreList;
        for (int core = 0; core < nProcs; ++core) {
            if (i & (1ULL << core)) coreList << QString::number(core);
        }
        maskList << QString("%1 - Logical Processor%2 %3")
                        .arg(maskHex)
                        .arg(coreList.size() > 1 ? "s" : "")
                        .arg(coreList.join(","));
    }
    bool ok = false;
    QString maskStr = QInputDialog::getItem(this, "Limit Logical Processor", "Select core mask:", maskList, 0, false, &ok);
    if (!ok || maskStr.isEmpty()) return;
    // Extract hex mask from selection
    QString maskHexOnly = maskStr.section(' ', 0, 0);
    bool convOk = false;
    DWORD_PTR mask = maskHexOnly.toULongLong(&convOk, 16);
    if (!convOk) return;
    // Find process ID
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(hSnap, &pe)) {
            do {
                if (QString::fromLocal8Bit(pe.szExeFile).compare(procName, Qt::CaseInsensitive) == 0) {
                    processID = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    if (processID == 0) {
        QMessageBox::warning(this, "Limit Logical Processor", "Process not found");
        return;
    }
    HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, processID);
    if (!hProc) {
        QMessageBox::critical(this, "Limit Logical Processor", "Unable to open process");
        return;
    }
    if (!SetProcessAffinityMask(hProc, mask)) {
        QMessageBox::critical(this, "Limit Logical Processor", "Failed to set process affinity mask");
        CloseHandle(hProc);
        return;
    }
    CloseHandle(hProc);
    QMessageBox::information(this, "Limit Logical Processor",
        QString("Process \"") + procName + "\" affinity mask set to " + maskHexOnly);
}

void MainWindow::onOptimizePerformance()
{
    // Gather eligible processes with their old affinity and priority weight
    struct Proc { DWORD pid; DWORD_PTR oldMask; DWORD_PTR newMask; int weight; QString name; };
    QVector<Proc> procs;
    SYSTEM_INFO sysInfo; GetSystemInfo(&sysInfo);
    DWORD numCores = sysInfo.dwNumberOfProcessors;
    // Blacklist
    static const QStringList blacklist = {"System","smss.exe","csrss.exe","wininit.exe","services.exe","lsass.exe","svchost.exe","explorer.exe","winlogon.exe"};
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
        if (Process32First(hSnap, &pe)) do {
            QString name = QString::fromLocal8Bit(pe.szExeFile);
            if (blacklist.contains(name, Qt::CaseInsensitive)) continue;
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
            if (!hProc) continue;
            DWORD_PTR oldMask=0, sysMask;
            GetProcessAffinityMask(hProc, &oldMask, &sysMask);
            DWORD pr = GetPriorityClass(hProc);
            int weight = 3;
            if (pr==HIGH_PRIORITY_CLASS) weight=5;
            else if (pr==ABOVE_NORMAL_PRIORITY_CLASS) weight=4;
            else if (pr==NORMAL_PRIORITY_CLASS) weight=3;
            else if (pr==BELOW_NORMAL_PRIORITY_CLASS) weight=2;
            else if (pr==IDLE_PRIORITY_CLASS) weight=1;
            procs.append({pe.th32ProcessID, oldMask, 0, weight, name});
            CloseHandle(hProc);
        } while (Process32Next(hSnap, &pe));
        CloseHandle(hSnap);
    }
    // Compute total weight
    int totalWeight = 0;
    for (auto &p : procs) totalWeight += p.weight;
    // Assign new masks proportionally
    DWORD_PTR coreIndex = 0;
    for (auto &p : procs) {
        int count = qMax(1, int((double)p.weight / totalWeight * numCores + 0.5));
        DWORD_PTR mask = 0;
        for (int i = 0; i < count; ++i) {
            mask |= (1ULL << (coreIndex % numCores));
            ++coreIndex;
        }
        p.newMask = mask;
        HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, p.pid);
        if (hProc) { SetProcessAffinityMask(hProc, mask); CloseHandle(hProc); }
    }
    // Show results
    QDialog dlg(this);
    dlg.setWindowTitle("Optimize Performance Results");
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QTableWidget* table = new QTableWidget(procs.size(), 3, &dlg);
    table->setHorizontalHeaderLabels({"Process","Old Mask","New Mask"});
    for (int i = 0; i < procs.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(procs[i].name));
        table->setItem(i, 1, new QTableWidgetItem(QString("0x%1").arg((ulong)procs[i].oldMask,0,16).toUpper()));
        table->setItem(i, 2, new QTableWidgetItem(QString("0x%1").arg((ulong)procs[i].newMask,0,16).toUpper()));
    }
    layout->addWidget(table);
    dlg.resize(600, 400);
    dlg.exec();
}

void MainWindow::onResetAffinity()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD_PTR fullMask = ((DWORD_PTR)1 << sysInfo.dwNumberOfProcessors) - 1;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(hSnap, &pe)) {
            do {
                HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProc) {
                    SetProcessAffinityMask(hProc, fullMask);
                    CloseHandle(hProc);
                }
            } while (Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    QMessageBox::information(this, "Reset Affinity", "Affinity reset for all processes.");
}

void MainWindow::onVerifyIntegrity()
{
    bool ok = false;
    QString processName = QInputDialog::getText(this, "Verify Integrity", "Enter process name:", QLineEdit::Normal, QString(), &ok);
    if (!ok || processName.isEmpty())
        return;
    ProcessInfo processInfo;
    bool valid = processInfo.VerifyProcessIntegrity(processName.toStdString().c_str());
    if (valid) {
        QMessageBox::information(this, "Verify Integrity", "Process integrity verified successfully.");
    } else {
        QMessageBox::critical(this, "Verify Integrity", "Process integrity check failed.");
    }
}

// Update CPU utilization chart
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
