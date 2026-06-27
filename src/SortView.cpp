#include "SortView.h"
#include "PlaybackBar.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <random>
#include <algorithm>
#include <functional>

// ---------- SortCanvas ----------
SortCanvas::SortCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(300);
}

void SortCanvas::setStep(const SortStep &s) { step_ = s; update(); }

void SortCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0x1e, 0x1e, 0x2e));
    const auto &d = step_.data;
    if (d.empty()) return;

    const int n = (int)d.size();
    const double w = (double)width() / n;
    const int maxVal = *std::max_element(d.begin(), d.end());
    const double hScale = (height() - 4.0) / std::max(1, maxVal);

    for (int i = 0; i < n; ++i) {
        QColor c(0x89, 0xb4, 0xfa);
        if (step_.sortedFrom >= 0 && i >= step_.sortedFrom) c = QColor(0xa6, 0xe3, 0xa1);
        if (i == step_.a) c = QColor(0xf3, 0x8b, 0xa8);
        if (i == step_.b) c = QColor(0xf9, 0xe2, 0xaf);
        const double barH = d[i] * hScale;
        p.fillRect(QRectF(i * w, height() - barH, std::max(1.0, w * 0.9), barH), c);
    }
}

// ---------- SortView ----------
SortView::SortView(QWidget *parent) : QWidget(parent) {
    auto *root = new QVBoxLayout(this);
    auto *controls = new QHBoxLayout;

    algoBox_ = new QComboBox;
    algoBox_->addItems({"Bubble Sort", "Insertion Sort", "Selection Sort",
                        "Quick Sort", "Merge Sort", "Heap Sort", "Shell Sort"});

    sizeSpin_ = new QSpinBox; sizeSpin_->setRange(5, 200); sizeSpin_->setValue(60);
    maxSpin_  = new QSpinBox; maxSpin_->setRange(10, 500); maxSpin_->setValue(100);
    genBtn_   = new QPushButton("Shuffle");
    statusLbl_ = new QLabel("Ready");

    controls->addWidget(new QLabel("Algorithm:"));
    controls->addWidget(algoBox_);
    controls->addWidget(new QLabel("Size:"));
    controls->addWidget(sizeSpin_);
    controls->addWidget(new QLabel("Max value:"));
    controls->addWidget(maxSpin_);
    controls->addWidget(genBtn_);
    controls->addStretch();

    canvas_ = new SortCanvas;
    bar_ = new PlaybackBar;

    root->addLayout(controls);
    root->addWidget(canvas_, 1);
    root->addWidget(bar_);
    root->addWidget(statusLbl_);

    connect(genBtn_, &QPushButton::clicked, this, &SortView::regenerate);
    connect(algoBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int){ rebuild(); });
    connect(sizeSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int){ regenerate(); });
    connect(maxSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int){ regenerate(); });
    connect(bar_, &PlaybackBar::frameChanged, this, &SortView::showFrame);

    regenerate();
}

void SortView::regenerate() {
    base_.resize(sizeSpin_->value());
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(5, maxSpin_->value());
    for (auto &x : base_) x = dist(rng);
    rebuild();
}

void SortView::rebuild() {
    buildSteps();
    bar_->setTotalFrames((int)steps_.size());
    if (!steps_.empty()) canvas_->setStep(steps_.front());
    statusLbl_->setText(QString("%1 — %2 steps").arg(algoBox_->currentText()).arg(steps_.size()));
}

void SortView::showFrame(int idx) {
    if (idx >= 0 && idx < (int)steps_.size()) canvas_->setStep(steps_[idx]);
}

void SortView::buildSteps() {
    steps_.clear();
    std::vector<int> v = base_;
    switch (algoBox_->currentIndex()) {
        case 0: genBubble(v); break;
        case 1: genInsertion(v); break;
        case 2: genSelection(v); break;
        case 3: genQuick(v); break;
        case 4: genMerge(v); break;
        case 5: genHeap(v); break;
        case 6: genShell(v); break;
    }
    SortStep fin; fin.data = base_;
    std::sort(fin.data.begin(), fin.data.end());
    fin.sortedFrom = 0;
    steps_.push_back(fin);
}

void SortView::genBubble(std::vector<int> v) {
    const int n = (int)v.size();
    for (int i = 0; i < n - 1; ++i)
        for (int j = 0; j < n - i - 1; ++j) {
            steps_.push_back({v, j, j + 1, n - i});
            if (v[j] > v[j + 1]) { std::swap(v[j], v[j + 1]); steps_.push_back({v, j, j + 1, n - i}); }
        }
}

void SortView::genInsertion(std::vector<int> v) {
    const int n = (int)v.size();
    for (int i = 1; i < n; ++i) {
        int key = v[i], j = i - 1;
        while (j >= 0 && v[j] > key) { v[j + 1] = v[j]; steps_.push_back({v, j, j + 1, -1}); --j; }
        v[j + 1] = key; steps_.push_back({v, j + 1, i, -1});
    }
}

void SortView::genSelection(std::vector<int> v) {
    const int n = (int)v.size();
    for (int i = 0; i < n - 1; ++i) {
        int mn = i;
        for (int j = i + 1; j < n; ++j) { steps_.push_back({v, mn, j, i}); if (v[j] < v[mn]) mn = j; }
        std::swap(v[i], v[mn]); steps_.push_back({v, i, mn, i + 1});
    }
}

void SortView::genQuick(std::vector<int> v) {
    std::vector<std::pair<int,int>> st{{0, (int)v.size() - 1}};
    while (!st.empty()) {
        auto [lo, hi] = st.back(); st.pop_back();
        if (lo >= hi) continue;
        int pivot = v[hi], i = lo - 1;
        for (int j = lo; j < hi; ++j) {
            steps_.push_back({v, j, hi, -1});
            if (v[j] < pivot) { ++i; std::swap(v[i], v[j]); steps_.push_back({v, i, j, -1}); }
        }
        std::swap(v[i + 1], v[hi]); steps_.push_back({v, i + 1, hi, -1});
        st.push_back({lo, i}); st.push_back({i + 2, hi});
    }
}

void SortView::genMerge(std::vector<int> v) {
    const int n = (int)v.size();
    for (int width = 1; width < n; width *= 2)
        for (int lo = 0; lo < n; lo += 2 * width) {
            int mid = std::min(lo + width, n), hi = std::min(lo + 2 * width, n);
            std::vector<int> merged; int i = lo, j = mid;
            while (i < mid && j < hi) merged.push_back(v[i] <= v[j] ? v[i++] : v[j++]);
            while (i < mid) merged.push_back(v[i++]);
            while (j < hi)  merged.push_back(v[j++]);
            for (int k = 0; k < (int)merged.size(); ++k) { v[lo + k] = merged[k]; steps_.push_back({v, lo + k, mid, -1}); }
        }
}

void SortView::genHeap(std::vector<int> v) {
    const int n = (int)v.size();
    std::function<void(int,int)> heapify = [&](int size, int root) {
        int largest = root, l = 2 * root + 1, r = 2 * root + 2;
        if (l < size && v[l] > v[largest]) largest = l;
        if (r < size && v[r] > v[largest]) largest = r;
        if (largest != root) {
            std::swap(v[root], v[largest]);
            steps_.push_back({v, root, largest, -1});
            heapify(size, largest);
        }
    };
    for (int i = n / 2 - 1; i >= 0; --i) heapify(n, i);
    for (int i = n - 1; i > 0; --i) {
        std::swap(v[0], v[i]);
        steps_.push_back({v, 0, i, i});
        heapify(i, 0);
    }
}

void SortView::genShell(std::vector<int> v) {
    const int n = (int)v.size();
    for (int gap = n / 2; gap > 0; gap /= 2)
        for (int i = gap; i < n; ++i) {
            int tmp = v[i], j;
            for (j = i; j >= gap && v[j - gap] > tmp; j -= gap) {
                v[j] = v[j - gap]; steps_.push_back({v, j, j - gap, -1});
            }
            v[j] = tmp; steps_.push_back({v, j, i, -1});
        }
}
