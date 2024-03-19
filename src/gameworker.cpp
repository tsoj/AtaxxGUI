#include "gameworker.hpp"

GameWorker::GameWorker(const AdjudicationSettings &adjudication,
                       const GameSettings &game,
                       std::shared_ptr<Engine> engine1,
                       std::shared_ptr<Engine> engine2)
    : m_adjudication(adjudication), m_game(game), m_engine1(engine1), m_engine2(engine2) {
}

void GameWorker::start_game() {
    m_stop_flag = false;

    emit update_time_control(m_game.engine1.tc, m_game.engine2.tc, libataxx::Position(m_game.fen).get_turn());
    const auto result = play(
        m_adjudication, m_game, m_engine1, m_engine2, [this](GameThingy info, SearchSettings tc1, SearchSettings tc2) {
            Q_ASSERT(info.history.size() > 0);
            if (m_stop_flag) {
                return false;
            }
            emit new_move(info);
            emit update_time_control(tc1, tc2, info.endpos.get_turn());
            return true;
        });
    emit finished_game(result);
}
void GameWorker::stopGame() {
    m_stop_flag = true;
    for (auto engine : std::vector{m_engine1, m_engine2}) {
        engine->quit();
        if (ProcessEngine *pe = dynamic_cast<ProcessEngine *>(engine.get())) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            pe->kill();
        }
    }
}