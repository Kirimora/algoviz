#pragma once
#include <QWidget>
#include <vector>
#include <QString>

class QComboBox;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QLabel;
class PlaybackBar;

class DPCanvas : public QWidget {
    Q_OBJECT
public:
    explicit DPCanvas(QWidget *parent = nullptr);
    std::vector<std::vector<int>> table;
    std::vector<std::vector<bool>> filled;
    std::vector<std::vector<bool>> onPath;   // backtracked solution path
    int curR = -1, curC = -1;
    QStringList rowLabels, colLabels;
    void update_();
protected:
    void paintEvent(QPaintEvent *) override;
};

class DPView : public QWidget {
    Q_OBJECT
public:
    explicit DPView(QWidget *parent = nullptr);
private slots:
    void rebuild();
    void showFrame(int idx);
private:
    struct Frame { int r, c, val; };
    void buildKnapsack();
    void buildLCS();
    void markPath();

    std::vector<Frame> frames_;
    std::vector<std::vector<int>> finalTable_;
    std::vector<std::vector<bool>> pathMask_;
    QStringList rowLabels_, colLabels_;

    DPCanvas *canvas_ = nullptr;
    QComboBox *algoBox_ = nullptr;
    QCheckBox *animateChk_ = nullptr;
    QLineEdit *input1_ = nullptr;
    QLineEdit *input2_ = nullptr;
    QLabel *lbl1_ = nullptr;
    QLabel *lbl2_ = nullptr;
    QPushButton *buildBtn_ = nullptr;
    QLabel *recurrenceLbl_ = nullptr;
    QLabel *statusLbl_ = nullptr;
    PlaybackBar *bar_ = nullptr;
};
