#include <QApplication>
#include <iostream>
#include <string>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Must provide path to settings file\n";
        return 1;
    }

    QApplication app(argc, argv);

    int seconds_between_games = 10;
    int milliseconds_between_moves = 1000;

    if(argc >= 3)
    {
        seconds_between_games = std::stoll(argv[2]);
    }


    if(argc >= 4)
    {
        milliseconds_between_moves = std::stoll(argv[3]);
    }

    MainWindow window(argv[1], seconds_between_games, milliseconds_between_moves);
    window.setWindowTitle("AtaxxGUI");

    window.show();
    return app.exec();
}