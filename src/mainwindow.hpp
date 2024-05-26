
#pragma once

#include <qmessagebox.h>
#include <../core/parse/settings.hpp>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QThread>
#include <QTimeEdit>
#include <QtCharts/QLineSeries>
#include <map>
#include <string_view>
#include <thread>
#include "boardview/boardscene.hpp"
#include "boardview/boardview.hpp"
#include "countdowntimer.hpp"
// #include "gameworker.hpp"
#include <QMessageBox>
#include <QtCharts/QChartView>
// #include "humanengine.hpp"

class TournamentWorker : public QObject {
    Q_OBJECT

   public:
    TournamentWorker(const std::string& settings_path,
                     const int seconds_between_games,
                     const int milliseconds_between_moves)
        : m_settings_path(settings_path),
          m_seconds_between_games(seconds_between_games),
          m_milliseconds_between_moves(milliseconds_between_moves) {
    }

   public slots:
    void start();

   signals:
    void new_game(const std::string& fen, const std::string& name1, const std::string& name2, int wtime, int btime);
    void game_finished(const libataxx::Result& result);
    void results_update(const Results& results);
    void new_move(const libataxx::Move& move, int ms, int64_t relative_score);

   private:
    std::string m_settings_path;
    int m_seconds_between_games;
    int m_milliseconds_between_moves;
};

class MaterialSlider : public QSlider {
    Q_OBJECT
   public:
    explicit MaterialSlider(QWidget* parent = nullptr) : QSlider(Qt::Horizontal, parent) {
        this->setStyleSheet(
            "QSlider::groove:horizontal {"
            "border: 1px solid transparent;"
            "height: 4px;"  // Groove thickness
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1e1e1e, stop:1 #b1b1b1);"
            "margin: 2px 0;"
            "}"
            "QSlider::handle:horizontal {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dcdcdc, stop:1 #b4b4b4);"
            "border: 1px solid transparent;"
            "height: 26px;"     // handle thickness
            "width: 26px;"      // Handle width
            "margin: -16px 0;"  // Handle margin
            "border-radius: 3px;"
            "}");

        this->setMinimum(-49);  // Set the minimum value
        this->setMaximum(49);   // Set the maximum value
        this->setValue(0);      // Set an initial value if needed
    }

   protected:
    void mousePressEvent([[maybe_unused]] QMouseEvent* event) override {
        // Do nothing to ignore mouse press events
    }

    void mouseMoveEvent([[maybe_unused]] QMouseEvent* event) override {
        // Do nothing to ignore mouse move events
    }

    void mouseReleaseEvent([[maybe_unused]] QMouseEvent* event) override {
        // Do nothing to ignore mouse release events
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(const std::string& settingsFileName,
                        int seconds_between_games,
                        int milliseconds_between_moves,
                        QWidget* parent = nullptr);
    ~MainWindow();

   private:
    void closeEvent(QCloseEvent* event) override {
        QMessageBox::warning(this, "", "You must kill this application with ctrl+c               ");
        event->ignore();
    }

    BoardScene* m_board_scene{nullptr};
    BoardView* m_board_view{nullptr};

    QRadioButton* m_turn_radio_white{nullptr};
    QRadioButton* m_turn_radio_black{nullptr};
    CountdownTimer* m_clock_white{nullptr};
    CountdownTimer* m_clock_black{nullptr};
    QLabel* m_clock_piece_white{nullptr};
    QLabel* m_clock_piece_black{nullptr};

    QTextEdit* m_pgn_text_field{nullptr};

    QLabel* m_material_balance_piece_white{nullptr};
    QLabel* m_material_balance_piece_black{nullptr};

    QComboBox* m_piece_theme_selection{nullptr};
    QComboBox* m_board_theme_selection{nullptr};

    MaterialSlider* m_material_balance_slider{nullptr};

    std::filesystem::path m_settings_file_path;
    std::map<std::string, EngineSettings> m_engines;

    QLabel* m_engine_name_black{nullptr};
    QLabel* m_engine_name_white{nullptr};
    QTableWidget* m_results_table{nullptr};

    std::vector<double> m_scores;
    QChartView* m_chart_view{nullptr};

    TournamentWorker* m_tournament_worker{nullptr};
    QThread m_worker_thread;
};