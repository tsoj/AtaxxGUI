#include "mainwindow.hpp"
#include "boardview/boardscene.hpp"
#include "boardview/boardview.hpp"
#include "boardview/images.hpp"
#include "engine/settings.hpp"
#include "guisettings.hpp"
#include "humanengine.hpp"
#include "texteditor.hpp"
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
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libataxx/move.hpp>
#include <nlohmann/json.hpp>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <stdexcept>

// #include "boardview/boardscene.hpp"

namespace {

void set_label_piece_pixmap(QLabel *label, libataxx::Piece piece, int size) {
  label->setPixmap(PieceImages::piece_images(piece).scaledToHeight(
      size, Qt::SmoothTransformation));
  label->setMaximumHeight(size);
  label->setMaximumWidth(size);
  label->setScaledContents(true);
  label->show();
}

auto get_recv_send_callbacks(const std::string &engine_name) {
  const auto write_line = [engine_name](const std::string &line,
                                        const std::string &prefix) {
    std::ofstream file(QCoreApplication::applicationDirPath().toStdString() +
                           "/" + engine_name + ".log",
                       std::ios::app);
    file << prefix << line << std::endl;
    file.close();
  };
  return std::pair{
      [write_line](const std::string &msg) { write_line(msg, "--> "); },
      [write_line](const std::string &msg) { write_line(msg, "<-- "); }};
}

const std::vector<std::string> start_positions = {
    "x5o/7/2-1-2/7/2-1-2/7/o5x x 0 1",
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

const std::string human_engine_name = "Human player";
const std::string example_engine_name = "Example engine";

const std::string default_settings_string =
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
    example_engine_name +
    "\","
    "            \"path\": \"/path/to/example/engine\","
    "            \"protocol\": \"UAI\","
    "            \"options\": {"
    "                \"ownbook\": \"true\""
    "            }"
    "        }"
    "    ]"
    "}";
} // namespace

void MainWindow::load_settings() {
  if (!std::filesystem::exists(m_settings_file_path)) {
    std::ofstream f(m_settings_file_path,
                    std::fstream::out | std::fstream::app);
    if (!f.is_open()) {
      throw std::runtime_error("Couldn't open settings file");
    }
    f << nlohmann::json::parse(default_settings_string).dump(4);
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
  const auto settings = GuiSettings(m_settings_file_path.string());
  m_engines.clear();
  for (const auto &engine : settings.engines) {
    m_engines[engine.name] = engine;
    if (engine.name == example_engine_name) {
      m_engines[engine.name].builtin = "mostcaptures";
    }
  }
  if (m_engines.contains(human_engine_name)) {
    throw std::runtime_error("Settings must not contain an engine called \"" +
                             human_engine_name + "\"");
  }

  for (auto engineSelection :
       std::vector{m_engine_selection1, m_engine_selection2}) {
    engineSelection->clear();
    engineSelection->addItem(human_engine_name.c_str());
    for (const auto &[engineName, engine] : m_engines) {
      engineSelection->addItem(engineName.c_str());
    }
  }

  m_time_spin_box->setTime(
      QTime(0, 0).addMSecs(std::max(settings.tc.wtime, settings.tc.btime)));
  m_inc_spin_box->setTime(
      QTime(0, 0, 0).addMSecs(std::max(settings.tc.winc, settings.tc.binc)));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_settings_file_path(
          QCoreApplication::applicationDirPath().toStdString() +
          "/settings.json") {
  QGridLayout *tc_layout = new QGridLayout();
  QLabel *time_label = new QLabel("Time (HH:mm:ss): ", this);
  QLabel *inc_label = new QLabel("Increment (mm:ss): ", this);
  m_time_spin_box = new QTimeEdit(this);
  m_inc_spin_box = new QTimeEdit(this);
  m_board_scene = new BoardScene(this);
  m_board_view = new BoardView(m_board_scene, this);
  m_human_engine = std::make_shared<HumanEngine>();
  QWidget *central_widget = new QWidget(this);
  QHBoxLayout *main_layout = new QHBoxLayout(central_widget);
  QVBoxLayout *left_layout = new QVBoxLayout();
  m_toggle_game_button = new QPushButton("Start Game", this);
  m_engine_selection1 = new QComboBox(this);
  m_engine_selection2 = new QComboBox(this);
  m_clock_piece_white = new QLabel(this);
  m_clock_piece_black = new QLabel(this);
  QPushButton *edit_settings_button =
      new QPushButton("Edit settings.json", this);
  QGridLayout *engine_selection_layout = new QGridLayout();
  QVBoxLayout *middle_layout = new QVBoxLayout();
  QHBoxLayout *clock_layout = new QHBoxLayout();
  m_selection_piece_white = new QLabel(this);
  m_selection_piece_black = new QLabel(this);
  m_turn_radio_white = new QRadioButton;
  m_turn_radio_black = new QRadioButton;
  m_clock_white = new CountdownTimer(this);
  m_clock_black = new CountdownTimer(this);
  QHBoxLayout *set_fen_layout = new QHBoxLayout();
  QLabel *fen_label = new QLabel("FEN: ", this);
  m_fen_text_field = new QLineEdit(this);
  m_fen_set_fen = new QPushButton("Set", this);
  m_start_pos_selection = new QComboBox(this);
  QVBoxLayout *right_layout = new QVBoxLayout();
  m_pgn_text_field = new QTextEdit(this);
  m_human_infinite_time_checkbox =
      new QCheckBox("Infinite time for human player", this);
  m_piece_theme_selection = new QComboBox(this);
  m_board_theme_selection = new QComboBox(this);

  load_settings();
  PieceImages::load();

  connect(m_board_scene, &BoardScene::human_move, m_human_engine.get(),
          &HumanEngine::on_human_move);

  connect(m_human_engine.get(), &HumanEngine::need_human_move_input,
          m_board_scene, &BoardScene::accept_move_input);

  // Create central widget
  setCentralWidget(central_widget);

  connect(edit_settings_button, &QPushButton::clicked, this,
          &MainWindow::edit_settings);

  tc_layout->addWidget(time_label, 0, 0);
  tc_layout->addWidget(m_time_spin_box, 0, 1);
  tc_layout->addWidget(inc_label, 1, 0);
  tc_layout->addWidget(m_inc_spin_box, 1, 1);

  m_time_spin_box->setDisplayFormat("HH:mm:ss");
  m_time_spin_box->setTimeRange(QTime(0, 0, 0), QTime(23, 59, 59));

  m_inc_spin_box->setDisplayFormat("mm:ss");
  m_inc_spin_box->setTimeRange(QTime(0, 0, 0), QTime(59, 59));
  left_layout->addLayout(tc_layout);
  left_layout->addWidget(m_human_infinite_time_checkbox);

  m_human_infinite_time_checkbox->setCheckState(Qt::CheckState::Checked);

  set_label_piece_pixmap(m_selection_piece_white, libataxx::Piece::White,
                         m_engine_selection2->height());
  set_label_piece_pixmap(m_selection_piece_black, libataxx::Piece::Black,
                         m_engine_selection1->height());

  m_piece_theme_selection->setPlaceholderText("Select piece theme");
  m_board_theme_selection->setPlaceholderText("Select board theme");

  m_piece_theme_selection->addItems(
      QDir(PieceImages::path_to_themes())
          .entryList(QDir::Dirs | QDir::NoDotAndDotDot));
  m_board_theme_selection->addItems(
      QDir(BoardImage::path_to_themes())
          .entryList(QDir::Dirs | QDir::NoDotAndDotDot));

  connect(m_piece_theme_selection, &QComboBox::currentTextChanged,
          [this](QString text) {
            if (this->m_piece_theme_selection->currentIndex() != -1) {
              PieceImages::load(text.toStdString());
              this->m_board_scene->reload();

              set_label_piece_pixmap(this->m_clock_piece_white,
                                     libataxx::Piece::White,
                                     this->m_clock_white->height());
              set_label_piece_pixmap(this->m_clock_piece_black,
                                     libataxx::Piece::Black,
                                     this->m_clock_black->height());

              set_label_piece_pixmap(this->m_selection_piece_white,
                                     libataxx::Piece::White,
                                     this->m_engine_selection2->height());
              set_label_piece_pixmap(this->m_selection_piece_black,
                                     libataxx::Piece::Black,
                                     this->m_engine_selection1->height());

              this->m_piece_theme_selection->setCurrentIndex((-1));
            }
          });

  connect(m_board_theme_selection, &QComboBox::currentTextChanged,
          [this](QString text) {
            if (this->m_board_theme_selection->currentIndex() != -1) {
              BoardImage::load(text.toStdString());
              this->m_board_scene->reload();
              this->m_board_theme_selection->setCurrentIndex((-1));
            }
          });

  engine_selection_layout->addWidget(m_selection_piece_black, 0, 0);
  engine_selection_layout->addWidget(m_selection_piece_white, 1, 0);
  engine_selection_layout->addWidget(m_engine_selection1, 0, 1);
  engine_selection_layout->addWidget(m_engine_selection2, 1, 1);

  left_layout->addLayout(engine_selection_layout);
  left_layout->addWidget(m_toggle_game_button);
  left_layout->addWidget(edit_settings_button);
  left_layout->addWidget(m_piece_theme_selection);
  left_layout->addWidget(m_board_theme_selection);
  left_layout->addStretch(1);
  main_layout->addLayout(left_layout);

  connect(m_toggle_game_button, &QPushButton::clicked, [this]() {
    if (this->m_toggle_game_button->text() == "Start Game") {
      start_game();
    } else {
      stop_game();
    }
  });

  set_label_piece_pixmap(m_clock_piece_white, libataxx::Piece::White,
                         m_clock_white->height());
  set_label_piece_pixmap(m_clock_piece_black, libataxx::Piece::Black,
                         m_clock_black->height());

  m_turn_radio_white->setEnabled(false);
  m_turn_radio_black->setEnabled(false);
  m_turn_radio_white->setLayoutDirection(Qt::RightToLeft);
  m_turn_radio_white->setMaximumWidth(m_clock_black->height());
  m_turn_radio_black->setMaximumWidth(m_clock_black->height());

  clock_layout->addWidget(m_turn_radio_white);
  clock_layout->addWidget(m_clock_piece_white);
  clock_layout->addWidget(m_clock_white);
  clock_layout->addWidget(m_clock_black);
  clock_layout->addWidget(m_clock_piece_black);
  clock_layout->addWidget(m_turn_radio_black);
  middle_layout->addLayout(clock_layout);

  m_clock_white->set_time(QTime(23, 59, 59));
  m_clock_black->set_time(QTime(23, 59, 59));

  m_board_view->setEnabled(true);
  middle_layout->addWidget(m_board_view);

  set_fen_layout->addWidget(fen_label);

  set_fen_layout->addWidget(m_fen_text_field);

  connect(m_board_scene, &BoardScene::new_fen, m_fen_text_field,
          &QLineEdit::setText);
  connect(m_fen_text_field, &QLineEdit::editingFinished, [this]() {
    if (m_game_worker == nullptr) {
      m_board_scene->set_board(
          libataxx::Position(m_fen_text_field->text().toStdString()));
    } else {
      this->m_fen_text_field->setText(
          QString::fromStdString(m_board_scene->board().get_fen()));
    }
  });

  set_fen_layout->addWidget(m_fen_set_fen);
  middle_layout->addLayout(set_fen_layout);

  m_start_pos_selection->setPlaceholderText("Select start position");
  for (size_t i = 0; i < start_positions.size(); ++i) {
    m_start_pos_selection->addItem(
        (std::to_string(i + 1) + ". " + start_positions.at(i)).c_str());
  }
  connect(m_start_pos_selection, &QComboBox::currentTextChanged,
          [this](QString text) {
            if (this->m_start_pos_selection->currentIndex() != -1 &&
                m_game_worker == nullptr) {
              auto startpos = text.toStdString();
              while (true) {
                auto c = startpos.front();
                startpos.erase(0, 1);
                if (c == '.')
                  break;
              }
              this->m_board_scene->set_board(libataxx::Position(startpos));
              this->m_start_pos_selection->setCurrentIndex((-1));
            }
          });

  m_start_pos_selection->setCurrentIndex((-1));
  middle_layout->addWidget(m_start_pos_selection);

  main_layout->addLayout(middle_layout);

  // Create right vertical layout for text field
  m_pgn_text_field->setReadOnly(true);
  right_layout->addWidget(m_pgn_text_field);

  // Add both layouts to the main layout
  main_layout->addLayout(right_layout);

  // Set layout to central widget
  central_widget->setLayout(main_layout);

  main_layout->setStretch(0, 1);
  main_layout->setStretch(1, 3);
  main_layout->setStretch(2, 1);

  m_board_scene->set_board(libataxx::Position(start_positions.front()));
}

void MainWindow::edit_settings() {
  TextEditor *editor = new TextEditor(m_settings_file_path.string());
  connect(editor, &TextEditor::changed_settings, this,
          &MainWindow::load_settings);
  editor->setAttribute(Qt::WA_DeleteOnClose);
  editor->resize(size());
  editor->show();
}

void MainWindow::start_game() {
  const int time = m_time_spin_box->time().msecsSinceStartOfDay();
  const int inc = m_inc_spin_box->time().msecsSinceStartOfDay();
  const auto tc = SearchSettings::as_time(time, time, inc, inc);

  const auto create_engine = [this, tc](std::string engine_name) {
    std::shared_ptr<Engine> engine{};
    EngineSettings engine_settings{};

    if (engine_name == human_engine_name) {
      engine_settings.name = human_engine_name;
      engine_settings.tc = this->m_human_infinite_time_checkbox->isChecked()
                               ? SearchSettings::as_depth(1)
                               : tc;

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
    std::tie(engine1, engine_setting1) =
        create_engine(this->m_engine_selection1->currentText().toStdString());
    std::tie(engine2, engine_setting2) =
        create_engine(this->m_engine_selection2->currentText().toStdString());
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
  m_game_worker =
      new GameWorker(AdjudicationSettings{},
                     GameSettings{.fen = this->m_board_scene->board().get_fen(),
                                  .engine1 = engine_setting1,
                                  .engine2 = engine_setting2},
                     engine1, engine2);

  m_game_worker->moveToThread(&m_worker_thread);

  connect(&m_worker_thread, &QThread::finished, m_game_worker,
          &QObject::deleteLater);

  connect(m_game_worker, &GameWorker::finished_game, this->m_board_scene,
          &BoardScene::on_game_finished, Qt::QueuedConnection);

  connect(
      m_game_worker, &GameWorker::finished_game, this,
      [this]() {
        m_clock_white->stop_clock();
        m_clock_black->stop_clock();
      },
      Qt::QueuedConnection);

  connect(
      m_game_worker, &GameWorker::finished_game, this,
      [this](GameThingy info) {
        m_pgn_text_field->setText(QString::fromStdString(get_pgn(
            PGNSettings{

            },
            this->m_engine_selection1->currentText().toStdString(),
            this->m_engine_selection2->currentText().toStdString(), info)));
        this->stop_game();
      },
      Qt::QueuedConnection);

  connect(
      m_game_worker, &GameWorker::new_move, this,
      [this](GameThingy info) {
        this->m_board_scene->on_new_move(info.history.back().move);

        m_pgn_text_field->setText(QString::fromStdString(get_pgn(
            PGNSettings{

            },
            this->m_engine_selection1->currentText().toStdString(),
            this->m_engine_selection2->currentText().toStdString(), info)));
      },
      Qt::QueuedConnection);

  // Black is engine1 (tc1)
  connect(
      m_game_worker, &GameWorker::update_time_control, this,
      [this](SearchSettings tc1, SearchSettings tc2,
             libataxx::Side side_to_move) {
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

MainWindow::~MainWindow() { stop_game(); }