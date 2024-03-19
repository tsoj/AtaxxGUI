#pragma once

#include <QLCDNumber>
#include <QTime>
#include <QTimer>

class CountdownTimer : public QLCDNumber {
    Q_OBJECT

   public:
    CountdownTimer(QWidget *parent = nullptr);

   public slots:

    void start_clock();
    void stop_clock();
    void set_time(QTime time);
    void tick();

   private:
    QTimer m_timer;
    QTime m_remaining_time;
};