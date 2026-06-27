#include "PlaybackBar.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

PlaybackBar::PlaybackBar(QWidget *parent) : QWidget(parent) {
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    backBtn_  = new QPushButton("⏮");
    playBtn_  = new QPushButton("▶");
    pauseBtn_ = new QPushButton("⏸");
    fwdBtn_   = new QPushButton("⏭");
    scrub_    = new QSlider(Qt::Horizontal);
    scrub_->setRange(0, 0);
    speed_    = new QSlider(Qt::Horizontal);
    speed_->setRange(1, 200);          // ms per frame
    speed_->setValue(20);
    speed_->setMaximumWidth(120);
    counter_  = new QLabel("0 / 0");
    counter_->setMinimumWidth(80);

    for (auto *b : {backBtn_, playBtn_, pauseBtn_, fwdBtn_})
        b->setMaximumWidth(40);

    lay->addWidget(backBtn_);
    lay->addWidget(playBtn_);
    lay->addWidget(pauseBtn_);
    lay->addWidget(fwdBtn_);
    lay->addWidget(scrub_, 1);
    lay->addWidget(counter_);
    lay->addWidget(new QLabel("Speed:"));
    lay->addWidget(speed_);

    connect(playBtn_,  &QPushButton::clicked, this, &PlaybackBar::play);
    connect(pauseBtn_, &QPushButton::clicked, this, &PlaybackBar::pause);
    connect(backBtn_,  &QPushButton::clicked, this, &PlaybackBar::stepBack);
    connect(fwdBtn_,   &QPushButton::clicked, this, &PlaybackBar::stepForward);
    connect(scrub_,    &QSlider::valueChanged, this, [this](int v){
        if (v != cursor_) { cursor_ = v; emitFrame(); }
    });
    connect(&timer_, &QTimer::timeout, this, &PlaybackBar::tick);
}

int PlaybackBar::intervalMs() const { return speed_->value(); }

void PlaybackBar::setTotalFrames(int n) {
    timer_.stop();
    total_ = n;
    cursor_ = 0;
    scrub_->blockSignals(true);
    scrub_->setRange(0, n > 0 ? n - 1 : 0);
    scrub_->setValue(0);
    scrub_->blockSignals(false);
    counter_->setText(QString("0 / %1").arg(n > 0 ? n - 1 : 0));
}

void PlaybackBar::play() {
    if (total_ == 0) return;
    if (cursor_ >= total_ - 1) { cursor_ = 0; emitFrame(); }
    timer_.start(speed_->value());
}

void PlaybackBar::pause() { timer_.stop(); }

void PlaybackBar::tick() {
    if (cursor_ >= total_ - 1) { timer_.stop(); return; }
    ++cursor_;
    timer_.setInterval(speed_->value());
    emitFrame();
}

void PlaybackBar::stepForward() {
    timer_.stop();
    if (cursor_ < total_ - 1) { ++cursor_; emitFrame(); }
}

void PlaybackBar::stepBack() {
    timer_.stop();
    if (cursor_ > 0) { --cursor_; emitFrame(); }
}

void PlaybackBar::seek(int idx) {
    if (idx < 0 || idx >= total_) return;
    cursor_ = idx;
    emitFrame();
}

void PlaybackBar::emitFrame() {
    scrub_->blockSignals(true);
    scrub_->setValue(cursor_);
    scrub_->blockSignals(false);
    counter_->setText(QString("%1 / %2").arg(cursor_).arg(total_ > 0 ? total_ - 1 : 0));
    emit frameChanged(cursor_);
}
