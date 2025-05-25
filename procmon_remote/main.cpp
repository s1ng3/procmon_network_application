// Author: Tudor-Cristian Sîngerean
// Date: 07.12.2024

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

// Description: This file contains the main function that initializes the Qt application and displays the MainWindow.

// The main function creates a QApplication instance, initializes the MainWindow, and starts the event loop.

