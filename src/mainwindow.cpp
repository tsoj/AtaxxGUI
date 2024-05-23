#include "mainwindow.hpp"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
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
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <algorithm>
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
// #include "engine/settings.hpp"
// #include "guisettings.hpp"
// #include "humanengine.hpp"
// #include "texteditor.hpp"

// #include "boardview/boardscene.hpp"

namespace {

void set_label_piece_pixmap(QLabel *label, libataxx::Piece piece, int size) {
    label->setPixmap(PieceImages::piece_images(piece).scaledToHeight(size, Qt::SmoothTransformation));
    label->setMaximumHeight(size);
    label->setMaximumWidth(size);
    label->setScaledContents(true);
    label->show();
}

auto get_recv_send_callbacks(const std::string &engine_name) {
    const auto write_line = [engine_name](const std::string &line, const std::string &prefix) {
        std::ofstream file(QCoreApplication::applicationDirPath().toStdString() + "/" + engine_name + ".log",
                           std::ios::app);
        file << prefix << line << std::endl;
        file.close();
    };
    return std::pair{[write_line](const std::string &msg) {
                         write_line(msg, "--> ");
                     },
                     [write_line](const std::string &msg) {
                         write_line(msg, "<-- ");
                     }};
}
}  // namespace

MainWindow::MainWindow(const std::string &settingsFileName, QWidget *parent) : QMainWindow(parent) {
    m_board_scene = new BoardScene(this);
    m_board_view = new BoardView(m_board_scene, this);
    QWidget *central_widget = new QWidget(this);
    QHBoxLayout *main_layout = new QHBoxLayout(central_widget);
    QVBoxLayout *left_layout = new QVBoxLayout();
    m_clock_piece_white = new QLabel(this);
    m_clock_piece_black = new QLabel(this);
    QVBoxLayout *middle_layout = new QVBoxLayout();
    QHBoxLayout *clock_layout = new QHBoxLayout();
    m_material_balance_piece_white = new QLabel(this);
    m_material_balance_piece_black = new QLabel(this);
    m_turn_radio_white = new QRadioButton;
    m_turn_radio_black = new QRadioButton;
    m_clock_white = new CountdownTimer(this);
    m_clock_black = new CountdownTimer(this);
    QHBoxLayout *material_balance_layout = new QHBoxLayout();
    QVBoxLayout *right_layout = new QVBoxLayout();
    m_pgn_text_field = new QTextEdit(this);
    m_piece_theme_selection = new QComboBox(this);
    m_board_theme_selection = new QComboBox(this);
    m_material_balance_slider = new MaterialSlider(this);

    auto *engine_name_black = new QLabel(this);
    auto *engine_name_white = new QLabel(this);

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

    left_layout->addWidget(m_piece_theme_selection);
    left_layout->addWidget(m_board_theme_selection);
    left_layout->addStretch(1);
    main_layout->addLayout(left_layout);

    set_label_piece_pixmap(m_clock_piece_white, libataxx::Piece::White, m_clock_white->height());
    set_label_piece_pixmap(m_clock_piece_black, libataxx::Piece::Black, m_clock_black->height());

    m_turn_radio_white->setEnabled(false);
    m_turn_radio_black->setEnabled(false);
    m_turn_radio_white->setLayoutDirection(Qt::RightToLeft);
    m_turn_radio_white->setMaximumWidth(m_clock_black->height());
    m_turn_radio_black->setMaximumWidth(m_clock_black->height());

    clock_layout->addWidget(m_turn_radio_white);
    clock_layout->addWidget(m_clock_piece_white);
    clock_layout->addWidget(engine_name_white);
    clock_layout->addWidget(m_clock_white);
    clock_layout->addWidget(m_clock_black);
    clock_layout->addWidget(engine_name_black);
    clock_layout->addWidget(m_clock_piece_black);
    clock_layout->addWidget(m_turn_radio_black);
    middle_layout->addLayout(clock_layout);

    m_clock_white->set_time(QTime(23, 59, 59));
    m_clock_black->set_time(QTime(23, 59, 59));

    m_board_view->setEnabled(true);
    middle_layout->addWidget(m_board_view);

    set_label_piece_pixmap(m_material_balance_piece_white, libataxx::Piece::White, m_material_balance_slider->height());
    set_label_piece_pixmap(m_material_balance_piece_black, libataxx::Piece::Black, m_material_balance_slider->height());
    material_balance_layout->addWidget(m_material_balance_piece_white);
    material_balance_layout->addWidget(m_material_balance_slider);
    material_balance_layout->addWidget(m_material_balance_piece_black);

    middle_layout->addLayout(material_balance_layout);

    main_layout->addLayout(middle_layout);

    // Create right vertical layout for text field
    // TODO add move list
    m_pgn_text_field->setReadOnly(true);
    right_layout->addWidget(m_pgn_text_field);

    // Add both layouts to the main layout
    main_layout->addLayout(right_layout);

    // Set layout to central widget
    central_widget->setLayout(main_layout);

    main_layout->setStretch(0, 1);
    main_layout->setStretch(1, 3);
    main_layout->setStretch(2, 1);

    m_board_scene->set_board(libataxx::Position("x5o/7/2-1-2/7/2-1-2/7/o5x x 0 1"));

    engine_name_white->setText("hehelai");
    engine_name_white->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    engine_name_black->setText("ao8sjdoiasjdoi");
    engine_name_black->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    const auto settings = parse::settings(settingsFileName);
    const auto openings = parse::openings(settings.openings_path, settings.shuffle);

    /*const auto callbacks = Callbacks{
        .on_engine_start =
            [](const std::string &) {
            },
        .on_game_started =
            [&settings, &game_tab, &screen](
                const int, const std::string &fen, const std::string &name1, const std::string &name2) {
                game_tab.new_game();
                game_tab.set_title(name1 + " vs " + name2);
                game_tab.set_position(fen);
                game_tab.set_clock(settings.tc);
                screen.PostEvent(Event::Custom);
            },
        .on_game_finished =
            [&settings](const int, const std::string &, const std::string &) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
            },
        .on_results_update =
            [&settings, &game_tab](const Results &results) {
                game_tab.update_results(results);
            },
        .on_info_send =
            [](const std::string &) {
            },
        .on_info_recv =
            [](const std::string &) {
            },
        .on_move =
            [&game_tab, &screen](const libataxx::Move &move, const int ms) {
                game_tab.makemove(move, ms);
                screen.PostEvent(Event::Custom);
            },
    };*/
}
/*
void MainWindow::start_game() {
    const int time = m_time_spin_box->time().msecsSinceStartOfDay();
    const int inc = m_inc_spin_box->time().msecsSinceStartOfDay();
    const auto tc = SearchSettings::as_time(time, time, inc, inc);

    const auto create_engine = [this, tc](std::string engine_name) {
        std::shared_ptr<Engine> engine{};
        EngineSettings engine_settings{};

        if (engine_name == human_engine_name) {
            engine_settings.name = human_engine_name;
            engine_settings.tc = this->m_human_infinite_time_checkbox->isChecked() ? SearchSettings::as_depth(1) : tc;

            engine = this->m_human_engine;
        } else {
            engine_settings = this->m_engines.at(engine_name);
            engine_settings.tc = tc;

            const auto [send, recv] = get_recv_send_callbacks(engine_name);

            engine = make_engine(engine_settings, send, recv);
        }
        return std::pair{engine, engine_settings};
    };

    std::shared_ptr<Engine> engine1{nullptr}, engine2{nullptr};
    EngineSettings engine_setting1, engine_setting2;
    try {
        std::tie(engine1, engine_setting1) = create_engine(this->m_engine_selection1->currentText().toStdString());
        std::tie(engine2, engine_setting2) = create_engine(this->m_engine_selection2->currentText().toStdString());
    } catch (const std::exception &e) {
        std::cout << "Failed to create engine: " << e.what() << std::endl;
        QMessageBox::warning(this, "Failed to create engine", e.what());
        return;
    }

    engine_setting1.id = 1;
    engine_setting2.id = 2;

    this->m_engine_selection1->setEnabled(false);
    this->m_engine_selection2->setEnabled(false);
    m_pgn_text_field->setText("");

    this->m_toggle_game_button->setText("Stop Game");

    m_fen_text_field->setReadOnly(true);
    m_fen_set_fen->setEnabled(false);
    m_start_pos_selection->setEnabled(false);

    Q_ASSERT(m_game_worker == nullptr);
    m_game_worker = new GameWorker(
        AdjudicationSettings{},
        GameSettings{
            .fen = this->m_board_scene->board().get_fen(), .engine1 = engine_setting1, .engine2 = engine_setting2},
        engine1,
        engine2);

    m_game_worker->moveToThread(&m_worker_thread);

    connect(&m_worker_thread, &QThread::finished, m_game_worker, &QObject::deleteLater);

    connect(m_game_worker,
            &GameWorker::finished_game,
            this->m_board_scene,
            &BoardScene::on_game_finished,
            Qt::QueuedConnection);

    connect(
        m_game_worker,
        &GameWorker::finished_game,
        this,
        [this]() {
            m_clock_white->stop_clock();
            m_clock_black->stop_clock();
        },
        Qt::QueuedConnection);

    connect(
        m_game_worker,
        &GameWorker::finished_game,
        this,
        [this](GameThingy info) {
            m_pgn_text_field->setText(QString::fromStdString(get_pgn(
                PGNSettings{

                },
                this->m_engine_selection1->currentText().toStdString(),
                this->m_engine_selection2->currentText().toStdString(),
                info)));
            this->stop_game();
        },
        Qt::QueuedConnection);

    connect(
        m_game_worker,
        &GameWorker::new_move,
        this,
        [this](GameThingy info) {
            this->m_board_scene->on_new_move(info.history.back().move);

            m_pgn_text_field->setText(QString::fromStdString(get_pgn(
                PGNSettings{

                },
                this->m_engine_selection1->currentText().toStdString(),
                this->m_engine_selection2->currentText().toStdString(),
                info)));
        },
        Qt::QueuedConnection);

    // Black is engine1 (tc1)
    connect(
        m_game_worker,
        &GameWorker::update_time_control,
        this,
        [this](SearchSettings tc1, SearchSettings tc2, libataxx::Side side_to_move) {
            if (tc2.type == SearchSettings::Type::Time) {
                m_clock_white->set_time(QTime(0, 0).addMSecs(tc2.wtime));
            } else {
                m_clock_white->set_time(QTime(23, 59, 59));
            }

            if (tc1.type == SearchSettings::Type::Time) {
                m_clock_black->set_time(QTime(0, 0).addMSecs(tc1.btime));
            } else {
                m_clock_black->set_time(QTime(23, 59, 59));
            }

            if (side_to_move == libataxx::Side::Black) {
                m_clock_white->stop_clock();
                if (tc1.type == SearchSettings::Type::Time) {
                    m_clock_black->start_clock();
                }

                m_turn_radio_white->setChecked(false);
                m_turn_radio_black->setChecked(true);

            } else {
                m_clock_black->stop_clock();
                if (tc2.type == SearchSettings::Type::Time) {
                    m_clock_white->start_clock();
                }

                m_turn_radio_black->setChecked(false);
                m_turn_radio_white->setChecked(true);
            }
        },
        Qt::QueuedConnection);

    m_worker_thread.start();
    QMetaObject::invokeMethod(m_game_worker, "start_game", Qt::QueuedConnection);
}

void MainWindow::stop_game() {
    if (m_game_worker != nullptr) {
        m_game_worker->stopGame();

        m_worker_thread.quit();
        m_worker_thread.wait();
        m_game_worker = nullptr;
    }

    m_engine_selection1->setEnabled(true);
    m_engine_selection2->setEnabled(true);
    m_toggle_game_button->setText("Start Game");
    m_fen_text_field->setReadOnly(false);
    m_fen_set_fen->setEnabled(true);
    m_start_pos_selection->setEnabled(true);
}
*/
MainWindow::~MainWindow() {
    // stop_game();
}