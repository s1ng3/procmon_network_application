#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QStyle>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QTimer>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QMap>

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
    void onProcessDisplay();
    void refreshProcesses();
    void onProcessLog();
    void onProcessSearch();
    void onKillProcess();
    void onOpenProcess();
    void onDisplayHardwareInfo();
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
    double lastCpuPercent;
    int baseSpeedMHz;
    int l1Size;
    int l2Size;
    int l3Size;
    int physicalCores;
    int socketCount;
    bool virtualizationEnabled;
    QTimer *hardwareInfoTimer; // timer to refresh hardware information
};

#endif // MAINWINDOW_H

