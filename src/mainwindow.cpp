#include "mainwindow.hpp"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineseries.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <../core/engine/create.hpp>
#include <../core/match/callbacks.hpp>
#include <../core/match/run.hpp>
#include <../core/parse/openings.hpp>
#include <../core/parse/settings.hpp>
#include <../core/pgn.hpp>
#include <QApplication>
#include <QDir>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QSplineSeries>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QtCharts/QChart>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libataxx/move.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "boardview/boardscene.hpp"
#include "boardview/boardview.hpp"
#include "boardview/images.hpp"

namespace {

void set_label_piece_pixmap(QLabel *label, libataxx::Piece piece, int size) {
    label->setPixmap(PieceImages::piece_images(piece).scaledToHeight(size, Qt::SmoothTransformation));
    label->setMaximumHeight(size);
    label->setMaximumWidth(size);
    label->setScaledContents(true);
    label->show();
}

void fill_table(QTableWidget *results_table, const Results &results) {
    results_table->clear();
    results_table->clearContents();
    results_table->setRowCount(0);
    results_table->setColumnCount(6);

    // Set the column headers
    QStringList headers = {"Engine", "Score", "Wins", "Losses", "Draws", "Crashes"};
    results_table->setHorizontalHeaderLabels(headers);

    // Set the column data types by setting appropriate item flags and validators if necessary
    // For simplicity, this example does not include validators, but you can add them as needed

    // Adjust the header to fit the content
    // results_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    std::vector<std::tuple<std::string, Score, int>> scores;
    for (const auto &[engine, score] : results.scores) {
        // Score (string)
        const int rank = (100 * (score.draws + 2 * score.wins)) / std::max(2 * score.played, 1);
        scores.emplace_back(engine, score, rank);
    }

    std::sort(scores.begin(), scores.end(), [](auto a, auto b) {
        return std::get<2>(a) > std::get<2>(b);
    });

    for (const auto &[engine, score, rank] : scores) {
        const int row = results_table->rowCount();
        results_table->insertRow(row);

        // Engine (string)
        auto *engineItem = new QTableWidgetItem(engine.c_str());
        results_table->setItem(row, 0, engineItem);

        // Score (string)
        auto *rankItem = new QTableWidgetItem((std::to_string(rank) + " %").c_str());
        results_table->setItem(row, 1, rankItem);

        // Wins (int)
        auto *winsItem = new QTableWidgetItem();
        winsItem->setData(Qt::EditRole, QVariant(score.wins));
        results_table->setItem(row, 2, winsItem);

        // Losses (int)
        auto *lossesItem = new QTableWidgetItem();
        lossesItem->setData(Qt::EditRole, QVariant(score.losses));
        results_table->setItem(row, 3, lossesItem);

        // Draws (int)
        auto *drawsItem = new QTableWidgetItem();
        drawsItem->setData(Qt::EditRole, QVariant(score.draws));
        results_table->setItem(row, 4, drawsItem);

        // Crashes (int)
        auto *crashesItem = new QTableWidgetItem();
        crashesItem->setData(Qt::EditRole, QVariant(score.crashes));
        results_table->setItem(row, 5, crashesItem);
    }

    results_table->setVisible(false);
    results_table->resizeColumnsToContents();
    results_table->setVisible(true);
}

}  // namespace

MainWindow::MainWindow(const std::string &settings_path,
                       const int seconds_between_games,
                       const int milliseconds_between_moves,
                       QWidget *parent)
    : QMainWindow(parent) {
    m_board_scene = new BoardScene(this);
    m_board_view = new BoardView(m_board_scene, this);
    auto *central_widget = new QWidget(this);
    auto *main_layout = new QHBoxLayout(central_widget);
    auto *left_layout = new QVBoxLayout();
    m_clock_piece_white = new QLabel(this);
    m_clock_piece_black = new QLabel(this);
    auto *middle_layout = new QVBoxLayout();
    auto *clock_layout = new QHBoxLayout();
    m_material_balance_piece_white = new QLabel(this);
    m_material_balance_piece_black = new QLabel(this);
    m_turn_radio_white = new QRadioButton;
    m_turn_radio_black = new QRadioButton;
    m_clock_white = new CountdownTimer(this);
    m_clock_black = new CountdownTimer(this);
    auto *material_balance_layout = new QHBoxLayout();
    auto *right_layout = new QVBoxLayout();
    m_pgn_text_field = new QTextEdit(this);
    m_piece_theme_selection = new QComboBox(this);
    m_board_theme_selection = new QComboBox(this);
    m_material_balance_slider = new MaterialSlider(this);

    m_engine_name_black = new QLabel(this);
    m_engine_name_white = new QLabel(this);

    m_results_table = new QTableWidget(this);
    m_chart_view = new QChartView(this);
    m_material_label = new QLabel(this);

    PieceImages::load();

    // Create central widget
    setCentralWidget(central_widget);

    m_piece_theme_selection->setPlaceholderText("Select piece theme");
    m_board_theme_selection->setPlaceholderText("Select board theme");

    m_piece_theme_selection->addItems(QDir(PieceImages::path_to_themes()).entryList(QDir::Dirs | QDir::NoDotAndDotDot));
    m_board_theme_selection->addItems(QDir(BoardImage::path_to_themes()).entryList(QDir::Dirs | QDir::NoDotAndDotDot));

    connect(m_piece_theme_selection, &QComboBox::currentTextChanged, [this](QString text) {
        if (this->m_piece_theme_selection->currentIndex() != -1) {
            PieceImages::load(text.toStdString());
            this->m_board_scene->reload();

            set_label_piece_pixmap(this->m_clock_piece_white, libataxx::Piece::White, this->m_clock_white->height());
            set_label_piece_pixmap(this->m_clock_piece_black, libataxx::Piece::Black, this->m_clock_black->height());

            set_label_piece_pixmap(
                m_material_balance_piece_white, libataxx::Piece::White, m_material_balance_slider->height());
            set_label_piece_pixmap(
                m_material_balance_piece_black, libataxx::Piece::Black, m_material_balance_slider->height());

            this->m_piece_theme_selection->setCurrentIndex((-1));
        }
    });

    connect(m_board_theme_selection, &QComboBox::currentTextChanged, [this](QString text) {
        if (this->m_board_theme_selection->currentIndex() != -1) {
            BoardImage::load(text.toStdString());
            this->m_board_scene->reload();
            this->m_board_theme_selection->setCurrentIndex((-1));
        }
    });

    // Create a chart view with the chart
    m_chart_view->setRenderHint(QPainter::Antialiasing);
    m_chart_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    left_layout->addWidget(m_piece_theme_selection);
    left_layout->addWidget(m_board_theme_selection);
    left_layout->addWidget(m_results_table);
    left_layout->addWidget(m_chart_view);
    main_layout->addLayout(left_layout);

    set_label_piece_pixmap(m_clock_piece_white, libataxx::Piece::White, m_clock_white->height());
    set_label_piece_pixmap(m_clock_piece_black, libataxx::Piece::Black, m_clock_black->height());

    m_turn_radio_white->setEnabled(false);
    m_turn_radio_black->setEnabled(false);
    m_turn_radio_white->setLayoutDirection(Qt::RightToLeft);
    m_turn_radio_white->setMaximumWidth(m_clock_black->height());
    m_turn_radio_black->setMaximumWidth(m_clock_black->height());

    clock_layout->addWidget(m_turn_radio_black);
    clock_layout->addWidget(m_clock_piece_black);
    clock_layout->addWidget(m_engine_name_black);
    clock_layout->addWidget(m_clock_black);
    clock_layout->addWidget(m_clock_white);
    clock_layout->addWidget(m_engine_name_white);
    clock_layout->addWidget(m_clock_piece_white);
    clock_layout->addWidget(m_turn_radio_white);

    middle_layout->addLayout(clock_layout);

    m_clock_white->set_time(QTime(23, 59, 59));
    m_clock_black->set_time(QTime(23, 59, 59));

    m_board_view->setEnabled(true);
    middle_layout->addWidget(m_board_view);

    set_label_piece_pixmap(m_material_balance_piece_white, libataxx::Piece::White, m_material_balance_slider->height());
    set_label_piece_pixmap(m_material_balance_piece_black, libataxx::Piece::Black, m_material_balance_slider->height());
    material_balance_layout->addWidget(m_material_balance_piece_black);
    material_balance_layout->addWidget(m_material_balance_slider);
    material_balance_layout->addWidget(m_material_balance_piece_white);

    middle_layout->addLayout(material_balance_layout);

    main_layout->addLayout(middle_layout);

    // Create right vertical layout for text field
    m_pgn_text_field->setReadOnly(true);
    m_material_label->setFixedHeight(m_material_balance_piece_black->height());
    m_material_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(m_pgn_text_field);
    right_layout->addWidget(m_material_label);

    // Add both layouts to the main layout
    main_layout->addLayout(right_layout);

    // Set layout to central widget
    central_widget->setLayout(main_layout);

    main_layout->setStretch(0, 4);
    main_layout->setStretch(1, 6);
    main_layout->setStretch(2, 1);

    m_board_scene->set_board(libataxx::Position("x5o/7/2-1-2/7/2-1-2/7/o5x x 0 1"));

    m_engine_name_white->setText("hehelai");
    m_engine_name_white->setAlignment(Qt::AlignCenter);
    m_engine_name_black->setText("ao8sjdoiasjdoi");
    m_engine_name_black->setAlignment(Qt::AlignCenter);

    m_tournament_worker = new TournamentWorker(settings_path, seconds_between_games, milliseconds_between_moves);

    m_tournament_worker->moveToThread(&m_worker_thread);

    connect(&m_worker_thread, &QThread::finished, m_tournament_worker, &QObject::deleteLater);

    connect(
        m_tournament_worker,
        &TournamentWorker::new_game,
        this,
        [&](const std::string &fen,
            const std::string &name1,
            const std::string &name2,
            const int wtime,
            const int btime) {
            std::cout << name1 << std::endl;
            std::cout << name2 << std::endl;
            m_engine_name_black->setText(name1.c_str());
            m_engine_name_white->setText(name2.c_str());
            m_board_scene->set_board(libataxx::Position(fen));
            m_clock_white->set_time(QTime(0, 0).addMSecs(wtime));
            m_clock_black->set_time(QTime(0, 0).addMSecs(btime));
            m_scores.clear();
            m_pgn_text_field->setText("");
            const int material = m_board_scene->board().get_score();
            m_material_balance_slider->setSliderPosition(-material);
            m_material_label->setText(("Material: " + std::to_string(material)).c_str());
        },
        Qt::QueuedConnection);

    connect(
        m_tournament_worker,
        &TournamentWorker::game_finished,
        this,
        [&](const libataxx::Result &result) {
            m_board_scene->on_game_finished(result);

            m_clock_white->stop_clock();
            m_clock_black->stop_clock();
        },
        Qt::QueuedConnection);

    connect(
        m_tournament_worker,
        &TournamentWorker::results_update,
        this,
        [&](const Results &results) {
            fill_table(m_results_table, results);
        },
        Qt::QueuedConnection);

    connect(
        m_tournament_worker,
        &TournamentWorker::new_move,
        this,
        [&](const libataxx::Move &move, const SearchSettings &tc, int cp_score) {
            this->m_board_scene->on_new_move(move);

            const auto board_after_move = m_board_scene->board();

            m_clock_black->set_time(QTime(0, 0).addMSecs(tc.btime));
            m_clock_white->set_time(QTime(0, 0).addMSecs(tc.wtime));

            size_t black_mod_rest = m_scores.size() % 2;
            if (board_after_move.get_turn() == libataxx::Side::Black) {
                cp_score = -cp_score;
                black_mod_rest = 1 - black_mod_rest;
            }
            m_scores.push_back(std::clamp(static_cast<double>(cp_score) / 100.0, -9.0, 9.0));

            auto *m_chart = new QChart();
            m_chart->legend()->hide();

            auto *m_black_score_series = new QLineSeries;
            auto *m_white_score_series = new QLineSeries;
            m_black_score_series->setPen(QPen(Qt::red, 2.5));
            m_white_score_series->setPen(QPen(Qt::blue, 2.5));
            double max_score = 3.0;
            for (size_t i = 0; i < m_scores.size(); ++i) {
                auto *series = (i % 2) == black_mod_rest ? m_black_score_series : m_white_score_series;
                series->append(i, m_scores.at(i));
                max_score = std::max(std::abs(m_scores.at(i)), max_score);
            }
            m_chart->addSeries(m_black_score_series);
            m_chart->addSeries(m_white_score_series);

            auto axis_y = new QValueAxis();
            axis_y->setTitleText("Engine evaluation");
            axis_y->setLabelFormat("%d");
            axis_y->setTickCount(7);  // Assuming symmetric range
            max_score += 0.1;
            axis_y->setRange(-max_score, max_score);
            m_chart->addAxis(axis_y, Qt::AlignLeft);
            m_black_score_series->attachAxis(axis_y);
            m_white_score_series->attachAxis(axis_y);

            auto axis_x = new QValueAxis();
            axis_x->setLabelFormat("%d");
            axis_x->setTickCount(3);  // Assuming symmetric range
            axis_x->setRange(0, m_scores.size());
            m_chart->addAxis(axis_x, Qt::AlignBottom);
            m_black_score_series->attachAxis(axis_x);
            m_white_score_series->attachAxis(axis_x);
            m_chart_view->setChart(m_chart);

            if (board_after_move.get_turn() == libataxx::Side::Black) {
                m_clock_white->stop_clock();
                m_clock_black->start_clock();

                m_turn_radio_white->setChecked(false);
                m_turn_radio_black->setChecked(true);

            } else {
                m_clock_black->stop_clock();
                m_clock_white->start_clock();

                m_turn_radio_black->setChecked(false);
                m_turn_radio_white->setChecked(true);
            }

            const int material = board_after_move.get_score();
            m_material_balance_slider->setSliderPosition(-material);
            m_material_label->setText(("Material: " + std::to_string(material)).c_str());

            m_pgn_text_field->append(static_cast<std::string>(move).c_str());
        },
        Qt::QueuedConnection);

    m_worker_thread.start();
    QMetaObject::invokeMethod(m_tournament_worker, "start", Qt::QueuedConnection);
}

void TournamentWorker::start() {
    const auto settings = parse::settings(m_settings_path);
    const auto openings = parse::openings(settings.openings_path, settings.shuffle);

    if (settings.concurrency != 1) {
        throw std::runtime_error("Only concurrency = 1 is supported");
    }

    std::optional<int64_t> current_cp_score = std::nullopt;

    const auto callbacks = Callbacks{
        .on_engine_start =
            [](const std::string &) {
            },
        .on_game_started =
            [&](const size_t /*id*/, const std::string &fen, const std::string &name1, const std::string &name2) {
                emit new_game(fen, name1, name2, settings.tc.wtime, settings.tc.btime);
            },
        .on_game_finished =
            [&](const size_t /*id*/, const libataxx::Result result, const std::string &, const std::string &) {
                emit game_finished(result);
                std::this_thread::sleep_for(std::chrono::seconds(m_seconds_between_games));
            },
        .on_results_update =
            [&](const Results &results) {
                emit results_update(results);
            },
        .on_info_send =
            [](const std::string &s) {
                std::cout << "--> " << s << std::endl;
            },
        .on_info_recv =
            [&](const std::string &s) {
                std::cout << "<-- " << s << std::endl;
                std::istringstream iss(s);
                std::string word;

                // Read words into vector
                while (iss >> word) {
                    if (word == "score") {
                        try {
                            iss >> word;
                            int64_t multiplier = 1;
                            if (word == "mate") {
                                iss >> word;
                                multiplier = 1000000;
                            }
                            current_cp_score = std::stoll(word) * multiplier;
                            return;
                        } catch (...) {
                            iss >> word;
                            current_cp_score = std::stoll(word);
                            return;
                        }
                    }
                }
            },
        .on_move =
            [&](const libataxx::Move &move, const int /* move_time_ms */, const SearchSettings &tc) {
                emit new_move(move, tc, current_cp_score.value_or(0));
                current_cp_score = std::nullopt;
                std::this_thread::sleep_for(std::chrono::milliseconds(m_milliseconds_between_moves));
            },
    };

    [[maybe_unused]] const auto results = run(settings, openings, callbacks);
}

MainWindow::~MainWindow() {
    std::cerr << "This program must be stopped using ctrl+c" << std::endl;
    std::abort();
}