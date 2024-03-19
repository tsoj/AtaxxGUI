#include "mainwindow.hpp"
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <../core/engine/create.hpp>
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
#include "engine/settings.hpp"
#include "guisettings.hpp"
#include "humanengine.hpp"
#include "texteditor.hpp"

// #include "boardview/boardscene.hpp"

namespace {

void setLabelPiecePixmap(QLabel *label, libataxx::Piece piece, int size) {
    label->setPixmap(PieceImages::piece_images(piece).scaledToHeight(size, Qt::SmoothTransformation));
    label->setMaximumHeight(size);
    label->setMaximumWidth(size);
    label->setScaledContents(true);
    label->show();
}

const std::vector<std::string> startPositions = {"x5o/7/2-1-2/7/2-1-2/7/o5x x 0 1",
                                                 "x5o/7/7/7/7/7/o5x x 0 1",
                                                 "x5o/1-3-1/2-1-2/7/2-1-2/1-3-1/o5x x 0 1",
                                                 "x5o/7/3-3/2-1-2/3-3/7/o5x x 0 1",
                                                 "x5o/3-3/3-3/1--1--1/3-3/3-3/o5x x 0 1",
                                                 "x2-2o/7/7/-5-/7/7/o2-2x x 0 1",
                                                 "x2-2o/3-3/3-3/---1---/3-3/3-3/o2-2x x 0 1",
                                                 "x5o/2-1-2/1-3-1/7/1-3-1/2-1-2/o5x x 0 1",
                                                 "x1-1-1o/7/-5-/7/-5-/7/o1-1-1x x 0 1",
                                                 "x2-2o/3-3/2---2/7/2---2/3-3/o2-2x x 0 1",
                                                 "x2-2o/3-3/7/--3--/7/3-3/o2-2x x 0 1",
                                                 "x1-1-1o/2-1-2/2-1-2/7/2-1-2/2-1-2/o1-1-1x x 0 1",
                                                 "x5o/7/2-1-2/3-3/2-1-2/7/o5x x 0 1",
                                                 "x5o/7/3-3/2---2/3-3/7/o5x x 0 1"};

const std::string humanEngineName = "Human player";
const std::string exampleEngineName = "Example engine";

const std::string defaultSettingsString =
    "{"
    "    \"timecontrol\": {"
    "        \"time\": 15000,"
    "        \"inc\": 100"
    "    },"
    "    \"options\": {"
    "        \"debug\": \"false\","
    "        \"threads\": \"1\","
    "        \"hash\": \"128\","
    "        \"ownbook\": \"false\""
    "    },"
    "    \"engines\": ["
    "        {"
    "            \"name\": \"" +
    exampleEngineName +
    "\","
    "            \"path\": \"/path/to/example/engine\","
    "            \"protocol\": \"UAI\","
    "            \"options\": {"
    "                \"ownbook\": \"true\""
    "            }"
    "        }"
    "    ]"
    "}";
}  // namespace

void MainWindow::load_settings() {
    if (!std::filesystem::exists(m_settings_file_path)) {
        std::ofstream f(m_settings_file_path, std::fstream::out | std::fstream::app);
        if (!f.is_open()) {
            throw std::runtime_error("Couldn't open settings file");
        }
        f << nlohmann::json::parse(defaultSettingsString).dump(4);
        f.close();
    }

    nlohmann::json j;
    std::ifstream input_file(m_settings_file_path);
    if (input_file.is_open()) {
        input_file >> j;
        input_file.close();
    } else {
        throw std::runtime_error("Could not open the file for reading");
    }

    // Pretty print the JSON object to a string
    std::string pretty_json = j.dump(4);

    // Write the pretty JSON string back to the file
    std::ofstream output_file(m_settings_file_path);
    if (output_file.is_open()) {
        output_file << pretty_json;
        output_file.close();
    } else {
        throw std::runtime_error("Could not open the file for writing");
    }

    std::cout << "Using settings file: " << m_settings_file_path << std::endl;
    const auto settings = GuiSettings(m_settings_file_path);
    m_engines.clear();
    for (const auto &engine : settings.engines) {
        m_engines[engine.name] = engine;
        if (engine.name == exampleEngineName) {
            m_engines[engine.name].builtin = "mostcaptures";
        }
    }
    if (m_engines.contains(humanEngineName)) {
        throw std::runtime_error("Settings must not contain an engine called \"" + humanEngineName + "\"");
    }

    for (auto engineSelection : std::vector{m_engine_selection1, m_engine_selection2}) {
        engineSelection->clear();
        engineSelection->addItem(humanEngineName.c_str());
        for (const auto &[engineName, engine] : m_engines) {
            engineSelection->addItem(engineName.c_str());
        }
    }

    m_time_spin_box->setTime(QTime(0, 0).addMSecs(std::max(settings.tc.wtime, settings.tc.btime)));
    m_inc_spin_box->setTime(QTime(0, 0, 0).addMSecs(std::max(settings.tc.winc, settings.tc.binc)));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_settings_file_path(QCoreApplication::applicationDirPath().toStdString() + "/settings.json") {
    QGridLayout *tcLayout = new QGridLayout();
    QLabel *timeLabel = new QLabel("Time (HH:mm:ss): ", this);
    QLabel *incLabel = new QLabel("Increment (mm:ss): ", this);
    m_time_spin_box = new QTimeEdit(this);
    m_inc_spin_box = new QTimeEdit(this);
    m_board_scene = new BoardScene(this);
    m_board_view = new BoardView(m_board_scene, this);
    m_human_engine = std::make_shared<HumanEngine>();
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout();
    m_toggle_game_button = new QPushButton("Start Game", this);
    m_engine_selection1 = new QComboBox(this);
    m_engine_selection2 = new QComboBox(this);
    m_clock_piece_white = new QLabel(this);
    m_clock_piece_black = new QLabel(this);
    QPushButton *editSettingsButton = new QPushButton("Edit settings.json", this);
    QGridLayout *engineSelectionLayout = new QGridLayout();
    QVBoxLayout *middleLayout = new QVBoxLayout();
    QHBoxLayout *clockLayout = new QHBoxLayout();
    m_selection_piece_white = new QLabel(this);
    m_selection_piece_black = new QLabel(this);
    m_turn_radio_white = new QRadioButton;
    m_turn_radio_black = new QRadioButton;
    m_clock_white = new CountdownTimer(this);
    m_clock_black = new CountdownTimer(this);
    QHBoxLayout *setFenLayout = new QHBoxLayout();
    QLabel *fenLabel = new QLabel("FEN: ", this);
    m_fen_text_field = new QLineEdit(this);
    m_fen_set_fen = new QPushButton("Set", this);
    m_start_pos_selection = new QComboBox(this);
    QVBoxLayout *rightLayout = new QVBoxLayout();
    m_pgn_text_field = new QTextEdit(this);
    m_human_infinite_time_checkbox = new QCheckBox("Infinite time for human player", this);
    m_piece_theme_selection = new QComboBox(this);
    m_board_theme_selection = new QComboBox(this);

    load_settings();
    PieceImages::load();

    connect(m_board_scene, &BoardScene::human_move, m_human_engine.get(), &HumanEngine::on_human_move);

    connect(m_human_engine.get(), &HumanEngine::need_human_move_input, m_board_scene, &BoardScene::accept_move_input);

    // Create central widget
    setCentralWidget(centralWidget);

    connect(editSettingsButton, &QPushButton::clicked, this, &MainWindow::edit_settings);

    tcLayout->addWidget(timeLabel, 0, 0);
    tcLayout->addWidget(m_time_spin_box, 0, 1);
    tcLayout->addWidget(incLabel, 1, 0);
    tcLayout->addWidget(m_inc_spin_box, 1, 1);

    m_time_spin_box->setDisplayFormat("HH:mm:ss");
    m_time_spin_box->setTimeRange(QTime(0, 0, 0), QTime(23, 59, 59));

    m_inc_spin_box->setDisplayFormat("mm:ss");
    m_inc_spin_box->setTimeRange(QTime(0, 0, 0), QTime(59, 59));
    leftLayout->addLayout(tcLayout);
    leftLayout->addWidget(m_human_infinite_time_checkbox);

    m_human_infinite_time_checkbox->setCheckState(Qt::CheckState::Checked);

    setLabelPiecePixmap(m_selection_piece_white, libataxx::Piece::White, m_engine_selection2->height());
    setLabelPiecePixmap(m_selection_piece_black, libataxx::Piece::Black, m_engine_selection1->height());

    m_piece_theme_selection->setPlaceholderText("Select piece theme");
    m_board_theme_selection->setPlaceholderText("Select board theme");

    m_piece_theme_selection->addItems(QDir(PieceImages::path_to_themes()).entryList(QDir::Dirs | QDir::NoDotAndDotDot));
    m_board_theme_selection->addItems(QDir(BoardImage::path_to_themes()).entryList(QDir::Dirs | QDir::NoDotAndDotDot));

    connect(m_piece_theme_selection, &QComboBox::currentTextChanged, [this](QString text) {
        if (this->m_piece_theme_selection->currentIndex() != -1) {
            PieceImages::load(text.toStdString());
            this->m_board_scene->reload();

            setLabelPiecePixmap(this->m_clock_piece_white, libataxx::Piece::White, this->m_clock_white->height());
            setLabelPiecePixmap(this->m_clock_piece_black, libataxx::Piece::Black, this->m_clock_black->height());

            setLabelPiecePixmap(
                this->m_selection_piece_white, libataxx::Piece::White, this->m_engine_selection2->height());
            setLabelPiecePixmap(
                this->m_selection_piece_black, libataxx::Piece::Black, this->m_engine_selection1->height());

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

    engineSelectionLayout->addWidget(m_selection_piece_black, 0, 0);
    engineSelectionLayout->addWidget(m_selection_piece_white, 1, 0);
    engineSelectionLayout->addWidget(m_engine_selection1, 0, 1);
    engineSelectionLayout->addWidget(m_engine_selection2, 1, 1);

    leftLayout->addLayout(engineSelectionLayout);
    leftLayout->addWidget(m_toggle_game_button);
    leftLayout->addWidget(editSettingsButton);
    leftLayout->addWidget(m_piece_theme_selection);
    leftLayout->addWidget(m_board_theme_selection);
    leftLayout->addStretch(1);
    mainLayout->addLayout(leftLayout);

    connect(m_toggle_game_button, &QPushButton::clicked, [this]() {
        if (this->m_toggle_game_button->text() == "Start Game") {
            start_game();
        } else {
            stop_game();
        }
    });

    setLabelPiecePixmap(m_clock_piece_white, libataxx::Piece::White, m_clock_white->height());
    setLabelPiecePixmap(m_clock_piece_black, libataxx::Piece::Black, m_clock_black->height());

    m_turn_radio_white->setEnabled(false);
    m_turn_radio_black->setEnabled(false);
    m_turn_radio_white->setLayoutDirection(Qt::RightToLeft);
    m_turn_radio_white->setMaximumWidth(m_clock_black->height());
    m_turn_radio_black->setMaximumWidth(m_clock_black->height());

    clockLayout->addWidget(m_turn_radio_white);
    clockLayout->addWidget(m_clock_piece_white);
    clockLayout->addWidget(m_clock_white);
    clockLayout->addWidget(m_clock_black);
    clockLayout->addWidget(m_clock_piece_black);
    clockLayout->addWidget(m_turn_radio_black);
    middleLayout->addLayout(clockLayout);

    m_clock_white->set_time(QTime(23, 59, 59));
    m_clock_black->set_time(QTime(23, 59, 59));

    m_board_view->setEnabled(true);
    middleLayout->addWidget(m_board_view);

    setFenLayout->addWidget(fenLabel);

    setFenLayout->addWidget(m_fen_text_field);

    connect(m_board_scene, &BoardScene::new_fen, m_fen_text_field, &QLineEdit::setText);
    connect(m_fen_text_field, &QLineEdit::editingFinished, [this]() {
        if (m_game_worker == nullptr) {
            m_board_scene->set_board(libataxx::Position(m_fen_text_field->text().toStdString()));
        } else {
            this->m_fen_text_field->setText(QString::fromStdString(m_board_scene->board().get_fen()));
        }
    });

    setFenLayout->addWidget(m_fen_set_fen);
    middleLayout->addLayout(setFenLayout);

    m_start_pos_selection->setPlaceholderText("Select start position");
    for (size_t i = 0; i < startPositions.size(); ++i) {
        m_start_pos_selection->addItem((std::to_string(i + 1) + ". " + startPositions.at(i)).c_str());
    }
    connect(m_start_pos_selection, &QComboBox::currentTextChanged, [this](QString text) {
        if (this->m_start_pos_selection->currentIndex() != -1 && m_game_worker == nullptr) {
            auto startpos = text.toStdString();
            while (true) {
                auto c = startpos.front();
                startpos.erase(0, 1);
                if (c == '.') break;
            }
            this->m_board_scene->set_board(libataxx::Position(startpos));
            this->m_start_pos_selection->setCurrentIndex((-1));
        }
    });

    m_start_pos_selection->setCurrentIndex((-1));
    middleLayout->addWidget(m_start_pos_selection);

    mainLayout->addLayout(middleLayout);

    // Create right vertical layout for text field
    m_pgn_text_field->setReadOnly(true);
    rightLayout->addWidget(m_pgn_text_field);

    // Add both layouts to the main layout
    mainLayout->addLayout(rightLayout);

    // Set layout to central widget
    centralWidget->setLayout(mainLayout);

    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 3);
    mainLayout->setStretch(2, 1);

    m_board_scene->set_board(libataxx::Position(startPositions.front()));
}

void MainWindow::edit_settings() {
    TextEditor *editor = new TextEditor(m_settings_file_path);
    connect(editor, &TextEditor::changed_settings, this, &MainWindow::load_settings);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->resize(size());
    editor->show();
}

void MainWindow::start_game() {
    const int time = m_time_spin_box->time().msecsSinceStartOfDay();
    const int inc = m_inc_spin_box->time().msecsSinceStartOfDay();
    const auto tc = SearchSettings::as_time(time, time, inc, inc);

    const auto create_engine = [this, tc](std::string engineName) {
        std::shared_ptr<Engine> engine{};
        EngineSettings engine_settings{};

        if (engineName == humanEngineName) {
            engine_settings.name = humanEngineName;
            engine_settings.tc = this->m_human_infinite_time_checkbox->isChecked() ? SearchSettings::as_depth(1) : tc;

            engine = this->m_human_engine;
        } else {
            engine_settings = this->m_engines.at(engineName);
            engine_settings.tc = tc;

            engine = make_engine(engine_settings);
        }
        return std::pair{engine, engine_settings};
    };

    std::shared_ptr<Engine> engine1{nullptr}, engine2{nullptr};
    EngineSettings engineSetting1, engineSetting2;
    try {
        std::tie(engine1, engineSetting1) = create_engine(this->m_engine_selection1->currentText().toStdString());
        std::tie(engine2, engineSetting2) = create_engine(this->m_engine_selection2->currentText().toStdString());
    } catch (const std::exception &e) {
        std::cout << "Failed to create engine: " << e.what() << std::endl;
        QMessageBox::warning(this, "Failed to create engine", e.what());
        return;
    }

    engineSetting1.id = 1;
    engineSetting2.id = 2;

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
            .fen = this->m_board_scene->board().get_fen(), .engine1 = engineSetting1, .engine2 = engineSetting2},
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
        [this](SearchSettings tc1, SearchSettings tc2, libataxx::Side sideToMove) {
            if (tc2.type == SearchSettings::Type::Time) {
                m_clock_white->set_time(QTime(0, 0).addMSecs(tc2.wtime));
            } else {
                m_clock_white->set_time(QTime(23, 59, 59));
            }

            if (tc1.type == SearchSettings::Type::Time) {
                m_clock_black->set_time(QTime(0, 0).addMSecs(tc1.wtime));
            } else {
                m_clock_black->set_time(QTime(23, 59, 59));
            }

            if (sideToMove == libataxx::Side::Black) {
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

MainWindow::~MainWindow() {
    stop_game();
}