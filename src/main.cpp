#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("AlgoViz");
    MainWindow w;
    w.show();
    return app.exec();
}
