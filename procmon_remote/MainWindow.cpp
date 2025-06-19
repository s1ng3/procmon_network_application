#include <winsock2.h>
#include <windows.h>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ProcessInfo.hpp"
#include "HandleDLLInspection.hpp"
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QToolButton>
#include <QStyle>
#include <QRandomGenerator>
#include <shellapi.h>
#include <tlhelp32.h>
#include <QStringList>
#include <QDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <unordered_map>
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
#include <QFile>
#include <QCryptographicHash>
#include <QMap>
#include <QByteArray>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QGridLayout>
#include <QGraphicsLinearLayout>

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

    // THEME DROPDOWN SETUP (top-left corner)
    themeSelector = new QComboBox(this);
    themeSelector->setFixedWidth(180);
    themeSelector->setToolTip("Switch color theme");
    // Store default theme style and palette
    defaultPalette = qApp->palette();
    defaultAppStyleSheet = qApp->styleSheet();
    initialMainStyleSheet = this->styleSheet();
    // Setup default black-blue theme colors and apply initially
    defaultThemeColors = QStringList() << "#000000" << "#1565C0" << "#2196F3" << "#0A0A0A" << "#1E1E1E";
    // Add Default Theme option
    themeSelector->addItem("Default Theme");
    // Theme names and palettes
    themes["Dark Purple"]      = {"#4B0082", "#6A0DAD", "#E040FB", "#1A0026", "#F0E6FF"};
    themes["Ocean Blue"]       = {"#006994", "#009DDC", "#FFD166", "#EAF6FF", "#023047"};
    themes["Sunset Orange"]    = {"#FF4500", "#FF8C42", "#FFD700", "#FFF5E6", "#330000"};
    themes["Forest Green"]     = {"#228B22", "#32CD32", "#ADFF2F", "#F0FFF0", "#0B3D0B"};
    themes["Pastel Mint"]      = {"#AAF0D1", "#90EE90", "#FFB6C1", "#FFFFFF", "#064E3B"};
    themes["Cherry Blossom"]   = {"#FFB7C5", "#FF69B4", "#DB7093", "#FFF0F5", "#4B0029"};
    themes["Neon Tech"]        = {"#0FF0FC", "#39FF14", "#FF073A", "#000010", "#E0E0E0"};
    themes["Desert Sunset"]    = {"#D2691E", "#FF8C00", "#FF4500", "#FFF8F0", "#3D1F00"};
    themes["Candy Pink"]       = {"#FF69B4", "#FF1493", "#FFB347", "#FFF5F8", "#550032"};
    themes["Stormy Teal"]      = {"#008080", "#20B2AA", "#FF6347", "#E0F7F7", "#003333"};
    themes["Ultra Violet"]     = {"#5F4B8B", "#7C3F98", "#B57EDC", "#1C0A28", "#EDE3F9"};
    themes["Cyberpunk Neon"]   = {"#FF006E", "#00F0FF", "#FFEA00", "#0D0015", "#EEEEEE"};
    themes["Coral Reef"]       = {"#FF6F61", "#FFA077", "#FFD38C", "#FFF8F2", "#3E1F0D"};
    themes["Lavender Fields"]  = {"#B57EDC", "#D3BFF9", "#F2E7FE", "#FCFBFF", "#4A2A6A"};
    themes["Golden Sunrise"]   = {"#FFB300", "#FFD54F", "#FFE082", "#FFF9E6", "#4A3000"};
    themeSelector->addItems(themes.keys());
    connect(themeSelector, &QComboBox::currentTextChanged, this, &MainWindow::onThemeChanged);
    menuBar()->setCornerWidget(themeSelector, Qt::TopLeftCorner);

    // Increase padding and spacing
    if (auto lay = ui->centralwidget->layout()) {
        lay->setContentsMargins(10, 10, 10, 10);
        lay->setSpacing(10);
        lay->setSizeConstraint(QLayout::SetNoConstraint);
    }

    // auto-stretch columns to fill table width and avoid horizontal scroll
    ui->tableProcesses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableProcesses->setMinimumWidth(600);

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
    if (!connect(ui->btnToggleHardwareInfo, &QPushButton::clicked, this, &MainWindow::onToggleHardwareInfo)) {
        QMessageBox::critical(this, "Error", "Failed to connect toggle hardware info signal.");
    }
    ui->textHardwareInfo->hide(); // hide hardware info initially
    // initialize hardware info timer for real-time updates
    hardwareInfoTimer = new QTimer(this);
    connect(hardwareInfoTimer, &QTimer::timeout, this, &MainWindow::updateHardwareInfo);
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
    // Setup context menu for process table
    ui->tableProcesses->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableProcesses, &QWidget::customContextMenuRequested,
            this, &MainWindow::onProcessContextMenu);

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

    // Core usage bar chart setup
    SYSTEM_INFO sysInfo2; GetSystemInfo(&sysInfo2);
    coreCount = sysInfo2.dwNumberOfProcessors;

    coreBarSet = new QBarSet("Usage");
    for (int i = 0; i < coreCount; ++i) *coreBarSet << 0;
    coreBarSeries = new QBarSeries(this);
    coreBarSeries->append(coreBarSet);
    coreBarSeries->setLabelsVisible(false);  // hide labels on bars
    coreBarChart = new QChart();
    coreBarChart->addSeries(coreBarSeries);
    coreBarChart->legend()->setVisible(true);
    coreBarChart->legend()->setAlignment(Qt::AlignRight);
    coreBarChart->legend()->setLabelBrush(QBrush(QColor("#fff"))); // will be overridden by theme
    coreBarChart->setAnimationOptions(QChart::NoAnimation);
    coreBarChart->setTitle("Per-CPU Core Usage");
    QStringList categories;
    for (int i = 0; i < coreCount; ++i) categories << QString::number(i);
    auto axisX2 = new QBarCategoryAxis(); axisX2->append(categories);
    axisX2->setTitleText("Core #");
    coreBarChart->addAxis(axisX2, Qt::AlignBottom);
    coreBarSeries->attachAxis(axisX2);
    auto axisY2 = new QValueAxis(); axisY2->setRange(0,100);
    axisY2->setTitleText("Usage (%)");
    coreBarChart->addAxis(axisY2, Qt::AlignLeft);
    coreBarSeries->attachAxis(axisY2);
    coreBarChartView = new QChartView(coreBarChart, this);
    coreBarChartView->setMinimumWidth(600); // setting width
    coreBarChartView->setMinimumHeight(300);
    coreBarChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    coreBarChartView->show();

    // PDH initialization for per-core CPU usage
    PdhOpenQuery(nullptr, 0, &coreQuery);
    coreCounters.resize(coreCount);
    for (int i = 0; i < coreCount; ++i) {
        QString path = QString("\\Processor(%1)\\% Processor Time").arg(i);
        PdhAddCounterW(coreQuery, (LPCWSTR)path.utf16(), 0, &coreCounters[i]);
    }
    PdhCollectQueryData(coreQuery);
    coreBarTimer = new QTimer(this);
    connect(coreBarTimer, &QTimer::timeout, this, &MainWindow::updateCoreUsageBars);
    coreBarTimer->start(1000);

    // place per-core chart left of buttons in processes tab
    if (auto mainHL = ui->tabProcesses->findChild<QHBoxLayout*>("mainHorizontalLayout")) {
        mainHL->insertWidget(0, coreBarChartView);
        // Create pie charts wrapper
        QWidget *pieWrapper = new QWidget(this);
        QGridLayout *grid = new QGridLayout(pieWrapper);
        grid->setContentsMargins(4,4,4,4);
        grid->setSpacing(4);
        // Memory Usage Distribution
        pieChartMem = new QChart(); pieChartMem->setTitle("Memory Usage"); pieChartMem->setTheme(QChart::ChartThemeDark);
        QPieSeries *seriesMem = new QPieSeries();
        MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem); GlobalMemoryStatusEx(&mem);
        PERFORMANCE_INFORMATION pi; pi.cb = sizeof(pi); GetPerformanceInfo(&pi, sizeof(pi));
        qreal usedMB = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024.0*1024.0);
        qreal freeMB = mem.ullAvailPhys / (1024.0*1024.0);
        qreal cachedMB = (pi.SystemCache * pi.PageSize) / (1024.0*1024.0);
        qreal pagedMB = mem.ullTotalPageFile / (1024.0*1024.0);
        seriesMem->append("Used", usedMB);
        seriesMem->append("Free", freeMB);
        seriesMem->append("Cached", cachedMB);
        seriesMem->append("Paged", pagedMB);
        seriesMem->setPieSize(0.5);
        pieChartMem->addSeries(seriesMem);
        pieChartMem->legend()->setFont(QFont("", 8));
        pieChartMem->legend()->setVisible(true);
        pieChartMem->legend()->setAlignment(Qt::AlignRight);
        pieChartViewMem = new QChartView(pieChartMem, this);
        pieChartViewMem->setRenderHint(QPainter::Antialiasing);
        pieChartViewMem->setMinimumSize(300, 250); // initial was 300
        grid->addWidget(pieChartViewMem, 0, 0);
        // Process State Distribution
        pieChartProcState = new QChart(); pieChartProcState->setTitle("Process States"); pieChartProcState->setTheme(QChart::ChartThemeDark);
        QPieSeries *seriesState = new QPieSeries();
        seriesState->append("Running", 1);
        seriesState->append("Sleeping", 1);
        seriesState->append("Stopped", 1);
        seriesState->setLabelsVisible(false);
        seriesState->setPieSize(0.5);
        pieChartProcState->addSeries(seriesState);
        pieChartProcState->legend()->setFont(QFont("", 8));
        pieChartProcState->legend()->setVisible(true);
        pieChartProcState->legend()->setAlignment(Qt::AlignRight);
        pieChartViewState = new QChartView(pieChartProcState, this);
        pieChartViewState->setRenderHint(QPainter::Antialiasing);
        pieChartViewState->setMinimumSize(300, 250);
        grid->addWidget(pieChartViewState, 0, 1);
        // Process Priority Classes
        pieChartProcPriority = new QChart(); pieChartProcPriority->setTitle("Process Priorities"); pieChartProcPriority->setTheme(QChart::ChartThemeDark);
        QPieSeries *seriesPrio = new QPieSeries();
        seriesPrio->append("Below-Normal", 1);
        seriesPrio->append("Normal", 1);
        seriesPrio->append("Above-Normal", 1);
        seriesPrio->append("High", 1);
        seriesPrio->append("Real-Time", 1);
        seriesPrio->setLabelsVisible(false);
        seriesPrio->setPieSize(0.5);
        pieChartProcPriority->addSeries(seriesPrio);
        pieChartProcPriority->legend()->setFont(QFont("", 8));
        pieChartProcPriority->legend()->setVisible(true);
        pieChartProcPriority->legend()->setAlignment(Qt::AlignRight);
        pieChartViewPriority = new QChartView(pieChartProcPriority, this);
        pieChartViewPriority->setRenderHint(QPainter::Antialiasing);
        pieChartViewPriority->setMinimumSize(300, 250);
        grid->addWidget(pieChartViewPriority, 1, 0);
        // Disk Usage per Partition
        pieChartDisk = new QChart(); pieChartDisk->setTitle("Disk Usage"); pieChartDisk->setTheme(QChart::ChartThemeDark);
        QPieSeries *seriesDisk = new QPieSeries();
        ULARGE_INTEGER freeBytes, totalBytes, availBytes;
        if (GetDiskFreeSpaceEx(nullptr, &freeBytes, &totalBytes, &availBytes)) {
            qreal usedDisk = (totalBytes.QuadPart - freeBytes.QuadPart) / (1024.0*1024.0*1024.0);
            qreal freeDisk = freeBytes.QuadPart / (1024.0*1024.0*1024.0);
            seriesDisk->append("Used (GB)", usedDisk);
            seriesDisk->append("Free (GB)", freeDisk);
        } else {
            seriesDisk->append("Used", 1);
            seriesDisk->append("Free", 1);
        }
        seriesDisk->setPieSize(0.5);
        pieChartDisk->addSeries(seriesDisk);
        pieChartDisk->legend()->setFont(QFont("", 8));
        pieChartDisk->legend()->setVisible(true);
        pieChartDisk->legend()->setAlignment(Qt::AlignRight);
        pieChartViewDisk = new QChartView(pieChartDisk, this);
        pieChartViewDisk->setRenderHint(QPainter::Antialiasing);
        pieChartViewDisk->setMinimumSize(300, 250);
        grid->addWidget(pieChartViewDisk, 1, 1);
        // Insert pieWrapper into layout between coreBarChartView and buttons
        mainHL->insertWidget(1, pieWrapper);
    }

    // Apply default theme after full UI initialization
    applyTheme(defaultThemeColors);
    // Setup pie chart update timer for dynamic updates
    pieTimer = new QTimer(this);
    connect(pieTimer, &QTimer::timeout, this, &MainWindow::updatePieCharts);
    pieTimer->start(1000);
    updatePieCharts();
    // Stretch process-tab buttons to match stats panel width
    {
        int statsWidth = statsWidget->sizeHint().width();
        QList<QPushButton*> procBtns = { ui->btnProcessDisplay, ui->btnProcessLog, ui->btnProcessSearch,
                                        ui->btnKillProcess, ui->btnOpenProcess, ui->btnGetProcessMemoryUsage,
                                        ui->btnGetProcessPath, ui->btnFastLimitRAM, ui->btnLimitJobObjects,
                                        ui->btnLimitLogicalProcessors, ui->btnOptimizePerformance,
                                        ui->btnResetAffinity, ui->btnVerifyIntegrity, ui->btnExit };
        for (auto b : procBtns) if (b) b->setFixedWidth(statsWidth);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProcessDisplay()
{
    if (!processesVisible) {
        ui->tableProcesses->show();
        coreBarChartView->hide();
        // also hide pie charts when displaying process table
        pieChartViewMem->hide(); pieChartViewState->hide(); pieChartViewPriority->hide(); pieChartViewDisk->hide();
        processesVisible = true;
        ui->btnProcessDisplay->setText("Hide Processes");
        refreshProcesses();
    } else {
        ui->tableProcesses->hide();
        coreBarChartView->show();
        // restore pie charts visibility
        pieChartViewMem->show(); pieChartViewState->show(); pieChartViewPriority->show(); pieChartViewDisk->show();
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

void MainWindow::updateHardwareInfo()
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    DWORD len = 0;
    GetLogicalProcessorInformation(nullptr, &len);
    auto procInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(len);
    GetLogicalProcessorInformation(procInfo, &len);
    int physicalCores = 0, l1=0, l2=0, l3=0;
    DWORD count = len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (DWORD i = 0; i < count; ++i) {
        if (procInfo[i].Relationship == RelationProcessorCore)
            ++physicalCores;
        if (procInfo[i].Relationship == RelationCache) {
            auto &c = procInfo[i].Cache;
            switch (c.Level) {
                case 1: l1 = c.Size/1024; break;
                case 2: l2 = c.Size/1024; break;
                case 3: l3 = c.Size/1024; break;
            }
        }
    }
    free(procInfo);

    MEMORYSTATUSEX mem{sizeof(mem)};
    GlobalMemoryStatusEx(&mem);

    // CPU name via CPUID
    char cpuName[0x40] = {0};
    int cpuInfo[4] = {-1};
    __cpuid(cpuInfo, 0x80000000);
    if (cpuInfo[0] >= 0x80000004) {
        __cpuid(cpuInfo, 0x80000002);
        memcpy(cpuName, cpuInfo, 16);
        __cpuid(cpuInfo, 0x80000003);
        memcpy(cpuName+16, cpuInfo, 16);
        __cpuid(cpuInfo, 0x80000004);
        memcpy(cpuName+32, cpuInfo, 16);
    }

    PERFORMANCE_INFORMATION pi{sizeof(pi)};
    GetPerformanceInfo(&pi, sizeof(pi));

    QString info;
    info += QString("CPU: %1\n").arg(QString::fromLocal8Bit(cpuName).trimmed());
    info += QString("Logical processors: %1\n").arg(sysInfo.dwNumberOfProcessors);
    info += QString("Physical cores: %1\n").arg(physicalCores);
    info += QString("L1 Cache: %1 KB\n").arg(l1);
    info += QString("L2 Cache: %1 KB\n").arg(l2);
    info += QString("L3 Cache: %1 KB\n").arg(l3);
    info += QString("Total Memory: %1 MB\n").arg(mem.ullTotalPhys/(1024*1024));
    info += QString("Available Memory: %1 MB\n").arg(mem.ullAvailPhys/(1024*1024));
    info += QString("Memory Load: %1%\n").arg(mem.dwMemoryLoad);
    info += QString("Page file total: %1 MB\n").arg(pi.CommitLimit*pi.PageSize/(1024*1024));
    info += QString("Page file used: %1 MB\n").arg(pi.CommitTotal*pi.PageSize/(1024*1024));
    info += QString("Handles: %1\n").arg(pi.HandleCount);
    info += QString("Processes: %1\n").arg(pi.ProcessCount);
    info += QString("Threads: %1\n").arg(pi.ThreadCount);
    info += QString("System Uptime: %1 sec\n").arg(GetTickCount64()/1000);

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
    // Prompt user for process name to limit
    QString processName = QInputDialog::getText(this, "Limit Logical Processors", "Enter process name:");
    // If no process name provided, abort operation
    if (processName.isEmpty()) return;
    // Prompt user for core mask in hexadecimal format (e.g., 0xA9)
    QString maskStr = QInputDialog::getText(this, "Limit Logical Processors", "Enter core mask (hex, e.g., 0xA9):");
    // If user did not provide a mask, abort operation
    if (maskStr.isEmpty()) return;
    // Convert mask string to unsigned long value, base auto-detected
    bool ok = false;
    unsigned long mask = maskStr.toULong(&ok, 0);
    // If conversion failed, abort operation
    if (!ok) return;
    // Take a snapshot of all running processes
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    // Prepare process entry structure with its size
    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
    // Track whether setting affinity succeeded
    bool success = false;
    // Iterate through the snapshot to find the matching process
    if (Process32First(hSnap, &pe)) {
        do {
            // Compare each process name case-insensitively against user input
            if (QString::fromLocal8Bit(pe.szExeFile).compare(processName, Qt::CaseInsensitive) == 0) {
                // Open the process with permission to change its affinity
                HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pe.th32ProcessID);
                if (hProc) {
                    // Attempt to set the specified affinity mask
                    if (SetProcessAffinityMask(hProc, mask)) success = true;
                    // Close the handle when done
                    CloseHandle(hProc);
                }
                // Stop searching after first match
                break;
            }
        } while (Process32Next(hSnap, &pe));
    }
    // Release the snapshot handle
    CloseHandle(hSnap);
    // Notify the user of the outcome based on the success flag
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

// Slot to update per-core CPU usage bars
void MainWindow::updateCoreUsageBars()
{
    PdhCollectQueryData(coreQuery);
    for (int i = 0; i < coreCount; ++i) {
        PDH_FMT_COUNTERVALUE val;
        if (PdhGetFormattedCounterValue(coreCounters[i], PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS) {
            coreBarSet->replace(i, val.doubleValue);
        }
    }
}

// Context menu for process table
void MainWindow::onProcessContextMenu(const QPoint &pos) {
    QModelIndex idx = ui->tableProcesses->indexAt(pos);
    if (!idx.isValid()) return;
    int row = idx.row();
    QString pidStr = ui->tableProcesses->item(row, 0)->text();
    DWORD pid = pidStr.toUInt();
    QMenu menu(this);
    QAction *actHandles = menu.addAction("Show Handles");
    QAction *actModules = menu.addAction("Show Modules");
    QAction *act = menu.exec(ui->tableProcesses->viewport()->mapToGlobal(pos));
    if (act == actHandles) showHandlesDialog(pid);
    else if (act == actModules) showModulesDialog(pid);
}

// Display handles for a given PID
void MainWindow::showHandlesDialog(DWORD pid) {
    std::vector<HandleInfo> handles;
    if (!HandleDLLInspection::getHandles(pid, handles)) {
        QMessageBox::warning(this, "Handles", "Failed to retrieve handles.");
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Handles for PID %1").arg(pid));
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QTableWidget *table = new QTableWidget(&dlg);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels(QStringList{"Handle", "Type", "Name"});
    table->setRowCount(static_cast<int>(handles.size()));
    for (int i = 0; i < static_cast<int>(handles.size()); ++i) {
        const auto &hi = handles[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::number(hi.handle)));
        table->setItem(i, 1, new QTableWidgetItem(hi.type));
        table->setItem(i, 2, new QTableWidgetItem(hi.name));
    }
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(table);
    dlg.resize(600, 400);
    dlg.exec();
}

// Display loaded modules for a given PID
void MainWindow::showModulesDialog(DWORD pid) {
    std::vector<ModuleInfo> modules;
    if (!HandleDLLInspection::getModules(pid, modules)) {
        QMessageBox::warning(this, "Modules", "Failed to retrieve modules.");
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Modules for PID %1").arg(pid));
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QTableWidget *table = new QTableWidget(&dlg);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList{"Module Name", "Path"});
    table->setRowCount(static_cast<int>(modules.size()));
    for (int i = 0; i < static_cast<int>(modules.size()); ++i) {
        const auto &mi = modules[i];
        table->setItem(i, 0, new QTableWidgetItem(mi.moduleName));
        table->setItem(i, 1, new QTableWidgetItem(mi.modulePath));
    }
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(table);
    dlg.resize(800, 500);
    dlg.exec();
}

// Implement onShowHandles and onShowModules
void MainWindow::onShowHandles() {
    int row = ui->tableProcesses->currentRow();
    if (row < 0) return;
    QString pidStr = ui->tableProcesses->item(row, 0)->text();
    if (pidStr.isEmpty()) return;
    DWORD pid = pidStr.toUInt();
    showHandlesDialog(pid);
}

void MainWindow::onShowModules() {
    int row = ui->tableProcesses->currentRow();
    if (row < 0) return;
    QString pidStr = ui->tableProcesses->item(row, 0)->text();
    if (pidStr.isEmpty()) return;
    DWORD pid = pidStr.toUInt();
    showModulesDialog(pid);
}

// Apply theme colors to the UI using palette and stylesheet
void MainWindow::applyTheme(const QStringList &colors) {
    // colors: [primary, secondary, accent, background, surface]
    QString primary   = colors.value(0, "#222");
    QString secondary = colors.value(1, "#444");
    QString accent    = colors.value(2, "#888");
    QString bg        = colors.value(3, "#fff");
    QString surface   = colors.value(4, "#eee");

    // Determine if background is dark or light for contrast
    auto luminance = [](const QString &hex) {
        QColor c(hex); double r=c.redF(),g=c.greenF(),b=c.blueF();
        return 0.2126*r + 0.7152*g + 0.0722*b;
    };
    bool darkBg = luminance(bg) < 0.5;
    QString textColor = darkBg ? "#fff" : "#111";
    QString iconColor = textColor;

    // Accent color luminance for buttons/tables
    bool darkAccent = luminance(accent) < 0.5;
    QString accentTextColor = darkAccent ? "#fff" : "#111";

    // Use the stats panel color for buttons and table
    QString statsBg = accent;
    QString statsText = accentTextColor;

    // Clear the main window's own stylesheet to prevent conflicts
    // with the application-wide stylesheet.
    this->setStyleSheet("");

    // Set palette for all widgets
    QPalette pal;
    pal.setColor(QPalette::Window, QColor(bg));
    pal.setColor(QPalette::WindowText, QColor(textColor));
    pal.setColor(QPalette::Base, QColor(surface));
    pal.setColor(QPalette::AlternateBase, QColor(bg));
    pal.setColor(QPalette::ToolTipBase, QColor(surface));
    pal.setColor(QPalette::ToolTipText, QColor(textColor));
    pal.setColor(QPalette::Text, QColor(textColor));
    pal.setColor(QPalette::Button, QColor(statsBg)); // stats panel color for buttons
    pal.setColor(QPalette::ButtonText, QColor(statsText));
    pal.setColor(QPalette::BrightText, QColor(darkBg ? "#fff" : "#000"));
    pal.setColor(QPalette::Highlight, QColor(accent));
    pal.setColor(QPalette::HighlightedText, QColor(darkAccent ? "#fff" : "#000"));
    qApp->setPalette(pal);

    // Set stylesheet for fine-grained control
    QString style = QString(R"(
        QMainWindow { background-color: %1; color: %2; }
        QWidget { background-color: %4; color: %2; }
        QPushButton, QToolButton {
            background-color: %6; color: %7; border-radius: 4px; border: none; padding: 6px 12px;
        }
        QPushButton:hover, QToolButton:hover { background-color: %5; color: %7; }
        QPushButton:pressed, QToolButton:pressed { background-color: %1; color: %2; }
        QTableWidget {
            background-color: %6; color: %7; selection-background-color: %5; selection-color: %7; gridline-color: %1;
        }
        QHeaderView::section { background-color: %6; color: %7; }
        QComboBox, QComboBox QAbstractItemView { background-color: %4; color: %2; selection-background-color: %5; }
        QToolTip { background-color: %5; color: %2; border: 1px solid %3; }
        QLabel { color: %2; background-color: transparent; } /* Ensure QLabel background is transparent */
    )")
        .arg(primary, textColor, accent, bg, secondary, statsBg, statsText);
    qApp->setStyleSheet(style);

    // CPU chart theme
    if (cpuChart) {
        cpuChart->setBackgroundBrush(QBrush(QColor(bg)));
        cpuChart->setTitleBrush(QBrush(QColor(textColor)));
        for (auto axis : cpuChart->axes()) {
            axis->setLabelsBrush(QBrush(QColor(textColor)));
            axis->setLinePen(QPen(QColor(accent)));
            axis->setGridLinePen(QPen(QColor(surface)));
            axis->setTitleBrush(QBrush(QColor(textColor))); // ensure axis title ("CPU %" and "Time") is visible
        }
        // style legend marker label color (if legend is used)
        for (auto marker : cpuChart->legend()->markers()) {
            marker->setLabelBrush(QBrush(QColor(textColor)));
        }
        // Fix: set color for QLineSeries only
        for (auto s : cpuChart->series()) {
            if (auto line = qobject_cast<QLineSeries*>(s)) {
                line->setColor(QColor(accent));
            }
        }
    }
    // Theme core bar chart as well
    if (coreBarChart) {
        coreBarChart->setBackgroundBrush(QBrush(QColor(bg)));
        coreBarChart->setTitleBrush(QBrush(QColor(textColor)));
        for (auto axis : coreBarChart->axes()) {
            axis->setLabelsBrush(QBrush(QColor(textColor)));
            axis->setLinePen(QPen(QColor(accent)));
            axis->setGridLinePen(QPen(QColor(surface)));
            axis->setTitleBrush(QBrush(QColor(textColor)));  // ensure axis title uses correct color
        }
        // style legend marker label color
        for (auto marker : coreBarChart->legend()->markers()) {
            marker->setLabelBrush(QBrush(QColor(textColor)));
        }
        // Set bar colors
        for (auto barSet : coreBarSeries->barSets()) {
            barSet->setBrush(QBrush(QColor(accent)));
            barSet->setLabelColor(QColor(textColor));
        }
    }
    // Stats panel
    if (statsWidget) {
        // Use accent as background, and accentTextColor for text
        statsWidget->setStyleSheet(QString("background-color:%1;color:%2;border-radius:4px;padding:4px;")
                                  .arg(statsBg, statsText));
    }

    // Theme pie charts: generate distinguishable slice colors by varying hue of accent color
    QList<QColor> sliceColors = { QColor(primary), QColor(secondary), QColor(accent), QColor(textColor), QColor(surface) };

    auto stylePie = [&](QChart *chart) {
        if (!chart) return;
        chart->setBackgroundBrush(QBrush(QColor(bg)));
        chart->setTitleBrush(QBrush(QColor(textColor)));
        // legend labels
        for (auto marker : chart->legend()->markers()) {
            marker->setLabelBrush(QBrush(QColor(textColor)));
        }
        // style pie slices
        for (auto s : chart->series()) {
            if (auto pie = qobject_cast<QPieSeries*>(s)) {
                int idx = 0;
                for (auto slice : pie->slices()) {
                    slice->setLabelBrush(QBrush(QColor(textColor)));
                    slice->setPen(QPen(QColor(surface)));
                    slice->setBrush(QBrush(sliceColors[idx++ % sliceColors.size()]));
                    if (chart == pieChartProcState || chart == pieChartProcPriority) {
                        slice->setLabelVisible(false);
                    } else {
                        slice->setLabelPosition(QPieSlice::LabelOutside);
                        slice->setLabelColor(QColor(textColor));
                        slice->setLabelArmLengthFactor(0.2);
                        slice->setLabelVisible(true);
                    }
                }
            }
        }
        // ensure legend shows full labels
        auto legend = chart->legend();
        if (auto ll = dynamic_cast<QGraphicsLinearLayout*>(legend->layout())) {
            ll->setOrientation(Qt::Vertical);
        }
        QFontMetrics fm(legend->font());
        int maxWidth = 0;
        for (auto marker : legend->markers()) {
            maxWidth = qMax(maxWidth, fm.horizontalAdvance(marker->label()));
        }
        legend->setMinimumSize(maxWidth + 20, 0);
    };
    stylePie(pieChartMem);
    stylePie(pieChartProcState);
    stylePie(pieChartProcPriority);
    stylePie(pieChartDisk);
}

void MainWindow::onThemeChanged(const QString &themeName) {
    if (themeName == "Default Theme") {
        applyTheme(defaultThemeColors);
    } else if (themes.contains(themeName)) {
        applyTheme(themes[themeName]);
    }
}

// Slot to update pie chart data dynamically
void MainWindow::updatePieCharts()
{
    // Slice label names
    const QString memNames[4] = {"Used", "Free", "Cached", "Paged"};
    const QString diskNames[2] = {"Used (GB)", "Free (GB)"};

    // Update Memory Usage pie
    MEMORYSTATUSEX mem{sizeof(mem)};
    GlobalMemoryStatusEx(&mem);
    PERFORMANCE_INFORMATION pi{sizeof(pi)};
    GetPerformanceInfo(&pi, sizeof(pi));
    double usedMB = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024.0*1024.0);
    double freeMB = mem.ullAvailPhys / (1024.0*1024.0);
    double cachedMB = (pi.SystemCache * pi.PageSize) / (1024.0*1024.0);
    double pagedMB = mem.ullTotalPageFile / (1024.0*1024.0);
    double totalMB = usedMB + freeMB + cachedMB + pagedMB;
    if (pieChartMem) {
        auto series = qobject_cast<QPieSeries*>(pieChartMem->series().first());
        if (series) {
            auto slices = series->slices();
            auto markers = pieChartMem->legend()->markers();
            double vals[4] = {usedMB, freeMB, cachedMB, pagedMB};
            for (int i = 0; i < sizeof(memNames)/sizeof(memNames[0]) && i < slices.size() && i < markers.size(); ++i) {
                slices.at(i)->setValue(vals[i]);
                double pct = totalMB > 0 ? (vals[i] / totalMB) * 100.0 : 0.0;
                slices.at(i)->setLabel(QString("%1%").arg(pct, 0, 'f', 1)); // Percentage on slice
                slices.at(i)->setLabelVisible(true);
                markers.at(i)->setLabel(memNames[i]); // Name in legend
            }
        }
    }
    // Update Disk Usage pie
    ULARGE_INTEGER freeBytes, totalBytes, availBytes;
    if (GetDiskFreeSpaceEx(nullptr, &freeBytes, &totalBytes, &availBytes)) {
        double usedGB = (totalBytes.QuadPart - freeBytes.QuadPart) / (1024.0*1024.0*1024.0);
        double freeGB = freeBytes.QuadPart / (1024.0*1024.0*1024.0);
        double totalGB = usedGB + freeGB;
        if (pieChartDisk) {
            auto series = qobject_cast<QPieSeries*>(pieChartDisk->series().first());
            if (series) {
                auto slices = series->slices();
                auto markers = pieChartDisk->legend()->markers();
                double valsDisk[2] = {usedGB, freeGB};
                for (int i = 0; i < sizeof(diskNames)/sizeof(diskNames[0]) && i < slices.size() && i < markers.size(); ++i) {
                    slices.at(i)->setValue(valsDisk[i]);
                    double pct = totalGB > 0 ? (valsDisk[i] / totalGB) * 100.0 : 0.0;
                    slices.at(i)->setLabel(QString("%1%").arg(pct, 0, 'f', 1)); // Percentage on slice
                    slices.at(i)->setLabelVisible(true);
                    markers.at(i)->setLabel(diskNames[i]); // Name in legend
                }
            }
        }
    }
    //TODO IT STARTS
    // Update Process State pie
    if (pieChartProcState) {
        auto series = qobject_cast<QPieSeries*>(pieChartProcState->series().first());
        if (series) {
            int running=0, sleeping=0, stopped=0;
            HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnap != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
                if (Process32First(hSnap, &pe)) {
                    do {
                        HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
                        if (hProc) {
                            DWORD exitCode=0;
                            if (GetExitCodeProcess(hProc, &exitCode)) {
                                if (exitCode == STILL_ACTIVE) running++;
                                else stopped++;
                            }
                            CloseHandle(hProc);
                        } else {
                            sleeping++; // treat inaccessible as sleeping
                        }
                    } while (Process32Next(hSnap, &pe));
                }
                CloseHandle(hSnap);
            }
            QList<QPieSlice*> slices = series->slices();
            int vals[3] = {running, sleeping, stopped};
            auto markers = pieChartProcState->legend()->markers();
            QStringList labels = {"Running", "Sleeping", "Stopped"};
            for (int i = 0; i < 3 && i < slices.size(); ++i) {
                slices[i]->setValue(vals[i]);
                slices[i]->setLabel(QString::number(vals[i]));
                slices[i]->setLabelVisible(true);
                if (i < markers.size()) markers[i]->setLabel(labels[i]);
            }
        }
    }
    // Update Process Priorities pie
    if (pieChartProcPriority) {
        auto series = qobject_cast<QPieSeries*>(pieChartProcPriority->series().first());
        if (series) {
            QMap<DWORD,int> prioCount;
            HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnap != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
                if (Process32First(hSnap, &pe)) {
                    do {
                        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
                        if (hProc) {
                            DWORD pc = GetPriorityClass(hProc);
                            prioCount[pc]++;
                            CloseHandle(hProc);
                        }
                    } while (Process32Next(hSnap, &pe));
                }
                CloseHandle(hSnap);
            }
            QList<QPieSlice*> slices = series->slices();
            int valsP[5] = {
                prioCount.value(BELOW_NORMAL_PRIORITY_CLASS),
                prioCount.value(NORMAL_PRIORITY_CLASS),
                prioCount.value(ABOVE_NORMAL_PRIORITY_CLASS),
                prioCount.value(HIGH_PRIORITY_CLASS),
                prioCount.value(REALTIME_PRIORITY_CLASS)
            };
            auto markersP = pieChartProcPriority->legend()->markers();
            QStringList prioLabels = {"Below-Normal", "Normal", "Above-Normal", "High", "Real-Time"};
            for (int i = 0; i < 5 && i < slices.size(); ++i) {
                slices[i]->setValue(valsP[i]);
                slices[i]->setLabel(QString::number(valsP[i]));
                slices[i]->setLabelVisible(true);
                if (i < markersP.size()) markersP[i]->setLabel(prioLabels[i]);
            }
        }
    }
    //TODO IT ENDS
}

// Implement the missing onToggleHardwareInfo slot and hide the hardware info text widget by default
void MainWindow::onToggleHardwareInfo()
{
    if (hardwareInfoTimer->isActive()) {
        hardwareInfoTimer->stop();
        ui->textHardwareInfo->hide();
        ui->btnToggleHardwareInfo->setText("Display Hardware Info");
    } else {
        ui->textHardwareInfo->show();
        updateHardwareInfo();
        hardwareInfoTimer->start(1000);
        ui->btnToggleHardwareInfo->setText("Hide Hardware Info");
    }
}

