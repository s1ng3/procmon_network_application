#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QStyle>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QTimer>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QMap>
#include <QPalette>
#include <QString>
#include <QVector>  // Added for per-core usage
#include <pdh.h>    // PDH for per-core CPU counters
#include <QInputDialog>
#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onShowHandles();
    void onShowModules();
    void onProcessDisplay();
    void refreshProcesses();
    void onProcessLog();
    void onProcessSearch();
    void onKillProcess();
    void onOpenProcess();
    void onToggleHardwareInfo();
    void onGetProcessMemoryUsage();
    void onGetProcessPath();
    void onFastLimitRAM();
    void onLimitJobObjects();
    void onLimitLogicalProcessors();
    void onOptimizePerformance();
    void onResetAffinity();
    void onVerifyIntegrity();
    void onProcessSelected(QTableWidgetItem* item);
    void onHeaderClicked(int column);
    void updateCpuUsage();
    void updateStats();
    void onThemeChanged(const QString &themeName);
    void updateHardwareInfo(); // slot for real-time hardware info updates
    void updateCoreUsageBars(); // slot to update per-core CPU usage bars
    void updatePieCharts(); // slot to update pie chart data
    void onProcessContextMenu(const QPoint &pos);
    void onFindHandlesDLLs(); // slot for Find button

private:
    void applyTheme(const QStringList &colors);
    QComboBox *themeSelector;
    QMap<QString, QStringList> themes;
    QToolButton *btnRefresh;
    bool processesVisible{false};
    Ui::MainWindow *ui;
    // CPU utilization chart members
    QChartView *cpuChartView;
    QChart *cpuChart;
    QLineSeries *cpuSeries;
    QTimer *cpuTimer;
    quint64 lastIdleTime;
    quint64 lastKernelTime;
    quint64 lastUserTime;
    int cpuPointIndex;

    // Core usage bar chart members
    QChart *coreBarChart;
    QChartView *coreBarChartView;
    QBarSeries *coreBarSeries;
    QBarSet *coreBarSet;
    QTimer *coreBarTimer; // timer for per-core CPU usage updates
    PDH_HQUERY coreQuery;                // PDH query handle
    QVector<PDH_HCOUNTER> coreCounters;  // PDH counters per core
    int coreCount;

    // Stats panel
    QWidget *statsWidget;
    QLabel *lblUtilization;
    QLabel *lblProcesses;
    QLabel *lblUptime;
    QLabel *lblBaseSpeed;
    QLabel *lblSockets;
    QLabel *lblCores;
    QLabel *lblLogical;
    QLabel *lblVirtualization;
    QLabel *lblThreads;
    QLabel *lblHandles;
    QLabel *lblL1Cache;
    QLabel *lblL2Cache;
    QLabel *lblL3Cache;

    // Additional members for stats and CPU info
    double lastCpuPercent{0.0};
    int baseSpeedMHz;
    int l1Size;
    int l2Size;
    int l3Size;
    int physicalCores;
    int socketCount;
    bool virtualizationEnabled;
    QTimer *hardwareInfoTimer; // timer to refresh hardware information
    QTimer *pieTimer; // timer for pie chart updates

    // Default theme members
    QPalette defaultPalette; // store default application palette
    QString defaultAppStyleSheet; // store default application stylesheet
    QString initialMainStyleSheet; // store initial MainWindow stylesheet
    QStringList defaultThemeColors; // colors for custom default theme

    // Pie charts
    QChart *pieChartMem, *pieChartProcState, *pieChartProcPriority, *pieChartDisk;
    QChartView *pieChartViewMem, *pieChartViewState, *pieChartViewPriority, *pieChartViewDisk;

    // Handles/DLLs search UI
    QWidget *handlesWidget; // container for handles search and table
    QLineEdit *searchLineEdit;
    QPushButton *btnFindHandles;
    QTableWidget *tableHandles;

    void showHandlesDialog(DWORD pid);
    void showModulesDialog(DWORD pid);
};

#endif // MAINWINDOW_H

