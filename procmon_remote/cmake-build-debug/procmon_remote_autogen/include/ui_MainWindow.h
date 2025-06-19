/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QTabWidget *tabWidget;
    QWidget *tabProcesses;
    QVBoxLayout *processLayout;
    QHBoxLayout *mainHorizontalLayout;
    QTableWidget *tableProcesses;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *buttonLayout;
    QPushButton *btnProcessDisplay;
    QPushButton *btnProcessLog;
    QPushButton *btnProcessSearch;
    QPushButton *btnKillProcess;
    QPushButton *btnOpenProcess;
    QPushButton *btnGetProcessMemoryUsage;
    QPushButton *btnGetProcessPath;
    QPushButton *btnFastLimitRAM;
    QPushButton *btnLimitJobObjects;
    QPushButton *btnLimitLogicalProcessors;
    QPushButton *btnOptimizePerformance;
    QPushButton *btnResetAffinity;
    QPushButton *btnVerifyIntegrity;
    QPushButton *btnExit;
    QWidget *tabHardware;
    QVBoxLayout *hardwareLayout;
    QTextEdit *textHardwareInfo;
    QSpacerItem *verticalSpacerHardware;
    QPushButton *btnToggleHardwareInfo;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1000, 450);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setObjectName("mainLayout");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setTabPosition(QTabWidget::TabPosition::North);
        tabProcesses = new QWidget();
        tabProcesses->setObjectName("tabProcesses");
        processLayout = new QVBoxLayout(tabProcesses);
        processLayout->setObjectName("processLayout");
        mainHorizontalLayout = new QHBoxLayout();
        mainHorizontalLayout->setObjectName("mainHorizontalLayout");
        tableProcesses = new QTableWidget(tabProcesses);
        if (tableProcesses->columnCount() < 7)
            tableProcesses->setColumnCount(7);
        tableProcesses->setObjectName("tableProcesses");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(tableProcesses->sizePolicy().hasHeightForWidth());
        tableProcesses->setSizePolicy(sizePolicy);
        tableProcesses->setMinimumSize(QSize(600, 0));
        tableProcesses->setSortingEnabled(true);
        tableProcesses->setColumnCount(7);

        mainHorizontalLayout->addWidget(tableProcesses);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        mainHorizontalLayout->addItem(horizontalSpacer);

        buttonLayout = new QVBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        btnProcessDisplay = new QPushButton(tabProcesses);
        btnProcessDisplay->setObjectName("btnProcessDisplay");

        buttonLayout->addWidget(btnProcessDisplay);

        btnProcessLog = new QPushButton(tabProcesses);
        btnProcessLog->setObjectName("btnProcessLog");

        buttonLayout->addWidget(btnProcessLog);

        btnProcessSearch = new QPushButton(tabProcesses);
        btnProcessSearch->setObjectName("btnProcessSearch");

        buttonLayout->addWidget(btnProcessSearch);

        btnKillProcess = new QPushButton(tabProcesses);
        btnKillProcess->setObjectName("btnKillProcess");

        buttonLayout->addWidget(btnKillProcess);

        btnOpenProcess = new QPushButton(tabProcesses);
        btnOpenProcess->setObjectName("btnOpenProcess");

        buttonLayout->addWidget(btnOpenProcess);

        btnGetProcessMemoryUsage = new QPushButton(tabProcesses);
        btnGetProcessMemoryUsage->setObjectName("btnGetProcessMemoryUsage");

        buttonLayout->addWidget(btnGetProcessMemoryUsage);

        btnGetProcessPath = new QPushButton(tabProcesses);
        btnGetProcessPath->setObjectName("btnGetProcessPath");

        buttonLayout->addWidget(btnGetProcessPath);

        btnFastLimitRAM = new QPushButton(tabProcesses);
        btnFastLimitRAM->setObjectName("btnFastLimitRAM");

        buttonLayout->addWidget(btnFastLimitRAM);

        btnLimitJobObjects = new QPushButton(tabProcesses);
        btnLimitJobObjects->setObjectName("btnLimitJobObjects");

        buttonLayout->addWidget(btnLimitJobObjects);

        btnLimitLogicalProcessors = new QPushButton(tabProcesses);
        btnLimitLogicalProcessors->setObjectName("btnLimitLogicalProcessors");

        buttonLayout->addWidget(btnLimitLogicalProcessors);

        btnOptimizePerformance = new QPushButton(tabProcesses);
        btnOptimizePerformance->setObjectName("btnOptimizePerformance");

        buttonLayout->addWidget(btnOptimizePerformance);

        btnResetAffinity = new QPushButton(tabProcesses);
        btnResetAffinity->setObjectName("btnResetAffinity");

        buttonLayout->addWidget(btnResetAffinity);

        btnVerifyIntegrity = new QPushButton(tabProcesses);
        btnVerifyIntegrity->setObjectName("btnVerifyIntegrity");

        buttonLayout->addWidget(btnVerifyIntegrity);

        btnExit = new QPushButton(tabProcesses);
        btnExit->setObjectName("btnExit");

        buttonLayout->addWidget(btnExit);


        mainHorizontalLayout->addLayout(buttonLayout);


        processLayout->addLayout(mainHorizontalLayout);

        tabWidget->addTab(tabProcesses, QString());
        tabHardware = new QWidget();
        tabHardware->setObjectName("tabHardware");
        hardwareLayout = new QVBoxLayout(tabHardware);
        hardwareLayout->setObjectName("hardwareLayout");
        textHardwareInfo = new QTextEdit(tabHardware);
        textHardwareInfo->setObjectName("textHardwareInfo");
        textHardwareInfo->setReadOnly(true);

        hardwareLayout->addWidget(textHardwareInfo);

        verticalSpacerHardware = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        hardwareLayout->addItem(verticalSpacerHardware);

        btnToggleHardwareInfo = new QPushButton(tabHardware);
        btnToggleHardwareInfo->setObjectName("btnToggleHardwareInfo");

        hardwareLayout->addWidget(btnToggleHardwareInfo);

        tabWidget->addTab(tabHardware, QString());

        mainLayout->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 810, 33));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Process Monitor", nullptr));
        tableProcesses->setProperty("horizontalHeaderLabels", QVariant(QStringList{
            QCoreApplication::translate("MainWindow", "PID", nullptr),
            QCoreApplication::translate("MainWindow", "Name", nullptr),
            QCoreApplication::translate("MainWindow", "Memory", nullptr),
            QCoreApplication::translate("MainWindow", "Threads", nullptr),
            QCoreApplication::translate("MainWindow", "Status", nullptr),
            QCoreApplication::translate("MainWindow", "User", nullptr),
            QCoreApplication::translate("MainWindow", "Priority", nullptr)}));
        btnProcessDisplay->setText(QCoreApplication::translate("MainWindow", "Display Processes", nullptr));
        btnProcessLog->setText(QCoreApplication::translate("MainWindow", "Log Process", nullptr));
        btnProcessSearch->setText(QCoreApplication::translate("MainWindow", "Search Process", nullptr));
        btnKillProcess->setText(QCoreApplication::translate("MainWindow", "Kill Process", nullptr));
        btnOpenProcess->setText(QCoreApplication::translate("MainWindow", "Open Process", nullptr));
        btnGetProcessMemoryUsage->setText(QCoreApplication::translate("MainWindow", "Get Process Memory Usage", nullptr));
        btnGetProcessPath->setText(QCoreApplication::translate("MainWindow", "Get Process Path", nullptr));
        btnFastLimitRAM->setText(QCoreApplication::translate("MainWindow", "Fast Limit RAM", nullptr));
        btnLimitJobObjects->setText(QCoreApplication::translate("MainWindow", "Limit With Job Objects", nullptr));
        btnLimitLogicalProcessors->setText(QCoreApplication::translate("MainWindow", "Limit Logical Processor", nullptr));
        btnOptimizePerformance->setText(QCoreApplication::translate("MainWindow", "Optimize Performance", nullptr));
        btnResetAffinity->setText(QCoreApplication::translate("MainWindow", "Reset Affinity", nullptr));
        btnVerifyIntegrity->setText(QCoreApplication::translate("MainWindow", "Verify Integrity", nullptr));
        btnExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabProcesses), QCoreApplication::translate("MainWindow", "Processes", nullptr));
        btnToggleHardwareInfo->setText(QCoreApplication::translate("MainWindow", "Display Hardware Info", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabHardware), QCoreApplication::translate("MainWindow", "Hardware Info", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
