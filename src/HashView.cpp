#include "HashView.h"
#include "PlaybackBar.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>

// ---------- HashCanvas ----------
HashCanvas::HashCanvas(QWidget *parent) : QWidget(parent) { setMinimumHeight(160); }

void HashCanvas::apply(const std::vector<int> &s, int pr, int pl, int hm) {
    cells = s; probe = pr; placed = pl; home = hm; update();
}

void HashCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0x1e, 0x1e, 0x2e));
    if (cells.empty()) return;
    const int n = (int)cells.size();
    const double w = (double)width() / n;
    p.setFont(QFont("monospace", 10));
    for (int i = 0; i < n; ++i) {
        QColor col(0x31, 0x32, 0x44);
        if (cells[i] != -1) col = QColor(0x89, 0xb4, 0xfa);
        if (i == home)   col = QColor(0x94, 0xe2, 0xd5); // teal = ideal slot
        if (i == probe)  col = QColor(0xf9, 0xe2, 0xaf); // yellow = probing
        if (i == placed) col = QColor(0xa6, 0xe3, 0xa1); // green = just placed
        QRectF cell(i * w + 2, 30, w - 4, height() - 60);
        p.fillRect(cell, col);
        p.setPen(QColor(0xcd, 0xd6, 0xf4));
        p.drawText(QRectF(i * w, 6, w, 20), Qt::AlignCenter, QString::number(i));
        if (cells[i] != -1)
            p.drawText(cell, Qt::AlignCenter, QString::number(cells[i]));
    }
}

// ---------- HashView ----------
HashView::HashView(QWidget *parent) : QWidget(parent) {
    auto *root = new QVBoxLayout(this);
    auto *controls = new QHBoxLayout;

    probeBox_ = new QComboBox;
    probeBox_->addItems({"Linear probing", "Quadratic probing", "Double hashing"});
    capSpin_ = new QSpinBox; capSpin_->setRange(5, 41); capSpin_->setValue(11);
    keysEdit_ = new QLineEdit("18, 41, 22, 44, 59, 32, 31, 73");
    buildBtn_ = new QPushButton("Build");
    statusLbl_ = new QLabel("Enter comma-separated keys, then Build");

    controls->addWidget(new QLabel("Strategy:"));
    controls->addWidget(probeBox_);
    controls->addWidget(new QLabel("Capacity:"));
    controls->addWidget(capSpin_);
    controls->addWidget(new QLabel("Keys:"));
    controls->addWidget(keysEdit_, 1);
    controls->addWidget(buildBtn_);

    canvas_ = new HashCanvas;
    bar_ = new PlaybackBar;

    root->addLayout(controls);
    root->addWidget(canvas_, 1);
    root->addWidget(bar_);
    root->addWidget(statusLbl_);

    connect(buildBtn_, &QPushButton::clicked, this, &HashView::rebuild);
    connect(probeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){ rebuild(); });
    connect(capSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int){ rebuild(); });
    connect(bar_, &PlaybackBar::frameChanged, this, &HashView::showFrame);

    rebuild();
}

int HashView::hash(int key) const { return key % capacity_; }

void HashView::rebuild() {
    capacity_ = capSpin_->value();
    std::vector<int> keys;
    for (const QString &part : keysEdit_->text().split(',', Qt::SkipEmptyParts)) {
        bool ok = false;
        int v = part.trimmed().toInt(&ok);
        if (ok) keys.push_back(v);
    }
    buildInsertSequence(keys);
    bar_->setTotalFrames((int)frames_.size());
    if (!frames_.empty()) {
        const auto &f = frames_.front();
        canvas_->apply(f.cells, f.probe, f.placed, f.home);
    }
    statusLbl_->setText(QString("%1 — %2 keys, %3 frames")
        .arg(probeBox_->currentText()).arg(keys.size()).arg(frames_.size()));
}

void HashView::insertOne() {} // reserved for future single-step insert

void HashView::buildInsertSequence(const std::vector<int> &keys) {
    frames_.clear();
    std::vector<int> table(capacity_, -1);
    const int strat = probeBox_->currentIndex();

    for (int key : keys) {
        int home = hash(key);
        bool placed = false;
        for (int i = 0; i < capacity_ && !placed; ++i) {
            int slot;
            switch (strat) {
                case 0: slot = (home + i) % capacity_; break;                 // linear
                case 1: slot = (home + i * i) % capacity_; break;             // quadratic
                default: { int step = 1 + (key % (capacity_ - 1));            // double hashing
                           slot = (home + i * step) % capacity_; } break;
            }
            if (table[slot] == -1) {
                table[slot] = key;
                frames_.push_back({table, slot, slot, home,
                    QString("key %1 placed at %2").arg(key).arg(slot)});
                placed = true;
            } else {
                frames_.push_back({table, slot, -1, home,
                    QString("key %1: slot %2 occupied, probing").arg(key).arg(slot)});
            }
        }
    }
}

void HashView::showFrame(int idx) {
    if (idx < 0 || idx >= (int)frames_.size()) return;
    const auto &f = frames_[idx];
    canvas_->apply(f.cells, f.probe, f.placed, f.home);
    statusLbl_->setText(f.note);
}
