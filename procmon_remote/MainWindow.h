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

private:
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
};

#endif // MAINWINDOW_H

