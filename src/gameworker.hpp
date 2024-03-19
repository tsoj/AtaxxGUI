#pragma once

#include <../core/engine/engine.hpp>
#include <../core/engine/process.hpp>
#include <../core/play.hpp>
#include <QThread>

class GameWorker : public QObject {
    Q_OBJECT

   public:
    GameWorker(const AdjudicationSettings &adjudication,
               const GameSettings &game,
               std::shared_ptr<Engine> engine1,
               std::shared_ptr<Engine> engine2);

   public slots:
    void start_game();
    void stopGame();

   signals:
    void finished_game(GameThingy result);
    void new_move(GameThingy info);
    void update_time_control(SearchSettings tc1, SearchSettings tc2, libataxx::Side side_to_move);

   private:
    AdjudicationSettings m_adjudication;
    GameSettings m_game;
    std::shared_ptr<Engine> m_engine1;
    std::shared_ptr<Engine> m_engine2;
    std::atomic_bool m_stop_flag;
};
