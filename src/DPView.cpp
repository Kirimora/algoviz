#include "DPView.h"
#include "PlaybackBar.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <algorithm>

// ---------- DPCanvas ----------
DPCanvas::DPCanvas(QWidget *parent) : QWidget(parent) { setMinimumHeight(300); }
void DPCanvas::update_() { update(); }

void DPCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0x1e, 0x1e, 0x2e));
    if (table.empty()) return;
    const int rows = (int)table.size(), cols = (int)table[0].size();
    const int labelW = 46, labelH = 24;
    const double cw = std::max(24.0, (double)(width() - labelW) / cols);
    const double chh = std::max(20.0, (double)(height() - labelH) / rows);
    p.setFont(QFont("monospace", 9));

    for (int c = 0; c < cols; ++c)
        if (c < colLabels.size())
            p.setPen(QColor(0x94, 0xe2, 0xd5)),
            p.drawText(QRectF(labelW + c * cw, 0, cw, labelH), Qt::AlignCenter, colLabels[c]);
    for (int r = 0; r < rows; ++r)
        if (r < rowLabels.size())
            p.setPen(QColor(0xf9, 0xe2, 0xaf)),
            p.drawText(QRectF(0, labelH + r * chh, labelW, chh), Qt::AlignCenter, rowLabels[r]);

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            QRectF cell(labelW + c * cw, labelH + r * chh, cw - 2, chh - 2);
            QColor col(0x31, 0x32, 0x44);
            if (!filled.empty() && filled[r][c]) col = QColor(0x45, 0x47, 0x5a);
            if (!onPath.empty() && onPath[r][c]) col = QColor(0xcb, 0xa6, 0xf7);
            if (r == curR && c == curC) col = QColor(0xa6, 0xe3, 0xa1);
            p.fillRect(cell, col);
            if (!filled.empty() && filled[r][c]) {
                p.setPen(QColor(0xcd, 0xd6, 0xf4));
                p.drawText(cell, Qt::AlignCenter, QString::number(table[r][c]));
            }
        }
}

// ---------- DPView ----------
DPView::DPView(QWidget *parent) : QWidget(parent) {
    auto *root = new QVBoxLayout(this);
    auto *controls = new QHBoxLayout;

    algoBox_ = new QComboBox;
    algoBox_->addItems({"0/1 Knapsack", "Longest Common Subsequence"});
    animateChk_ = new QCheckBox("Animate fill");
    animateChk_->setChecked(true);
    lbl1_ = new QLabel("Weights:");
    input1_ = new QLineEdit("1, 3, 4, 5");
    lbl2_ = new QLabel("Values / Capacity:");
    input2_ = new QLineEdit("1, 4, 5, 7 | 7");
    buildBtn_ = new QPushButton("Build");
    statusLbl_ = new QLabel("Ready");
    recurrenceLbl_ = new QLabel;
    recurrenceLbl_->setWordWrap(true);
    recurrenceLbl_->setStyleSheet("color:#94e2d5; font-family:monospace;");

    controls->addWidget(new QLabel("Problem:"));
    controls->addWidget(algoBox_);
    controls->addWidget(animateChk_);
    controls->addWidget(lbl1_);
    controls->addWidget(input1_);
    controls->addWidget(lbl2_);
    controls->addWidget(input2_);
    controls->addWidget(buildBtn_);

    canvas_ = new DPCanvas;
    bar_ = new PlaybackBar;

    root->addLayout(controls);
    root->addWidget(recurrenceLbl_);
    root->addWidget(canvas_, 1);
    root->addWidget(bar_);
    root->addWidget(statusLbl_);

    connect(buildBtn_, &QPushButton::clicked, this, &DPView::rebuild);
    connect(animateChk_, &QCheckBox::toggled, this, [this](bool){ rebuild(); });
    connect(algoBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
        if (idx == 0) { lbl1_->setText("Weights:"); input1_->setText("1, 3, 4, 5");
                        lbl2_->setText("Values | Cap:"); input2_->setText("1, 4, 5, 7 | 7"); }
        else { lbl1_->setText("String A:"); input1_->setText("AGCAT");
               lbl2_->setText("String B:"); input2_->setText("GAC"); }
        rebuild();
    });
    connect(bar_, &PlaybackBar::frameChanged, this, &DPView::showFrame);

    rebuild();
}

void DPView::rebuild() {
    frames_.clear();
    if (algoBox_->currentIndex() == 0) buildKnapsack();
    else buildLCS();

    canvas_->rowLabels = rowLabels_;
    canvas_->colLabels = colLabels_;
    const int R = (int)finalTable_.size(), C = R ? (int)finalTable_[0].size() : 0;
    canvas_->table = finalTable_;
    canvas_->onPath = pathMask_;

    if (animateChk_->isChecked()) {
        canvas_->filled.assign(R, std::vector<bool>(C, false));
        bar_->setTotalFrames((int)frames_.size() + 1);
        bar_->seek(0);
        bar_->play();
    } else {
        canvas_->filled.assign(R, std::vector<bool>(C, true));
        canvas_->curR = canvas_->curC = -1;
        bar_->setTotalFrames(0);
    }
    canvas_->update_();
}

void DPView::showFrame(int idx) {
    const int R = (int)finalTable_.size(), C = R ? (int)finalTable_[0].size() : 0;
    canvas_->filled.assign(R, std::vector<bool>(C, false));
    canvas_->table.assign(R, std::vector<int>(C, 0));
    for (int i = 0; i < idx && i < (int)frames_.size(); ++i) {
        const auto &f = frames_[i];
        canvas_->table[f.r][f.c] = f.val;
        canvas_->filled[f.r][f.c] = true;
    }
    if (idx > 0 && idx <= (int)frames_.size()) {
        canvas_->curR = frames_[idx - 1].r;
        canvas_->curC = frames_[idx - 1].c;
    } else { canvas_->curR = canvas_->curC = -1; }
    
    canvas_->onPath = (idx >= (int)frames_.size()) ? pathMask_
                       : std::vector<std::vector<bool>>(R, std::vector<bool>(C, false));
    canvas_->update_();
}

static std::vector<int> parseInts(const QString &s) {
    std::vector<int> out;
    for (const QString &p : s.split(',', Qt::SkipEmptyParts)) {
        bool ok = false; int v = p.trimmed().toInt(&ok);
        if (ok) out.push_back(v);
    }
    return out;
}

void DPView::buildKnapsack() {
    auto weights = parseInts(input1_->text());
    QStringList parts = input2_->text().split('|');
    auto values = parseInts(parts.value(0));
    int cap = parts.size() > 1 ? parts[1].trimmed().toInt() : 10;
    int n = std::min(weights.size(), values.size());
    weights.resize(n); values.resize(n);

    finalTable_.assign(n + 1, std::vector<int>(cap + 1, 0));
    rowLabels_.clear(); colLabels_.clear();
    rowLabels_ << "∅";
    for (int i = 0; i < n; ++i) rowLabels_ << QString("w%1=%2").arg(i + 1).arg(weights[i]);
    for (int w = 0; w <= cap; ++w) colLabels_ << QString::number(w);

    for (int i = 0; i <= n; ++i)
        for (int w = 0; w <= cap; ++w) {
            int val;
            if (i == 0 || w == 0) val = 0;
            else if (weights[i - 1] <= w)
                val = std::max(finalTable_[i - 1][w],
                               values[i - 1] + finalTable_[i - 1][w - weights[i - 1]]);
            else val = finalTable_[i - 1][w];
            finalTable_[i][w] = val;
            frames_.push_back({i, w, val});
        }

   
    pathMask_.assign(n + 1, std::vector<bool>(cap + 1, false));
    int w = cap;
    for (int i = n; i > 0 && w >= 0; --i) {
        if (finalTable_[i][w] != finalTable_[i - 1][w]) {
            pathMask_[i][w] = true;
            w -= weights[i - 1];
        }
    }
    recurrenceLbl_->setText("dp[i][w] = max(dp[i-1][w], value[i] + dp[i-1][w - weight[i]])   "
                            "→ best value = " + QString::number(finalTable_[n][cap]));
}

void DPView::buildLCS() {
    QString a = input1_->text().trimmed();
    QString b = input2_->text().trimmed();
    int n = a.size(), m = b.size();
    finalTable_.assign(n + 1, std::vector<int>(m + 1, 0));
    rowLabels_.clear(); colLabels_.clear();
    rowLabels_ << "∅"; for (QChar ch : a) rowLabels_ << QString(ch);
    colLabels_ << "∅"; for (QChar ch : b) colLabels_ << QString(ch);

    for (int i = 0; i <= n; ++i)
        for (int j = 0; j <= m; ++j) {
            int val;
            if (i == 0 || j == 0) val = 0;
            else if (a[i - 1] == b[j - 1]) val = finalTable_[i - 1][j - 1] + 1;
            else val = std::max(finalTable_[i - 1][j], finalTable_[i][j - 1]);
            finalTable_[i][j] = val;
            frames_.push_back({i, j, val});
        }

    pathMask_.assign(n + 1, std::vector<bool>(m + 1, false));
    int i = n, j = m; QString lcs;
    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) { pathMask_[i][j] = true; lcs.prepend(a[i - 1]); --i; --j; }
        else if (finalTable_[i - 1][j] >= finalTable_[i][j - 1]) --i;
        else --j;
    }
    recurrenceLbl_->setText("dp[i][j] = a[i]==b[j] ? dp[i-1][j-1]+1 : max(dp[i-1][j], dp[i][j-1])   "
                            "→ LCS = \"" + lcs + "\" (length " + QString::number(finalTable_[n][m]) + ")");
}

void DPView::markPath() {}
