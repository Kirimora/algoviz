#pragma once
#include <QWidget>
#include <vector>
#include <utility>

class QComboBox;
class QPushButton;
class QCheckBox;
class QLabel;
class PlaybackBar;

enum class Cell { Empty, Wall, Start, Goal, Visited, Path, Frontier };

class GridCanvas : public QWidget {
    Q_OBJECT
public:
    explicit GridCanvas(QWidget *parent = nullptr);
    int cols() const { return cols_; }
    int rows() const { return rows_; }

    std::vector<std::vector<Cell>> grid;
    int startR = 2, startC = 2;
    int goalR  = 16, goalC = 30;

    void reset();      // clear search artifacts, keep walls
    void clearAll();   // wipe everything
    void generateMaze();
signals:
    void cellEdited();
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
private:
    int rows_ = 20, cols_ = 35;
    bool paintingWall_ = true;
    void editAt(const QPoint &pos);
};

class PathView : public QWidget {
    Q_OBJECT
public:
    explicit PathView(QWidget *parent = nullptr);
private slots:
    void run();
    void showFrame(int idx);
private:
    struct Frame { int r, c; Cell type; };
    void buildBFS();
    void buildDFS();
    void buildDijkstra();
    void buildAStar();
    void reconstruct(const std::vector<std::vector<std::pair<int,int>>> &prev);
    int neighborCount() const;   // 4 or 8 depending on diagonal toggle

    std::vector<Frame> frames_;
    std::vector<std::vector<Cell>> baseGrid_; // snapshot for scrubbing
    int baseStartR_, baseStartC_, baseGoalR_, baseGoalC_;

    GridCanvas *canvas_ = nullptr;
    QComboBox  *algoBox_ = nullptr;
    QPushButton *runBtn_ = nullptr;
    QPushButton *clearBtn_ = nullptr;
    QPushButton *mazeBtn_ = nullptr;
    QCheckBox  *diagChk_ = nullptr;
    QLabel *statusLbl_ = nullptr;
    PlaybackBar *bar_ = nullptr;
};
