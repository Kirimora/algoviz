#pragma once
#include <QWidget>
#include <vector>

class QComboBox;
class QPushButton;
class QSpinBox;
class QLineEdit;
class QLabel;
class PlaybackBar;

// Visualizes open-addressing hash insertion. Each frame highlights a slot
// being probed and whether it was a hit (placed) or collision (keep probing).
class HashCanvas : public QWidget {
    Q_OBJECT
public:
    explicit HashCanvas(QWidget *parent = nullptr);
    std::vector<int> cells;       // -1 = empty
    int probe = -1;               // slot currently being probed
    int placed = -1;              // slot just filled
    int home = -1;                // ideal slot (hash result)
    void apply(const std::vector<int> &s, int pr, int pl, int hm);
protected:
    void paintEvent(QPaintEvent *) override;
};

class HashView : public QWidget {
    Q_OBJECT
public:
    explicit HashView(QWidget *parent = nullptr);
private slots:
    void rebuild();
    void insertOne();
    void showFrame(int idx);
private:
    struct Frame { std::vector<int> cells; int probe, placed, home; QString note; };
    void buildInsertSequence(const std::vector<int> &keys);
    int hash(int key) const;

    int capacity_ = 11;
    std::vector<int> table_;
    std::vector<Frame> frames_;

    HashCanvas *canvas_ = nullptr;
    QComboBox  *probeBox_ = nullptr;
    QSpinBox   *capSpin_ = nullptr;
    QLineEdit  *keysEdit_ = nullptr;
    QPushButton *buildBtn_ = nullptr;
    QLabel *statusLbl_ = nullptr;
    PlaybackBar *bar_ = nullptr;
};
