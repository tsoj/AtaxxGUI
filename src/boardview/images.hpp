#pragma once
#include <QCoreApplication>
#include <QGraphicsObject>
#include <QPixmap>
#include <filesystem>
#include <libataxx/piece.hpp>

class PieceImages {
   private:
    inline static std::optional<std::array<QPixmap, 3>> piece_image_table;

   public:
    static inline const QPixmap& piece_images(const libataxx::Piece& piece) {
        if (!piece_image_table.has_value()) {
            load();
        }
        Q_ASSERT(piece_image_table.has_value());
        return piece_image_table.value().at(static_cast<int>(piece));
    }

    static inline std::filesystem::path path_to_themes() {
        return QCoreApplication::applicationDirPath().toStdString() + "/piece_images/";
    }

    static inline void load(std::string name = "") {
        const std::filesystem::path dst_path =
            QCoreApplication::applicationDirPath().toStdString() + "/selected_piece_images/";

        if (name != "" || !std::filesystem::exists(dst_path)) {
            if (name == "") {
                name = "default";
            }
            const std::filesystem::path src_path = path_to_themes() / name;

            std::filesystem::create_directories(dst_path);

            for (auto f : std::vector{"x.png", "o.png", "-.png"}) {
                std::filesystem::copy_file(src_path / f, dst_path / f, std::filesystem::copy_options::overwrite_existing);
            }
        }

        piece_image_table = {
            QPixmap(QString::fromStdString(dst_path.string() + "x.png")),
            QPixmap(QString::fromStdString(dst_path.string() + "o.png")),
            QPixmap(QString::fromStdString(dst_path.string() + "-.png")),
        };
    }
};

class BoardImage {
   private:
    inline static std::optional<QPixmap> board_image_pixmap;

   public:
    static inline const QPixmap& board_image() {
        if (!board_image_pixmap.has_value()) {
            load();
        }
        Q_ASSERT(board_image_pixmap.has_value());
        return board_image_pixmap.value();
    }

    static inline std::filesystem::path path_to_themes() {
        return QCoreApplication::applicationDirPath().toStdString() + "/board_images/";
    }

    static inline void load(std::string name = "") {
        const std::filesystem::path dst_path =
            QCoreApplication::applicationDirPath().toStdString() + "/selected_board_image/";

        if (name != "" || !std::filesystem::exists(dst_path)) {
            if (name == "") {
                name = "default";
            }
            const std::filesystem::path src_path = path_to_themes() / name;

            std::filesystem::create_directories(dst_path);

            std::filesystem::copy_file(
                src_path / "board.png", dst_path / "board.png", std::filesystem::copy_options::overwrite_existing);
        }

        board_image_pixmap = QPixmap(QString::fromStdString(dst_path.string() + "board.png"));
    }
};