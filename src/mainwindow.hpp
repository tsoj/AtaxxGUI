
#pragma once

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <QThread>
#include <QTimeEdit>
#include <map>
#include "humanengine.hpp"
#include "boardview/boardscene.hpp"
#include "boardview/boardview.hpp"
#include "countdowntimer.hpp"
#include "gameworker.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   private slots:
    void load_settings();
    void start_game();
    void stop_game();
    void edit_settings();

   private:
    BoardScene* m_board_scene{nullptr};
    BoardView* m_board_view{nullptr};
    std::shared_ptr<HumanEngine> m_human_engine;

    QPushButton* m_fen_set_fen{nullptr};
    QLineEdit* m_fen_text_field{nullptr};
    QComboBox* m_start_pos_selection{nullptr};

    QPushButton* m_toggle_game_button{nullptr};

    QTimeEdit* m_time_spin_box{nullptr};
    QTimeEdit* m_inc_spin_box{nullptr};

    QRadioButton* m_turn_radio_white{nullptr};
    QRadioButton* m_turn_radio_black{nullptr};
    CountdownTimer* m_clock_white{nullptr};
    CountdownTimer* m_clock_black{nullptr};
    QLabel* m_clock_piece_white{nullptr};
    QLabel* m_clock_piece_black{nullptr};

    QTextEdit* m_pgn_text_field{nullptr};

    GameWorker* m_game_worker{nullptr};
    QThread m_worker_thread;

    QLabel* m_selection_piece_white{nullptr};
    QLabel* m_selection_piece_black{nullptr};
    QComboBox* m_engine_selection1{nullptr};
    QComboBox* m_engine_selection2{nullptr};

    QCheckBox* m_human_infinite_time_checkbox{nullptr};

    QComboBox* m_piece_theme_selection{nullptr};
    QComboBox* m_board_theme_selection{nullptr};

    std::filesystem::path m_settings_file_path;
    std::map<std::string, EngineSettings> m_engines;
};