#pragma once
#include <QWidget>
#include <QTimer>

class QPushButton;
class QSlider;
class QLabel;

// A reusable transport bar: play / pause / step-back / step-forward / scrubber + counter.
// Owners give it a total frame count and connect to frameChanged(idx).
class PlaybackBar : public QWidget {
    Q_OBJECT
public:
    explicit PlaybackBar(QWidget *parent = nullptr);

    void setTotalFrames(int n);   // resets cursor to 0
    int  cursor() const { return cursor_; }
    int  intervalMs() const;

signals:
    void frameChanged(int idx);   // emitted whenever the current frame changes

public slots:
    void play();
    void pause();
    void stepForward();
    void stepBack();
    void seek(int idx);

private slots:
    void tick();

private:
    void emitFrame();

    QTimer timer_;
    int cursor_ = 0;
    int total_ = 0;

    QPushButton *playBtn_;
    QPushButton *pauseBtn_;
    QPushButton *backBtn_;
    QPushButton *fwdBtn_;
    QSlider     *scrub_;
    QSlider     *speed_;
    QLabel      *counter_;
};
