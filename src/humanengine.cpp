#include "humanengine.hpp"
#include <chrono>
#include <thread>

[[nodiscard]] HumanEngine::HumanEngine() : Engine({}, {}) {
    std::lock_guard lock(m_mutex);
    m_human_move = libataxx::Move::nomove();
}

[[nodiscard]] auto HumanEngine::go(const SearchSettings &settings) -> std::string {
    const std::chrono::nanoseconds move_time = std::chrono::milliseconds{1} * [this, settings]() {
        if (settings.type == SearchSettings::Type::Movetime) {
            return settings.movetime;
        }
        if (settings.type != SearchSettings::Type::Time) {
            this->m_infinite_time = true;
            return 0;
        }
        if (m_position.get_turn() == libataxx::Side::Black) {
            return settings.btime;
        } else {
            return settings.wtime;
        }
        return 0;
    }();

    const auto start = std::chrono::steady_clock::now();
    {
        std::lock_guard lock(m_mutex);
        m_human_move = std::nullopt;
    }

    auto move = libataxx::Move::nomove();

    if (m_position.legal_moves().size() == 1) {
        move = m_position.legal_moves().back();
    }

    if (m_position.legal_moves().size() > 1) {
        emit need_human_move_input(true);
        while (m_infinite_time ||
               (std::chrono::steady_clock::now() - start) <= move_time + std::chrono::milliseconds{100}) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            std::lock_guard lock(m_mutex);
            if (m_human_move.has_value()) {
                move = m_human_move.value();
                break;
            }
        }
        emit need_human_move_input(false);
    }
    return static_cast<std::string>(move);
};

auto HumanEngine::position(const libataxx::Position &pos) -> void {
    m_position = pos;
}

void HumanEngine::on_human_move(const libataxx::Move move, const libataxx::Side side) {
    std::lock_guard lock(m_mutex);
    if (m_position.get_turn() == side && m_position.is_legal_move(move)) {
        m_human_move = move;
    }
}

[[nodiscard]] auto HumanEngine::is_running() -> bool {
    std::lock_guard lock(m_mutex);
    return !m_human_move.has_value();
};

auto HumanEngine::quit() -> void {
    stop();
};

auto HumanEngine::stop() -> void {
    const auto legal_moves = m_position.legal_moves();
    if (legal_moves.size() > 0) {
        on_human_move(legal_moves.empty() ? libataxx::Move::nomove() : legal_moves.back(), m_position.get_turn());
    }
};