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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QPushButton *btnProcessDisplay;
    QPushButton *btnProcessLog;
    QPushButton *btnProcessSearch;
    QPushButton *btnKillProcess;
    QPushButton *btnDisplayHardwareInfo;
    QPushButton *btnExit;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        btnProcessDisplay = new QPushButton(centralwidget);
        btnProcessDisplay->setObjectName("btnProcessDisplay");

        verticalLayout->addWidget(btnProcessDisplay);

        btnProcessLog = new QPushButton(centralwidget);
        btnProcessLog->setObjectName("btnProcessLog");

        verticalLayout->addWidget(btnProcessLog);

        btnProcessSearch = new QPushButton(centralwidget);
        btnProcessSearch->setObjectName("btnProcessSearch");

        verticalLayout->addWidget(btnProcessSearch);

        btnKillProcess = new QPushButton(centralwidget);
        btnKillProcess->setObjectName("btnKillProcess");

        verticalLayout->addWidget(btnKillProcess);

        btnDisplayHardwareInfo = new QPushButton(centralwidget);
        btnDisplayHardwareInfo->setObjectName("btnDisplayHardwareInfo");

        verticalLayout->addWidget(btnDisplayHardwareInfo);

        btnExit = new QPushButton(centralwidget);
        btnExit->setObjectName("btnExit");

        verticalLayout->addWidget(btnExit);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Process Monitor", nullptr));
        btnProcessDisplay->setText(QCoreApplication::translate("MainWindow", "Process Display", nullptr));
        btnProcessLog->setText(QCoreApplication::translate("MainWindow", "Process Log", nullptr));
        btnProcessSearch->setText(QCoreApplication::translate("MainWindow", "Process Search", nullptr));
        btnKillProcess->setText(QCoreApplication::translate("MainWindow", "Kill Process", nullptr));
        btnDisplayHardwareInfo->setText(QCoreApplication::translate("MainWindow", "Display Hardware Info", nullptr));
        btnExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
