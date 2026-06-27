#pragma once
#include <QWidget>
#include <vector>

class QComboBox;
class QPushButton;
class QSpinBox;
class QLabel;
class PlaybackBar;

struct SortStep {
    std::vector<int> data;
    int a = -1;
    int b = -1;
    int sortedFrom = -1;
};

class SortCanvas : public QWidget {
    Q_OBJECT
public:
    explicit SortCanvas(QWidget *parent = nullptr);
    void setStep(const SortStep &s);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    SortStep step_;
};

class SortView : public QWidget {
    Q_OBJECT
public:
    explicit SortView(QWidget *parent = nullptr);

private slots:
    void regenerate();
    void rebuild();
    void showFrame(int idx);

private:
    void buildSteps();
    void genBubble(std::vector<int> v);
    void genInsertion(std::vector<int> v);
    void genSelection(std::vector<int> v);
    void genQuick(std::vector<int> v);
    void genMerge(std::vector<int> v);
    void genHeap(std::vector<int> v);
    void genShell(std::vector<int> v);

    std::vector<int> base_;
    std::vector<SortStep> steps_;

    QComboBox  *algoBox_  = nullptr;
    QPushButton *genBtn_  = nullptr;
    QSpinBox   *sizeSpin_ = nullptr;
    QSpinBox   *maxSpin_  = nullptr;
    QLabel     *statusLbl_ = nullptr;
    PlaybackBar *bar_     = nullptr;
    SortCanvas  *canvas_  = nullptr;
};
