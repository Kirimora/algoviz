#include "MainWindow.h"
#include "SortView.h"
#include "PathView.h"
#include "HashView.h"
#include "DPView.h"
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("AlgoViz — Algorithm Visualizer");
    resize(1040, 680);

    auto *tabs = new QTabWidget;
    tabs->addTab(new SortView, "Sorting");
    tabs->addTab(new PathView, "Pathfinding");
    tabs->addTab(new HashView, "Hash Table");
    tabs->addTab(new DPView,   "Dynamic Programming");
    setCentralWidget(tabs);
}
