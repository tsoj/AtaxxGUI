
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
#include <string_view>
#include "boardview/boardscene.hpp"
#include "boardview/boardview.hpp"
#include "countdowntimer.hpp"
// #include "gameworker.hpp"
// #include "humanengine.hpp"

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
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #dcdcdc);"
            "border: 1px solid #5c5c5c;"
            "height: 26px;"     // handle thickness
            "width: 26px;"      // Handle width
            "margin: -16px 0;"  // Handle margin
            "border-radius: 3px;"
            "}");

        this->setMinimum(-49);  // Set the minimum value
        this->setMaximum(49);   // Set the maximum value
        this->setValue(0);      // Set an initial value if needed
    }

    //    protected:
    //     void mousePressEvent([[maybe_unused]] QMouseEvent* event) override {
    //         // Do nothing to ignore mouse press events
    //     }

    //     void mouseMoveEvent([[maybe_unused]] QMouseEvent* event) override {
    //         // Do nothing to ignore mouse move events
    //     }

    //     void mouseReleaseEvent([[maybe_unused]] QMouseEvent* event) override {
    //         // Do nothing to ignore mouse release events
    //     }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(const std::string& settingsFileName, QWidget* parent = nullptr);
    ~MainWindow();

   private:
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
};