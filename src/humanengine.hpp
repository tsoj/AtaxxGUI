#pragma once

#include <qobject.h>
#include <../core/engine/engine.hpp>
#include <libataxx/move.hpp>
#include <mutex>

class HumanEngine : public QObject, public Engine {
    Q_OBJECT

   public:
    [[nodiscard]] HumanEngine();

    [[nodiscard]] auto go(const SearchSettings &settings) -> std::string final;

    auto position(const libataxx::Position &pos) -> void final;

    auto set_option([[maybe_unused]] const std::string &name, [[maybe_unused]] const std::string &value) -> void final {
    }

    auto init() -> void final {
    }

    auto isready() -> void final {
    }

    auto newgame() -> void final {
    }

   signals:
    void need_human_move_input(bool);

   public slots:
    void on_human_move(const libataxx::Move move, const libataxx::Side side);

   protected:
    [[nodiscard]] auto is_running() -> bool final;

    auto quit() -> void final;

    auto stop() -> void final;

   private:
    libataxx::Position m_position;
    std::mutex m_mutex;
    std::optional<libataxx::Move> m_human_move;
    bool m_infinite_time = false;
};