#include "countdowntimer.hpp"

CountdownTimer::CountdownTimer(QWidget *parent) : QLCDNumber(parent) {
    setSegmentStyle(Filled);
    connect(&m_timer, &QTimer::timeout, this, &CountdownTimer::tick);
}

void CountdownTimer::start_clock() {
    m_timer.start(1000);  // Start the timer to tick every second
}
void CountdownTimer::stop_clock() {
    m_timer.stop();
}

void CountdownTimer::set_time(QTime time) {
    m_remaining_time = time;
    display(m_remaining_time.toString("hh:mm:ss"));
}
void CountdownTimer::tick() {
    const auto old = m_remaining_time;
    m_remaining_time = m_remaining_time.addSecs(-1);
    if (old < m_remaining_time) {
        m_remaining_time = QTime(0, 0);
    }
    display(m_remaining_time.toString("hh:mm:ss"));
}