#include "PathView.h"
#include "PlaybackBar.h"
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <queue>
#include <cmath>
#include <array>
#include <random>
#include <climits>

// ---------- GridCanvas ----------
GridCanvas::GridCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumSize(560, 360);
    clearAll();
}

void GridCanvas::clearAll() {
    grid.assign(rows_, std::vector<Cell>(cols_, Cell::Empty));
    grid[startR][startC] = Cell::Start;
    grid[goalR][goalC]   = Cell::Goal;
    update();
}

void GridCanvas::reset() {
    for (auto &row : grid)
        for (auto &c : row)
            if (c == Cell::Visited || c == Cell::Path || c == Cell::Frontier) c = Cell::Empty;
    grid[startR][startC] = Cell::Start;
    grid[goalR][goalC]   = Cell::Goal;
    update();
}

void GridCanvas::generateMaze() {
    // Randomized DFS maze on odd cells; walls everywhere first.
    for (auto &row : grid) std::fill(row.begin(), row.end(), Cell::Wall);
    std::mt19937 rng(std::random_device{}());
    std::vector<std::pair<int,int>> st{{1, 1}};
    grid[1][1] = Cell::Empty;
    const int dr[] = {-2, 2, 0, 0}, dc[] = {0, 0, -2, 2};
    while (!st.empty()) {
        auto [r, c] = st.back();
        std::vector<int> dirs{0, 1, 2, 3};
        std::shuffle(dirs.begin(), dirs.end(), rng);
        bool moved = false;
        for (int d : dirs) {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr > 0 && nr < rows_ - 1 && nc > 0 && nc < cols_ - 1 && grid[nr][nc] == Cell::Wall) {
                grid[(r + nr) / 2][(c + nc) / 2] = Cell::Empty;
                grid[nr][nc] = Cell::Empty;
                st.push_back({nr, nc});
                moved = true;
                break;
            }
        }
        if (!moved) st.pop_back();
    }
    grid[startR][startC] = Cell::Start;
    grid[goalR][goalC]   = Cell::Goal;
    update();
    emit cellEdited();
}

void GridCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0x18, 0x18, 0x25));
    const double cw = (double)width() / cols_, ch = (double)height() / rows_;
    for (int r = 0; r < rows_; ++r)
        for (int c = 0; c < cols_; ++c) {
            QColor col(0x31, 0x32, 0x44);
            switch (grid[r][c]) {
                case Cell::Wall:     col = QColor(0x45, 0x47, 0x5a); break;
                case Cell::Start:    col = QColor(0xa6, 0xe3, 0xa1); break;
                case Cell::Goal:     col = QColor(0xf3, 0x8b, 0xa8); break;
                case Cell::Visited:  col = QColor(0x74, 0xc7, 0xec); break;
                case Cell::Frontier: col = QColor(0xf9, 0xe2, 0xaf); break;
                case Cell::Path:     col = QColor(0xcb, 0xa6, 0xf7); break;
                default: break;
            }
            p.fillRect(QRectF(c * cw, r * ch, cw - 1, ch - 1), col);
        }
}

void GridCanvas::editAt(const QPoint &pos) {
    const int c = pos.x() / (width() / cols_);
    const int r = pos.y() / (height() / rows_);
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) return;
    if (grid[r][c] == Cell::Start || grid[r][c] == Cell::Goal) return;
    grid[r][c] = paintingWall_ ? Cell::Wall : Cell::Empty;
    update();
    emit cellEdited();
}

void GridCanvas::mousePressEvent(QMouseEvent *e) {
    const int c = e->pos().x() / (width() / cols_);
    const int r = e->pos().y() / (height() / rows_);
    if (r >= 0 && r < rows_ && c >= 0 && c < cols_)
        paintingWall_ = (grid[r][c] != Cell::Wall);
    editAt(e->pos());
}

void GridCanvas::mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton) editAt(e->pos());
}

// ---------- PathView ----------
PathView::PathView(QWidget *parent) : QWidget(parent) {
    auto *root = new QVBoxLayout(this);
    auto *controls = new QHBoxLayout;

    algoBox_ = new QComboBox;
    algoBox_->addItems({"BFS", "DFS", "Dijkstra", "A*"});
    runBtn_   = new QPushButton("Run");
    clearBtn_ = new QPushButton("Clear");
    mazeBtn_  = new QPushButton("Generate Maze");
    diagChk_  = new QCheckBox("Diagonal moves");
    statusLbl_ = new QLabel("Click/drag to draw walls");

    controls->addWidget(new QLabel("Algorithm:"));
    controls->addWidget(algoBox_);
    controls->addWidget(diagChk_);
    controls->addWidget(runBtn_);
    controls->addWidget(mazeBtn_);
    controls->addWidget(clearBtn_);
    controls->addStretch();

    canvas_ = new GridCanvas;
    bar_ = new PlaybackBar;

    root->addLayout(controls);
    root->addWidget(canvas_, 1);
    root->addWidget(bar_);
    root->addWidget(statusLbl_);

    connect(runBtn_, &QPushButton::clicked, this, &PathView::run);
    connect(mazeBtn_, &QPushButton::clicked, this, [this]{
        canvas_->generateMaze(); frames_.clear(); bar_->setTotalFrames(0);
        statusLbl_->setText("Maze generated");
    });
    connect(clearBtn_, &QPushButton::clicked, this, [this]{
        canvas_->clearAll(); frames_.clear(); bar_->setTotalFrames(0);
        statusLbl_->setText("Cleared");
    });
    connect(bar_, &PlaybackBar::frameChanged, this, &PathView::showFrame);
}

int PathView::neighborCount() const { return diagChk_->isChecked() ? 8 : 4; }

void PathView::run() {
    canvas_->reset();
    frames_.clear();
    // snapshot the grid (walls + endpoints) so scrubbing can rebuild from scratch
    baseGrid_ = canvas_->grid;
    baseStartR_ = canvas_->startR; baseStartC_ = canvas_->startC;
    baseGoalR_  = canvas_->goalR;  baseGoalC_  = canvas_->goalC;

    switch (algoBox_->currentIndex()) {
        case 0: buildBFS(); break;
        case 1: buildDFS(); break;
        case 2: buildDijkstra(); break;
        case 3: buildAStar(); break;
    }
    bar_->setTotalFrames((int)frames_.size() + 1); // +1 so frame 0 = clean grid
    bar_->seek(0);
    bar_->play();
    statusLbl_->setText(QString("%1 — %2 frames").arg(algoBox_->currentText()).arg(frames_.size()));
}

void PathView::showFrame(int idx) {
    // Rebuild the grid from the snapshot, then replay frames [0, idx).
    canvas_->grid = baseGrid_;
    for (int i = 0; i < idx && i < (int)frames_.size(); ++i) {
        const auto &f = frames_[i];
        auto &cell = canvas_->grid[f.r][f.c];
        if (cell != Cell::Start && cell != Cell::Goal) cell = f.type;
    }
    canvas_->update();
}

static const std::array<int,8> DR{-1, 1, 0, 0, -1, -1, 1, 1};
static const std::array<int,8> DC{0, 0, -1, 1, -1, 1, -1, 1};

void PathView::reconstruct(const std::vector<std::vector<std::pair<int,int>>> &prev) {
    int r = canvas_->goalR, c = canvas_->goalC;
    if (prev[r][c].first == -1 && !(r == canvas_->startR && c == canvas_->startC)) return;
    std::vector<Frame> path;
    while (!(r == canvas_->startR && c == canvas_->startC)) {
        path.push_back({r, c, Cell::Path});
        auto [pr, pc] = prev[r][c];
        if (pr == -1) break;
        r = pr; c = pc;
    }
    for (auto it = path.rbegin(); it != path.rend(); ++it) frames_.push_back(*it);
}

void PathView::buildBFS() {
    const int R = canvas_->rows(), C = canvas_->cols(), K = neighborCount();
    std::vector<std::vector<bool>> seen(R, std::vector<bool>(C, false));
    std::vector<std::vector<std::pair<int,int>>> prev(R, std::vector<std::pair<int,int>>(C, {-1,-1}));
    std::queue<std::pair<int,int>> q;
    q.push({canvas_->startR, canvas_->startC});
    seen[canvas_->startR][canvas_->startC] = true;
    while (!q.empty()) {
        auto [r, c] = q.front(); q.pop();
        frames_.push_back({r, c, Cell::Visited});
        if (r == canvas_->goalR && c == canvas_->goalC) { reconstruct(prev); return; }
        for (int k = 0; k < K; ++k) {
            int nr = r + DR[k], nc = c + DC[k];
            if (nr<0||nr>=R||nc<0||nc>=C) continue;
            if (seen[nr][nc] || canvas_->grid[nr][nc] == Cell::Wall) continue;
            seen[nr][nc] = true; prev[nr][nc] = {r, c};
            frames_.push_back({nr, nc, Cell::Frontier});
            q.push({nr, nc});
        }
    }
    reconstruct(prev);
}

void PathView::buildDFS() {
    const int R = canvas_->rows(), C = canvas_->cols(), K = neighborCount();
    std::vector<std::vector<bool>> seen(R, std::vector<bool>(C, false));
    std::vector<std::vector<std::pair<int,int>>> prev(R, std::vector<std::pair<int,int>>(C, {-1,-1}));
    std::vector<std::pair<int,int>> st{{canvas_->startR, canvas_->startC}};
    while (!st.empty()) {
        auto [r, c] = st.back(); st.pop_back();
        if (seen[r][c]) continue;
        seen[r][c] = true;
        frames_.push_back({r, c, Cell::Visited});
        if (r == canvas_->goalR && c == canvas_->goalC) { reconstruct(prev); return; }
        for (int k = 0; k < K; ++k) {
            int nr = r + DR[k], nc = c + DC[k];
            if (nr<0||nr>=R||nc<0||nc>=C) continue;
            if (seen[nr][nc] || canvas_->grid[nr][nc] == Cell::Wall) continue;
            if (prev[nr][nc].first == -1) prev[nr][nc] = {r, c};
            st.push_back({nr, nc});
        }
    }
    reconstruct(prev);
}

void PathView::buildDijkstra() {
    const int R = canvas_->rows(), C = canvas_->cols(), K = neighborCount();
    std::vector<std::vector<int>> dist(R, std::vector<int>(C, INT_MAX));
    std::vector<std::vector<std::pair<int,int>>> prev(R, std::vector<std::pair<int,int>>(C, {-1,-1}));
    using Node = std::tuple<int,int,int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<>> pq;
    dist[canvas_->startR][canvas_->startC] = 0;
    pq.push({0, canvas_->startR, canvas_->startC});
    while (!pq.empty()) {
        auto [d, r, c] = pq.top(); pq.pop();
        if (d > dist[r][c]) continue;
        frames_.push_back({r, c, Cell::Visited});
        if (r == canvas_->goalR && c == canvas_->goalC) { reconstruct(prev); return; }
        for (int k = 0; k < K; ++k) {
            int nr = r + DR[k], nc = c + DC[k];
            if (nr<0||nr>=R||nc<0||nc>=C) continue;
            if (canvas_->grid[nr][nc] == Cell::Wall) continue;
            int w = (k < 4) ? 10 : 14; // diagonal cost ~sqrt(2)
            if (dist[r][c] + w < dist[nr][nc]) {
                dist[nr][nc] = dist[r][c] + w; prev[nr][nc] = {r, c};
                frames_.push_back({nr, nc, Cell::Frontier});
                pq.push({dist[nr][nc], nr, nc});
            }
        }
    }
    reconstruct(prev);
}

void PathView::buildAStar() {
    const int R = canvas_->rows(), C = canvas_->cols(), K = neighborCount();
    auto h = [&](int r, int c){
        int dr = std::abs(r - canvas_->goalR), dc = std::abs(c - canvas_->goalC);
        return diagChk_->isChecked() ? 10 * std::max(dr, dc) : 10 * (dr + dc);
    };
    std::vector<std::vector<int>> g(R, std::vector<int>(C, INT_MAX));
    std::vector<std::vector<std::pair<int,int>>> prev(R, std::vector<std::pair<int,int>>(C, {-1,-1}));
    using Node = std::tuple<int,int,int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<>> pq;
    g[canvas_->startR][canvas_->startC] = 0;
    pq.push({h(canvas_->startR, canvas_->startC), canvas_->startR, canvas_->startC});
    while (!pq.empty()) {
        auto [f, r, c] = pq.top(); pq.pop();
        frames_.push_back({r, c, Cell::Visited});
        if (r == canvas_->goalR && c == canvas_->goalC) { reconstruct(prev); return; }
        for (int k = 0; k < K; ++k) {
            int nr = r + DR[k], nc = c + DC[k];
            if (nr<0||nr>=R||nc<0||nc>=C) continue;
            if (canvas_->grid[nr][nc] == Cell::Wall) continue;
            int w = (k < 4) ? 10 : 14;
            if (g[r][c] + w < g[nr][nc]) {
                g[nr][nc] = g[r][c] + w; prev[nr][nc] = {r, c};
                frames_.push_back({nr, nc, Cell::Frontier});
                pq.push({g[nr][nc] + h(nr, nc), nr, nc});
            }
        }
    }
    reconstruct(prev);
}
