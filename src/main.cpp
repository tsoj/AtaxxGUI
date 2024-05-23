#include <QApplication>
#include <iostream>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Must provide path to settings file\n";
        return 1;
    }

    QApplication app(argc, argv);

    MainWindow window(argv[1]);
    window.setWindowTitle("AtaxxGUI");

    window.show();
    return app.exec();
}