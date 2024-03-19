/*
    This file is part of Cute libataxx.

    Cute libataxx is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute libataxx is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute libataxx.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    This file was heavily modified from its original Cute Chess source by tsoj.
*/

#include "graphicsboard.hpp"
#include <QApplication>
#include <QGraphicsRectItem>
#include <QMargins>
#include <QPainter>
#include <QPalette>
#include <QPropertyAnimation>
#include <libataxx/square.hpp>
#include "graphicspiece.hpp"
#include "images.hpp"

namespace {}  // anonymous namespace

GraphicsBoard::GraphicsBoard(qreal squareSize, QGraphicsItem* parent)
    : QGraphicsItem(parent),
      m_square_size(squareSize),
      m_coord_size(squareSize / 2.0),
      m_squares(files * ranks),
      m_flipped(false) {
    m_rect.setSize(QSizeF(squareSize * files, squareSize * ranks));
    m_rect.moveCenter(QPointF(0, 0));
    m_text_color = QApplication::palette().text().color();

    setCacheMode(DeviceCoordinateCache);
}

GraphicsBoard::~GraphicsBoard() {
}

int GraphicsBoard::type() const {
    return Type;
}

QRectF GraphicsBoard::boundingRect() const {
    const auto margins = QMarginsF(m_coord_size, m_coord_size, m_coord_size, m_coord_size);
    return m_rect.marginsAdded(margins);
}

void GraphicsBoard::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawPixmap(m_rect.toRect(), BoardImage::board_image());

    QRectF rect(m_rect.topLeft(), QSizeF(m_square_size, m_square_size));

    auto font = painter->font();
    font.setPointSizeF(font.pointSizeF() * 0.7);
    painter->setFont(font);
    painter->setPen(m_text_color);

    // paint file coordinates
    const QString alphabet = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < files; i++) {
        const qreal tops[] = {m_rect.top() - m_coord_size, m_rect.bottom()};
        for (const auto top : tops) {
            rect = QRectF(m_rect.left() + (m_square_size * i), top, m_square_size, m_coord_size);
            int file = m_flipped ? files - i - 1 : i;
            painter->drawText(rect, Qt::AlignCenter, alphabet[file]);
        }
    }

    // paint rank coordinates
    for (int i = 0; i < ranks; i++) {
        const qreal lefts[] = {m_rect.left() - m_coord_size, m_rect.right()};
        for (const auto left : lefts) {
            rect = QRectF(left, m_rect.top() + (m_square_size * i), m_coord_size, m_square_size);
            int rank = m_flipped ? i + 1 : ranks - i;
            const auto num = QString::number(rank);
            painter->drawText(rect, Qt::AlignCenter, num);
        }
    }
}

std::optional<libataxx::Square> GraphicsBoard::square_at(const QPointF& point) const {
    if (!m_rect.contains(point)) return std::nullopt;

    int col = (point.x() + m_rect.width() / 2) / m_square_size;
    int row = (point.y() + m_rect.height() / 2) / m_square_size;

    if (m_flipped) return libataxx::Square(files - col - 1, row);
    return libataxx::Square(col, ranks - row - 1);
}

QPointF GraphicsBoard::square_pos(const libataxx::Square& square) const {
    qreal x = m_rect.left() + m_square_size / 2;
    qreal y = m_rect.top() + m_square_size / 2;

    if (m_flipped) {
        x += m_square_size * (files - static_cast<int>(square.file()) - 1);
        y += m_square_size * static_cast<int>(square.rank());
    } else {
        x += m_square_size * static_cast<int>(square.file());
        y += m_square_size * (ranks - static_cast<int>(square.rank()) - 1);
    }

    return QPointF(x, y);
}

libataxx::Piece GraphicsBoard::piece_type_at(const libataxx::Square& square) const {
    GraphicsPiece* piece = piece_at(square);
    if (piece == nullptr) return libataxx::Piece();
    return piece->piece_type();
}

GraphicsPiece* GraphicsBoard::piece_at(const libataxx::Square& square) const {
    GraphicsPiece* piece = m_squares.at(square_index(square));
    Q_ASSERT(piece == nullptr || piece->container() == this);
    return piece;
}

GraphicsPiece* GraphicsBoard::take_piece_at(const libataxx::Square& square) {
    int index = square_index(square);
    if (index == -1) return nullptr;

    GraphicsPiece* piece = m_squares.at(index);
    if (piece == nullptr) return nullptr;

    m_squares[index] = nullptr;
    piece->setParentItem(nullptr);
    piece->set_container(nullptr);

    return piece;
}

void GraphicsBoard::clear_squares() {
    qDeleteAll(m_squares);
    m_squares.clear();
}

void GraphicsBoard::set_square(const libataxx::Square& square, GraphicsPiece* piece) {
    int index = square_index(square);
    delete m_squares[index];

    if (piece == nullptr)
        m_squares[index] = nullptr;
    else {
        m_squares[index] = piece;
        piece->set_container(this);
        piece->setParentItem(this);
        piece->setPos(square_pos(square));
    }
}

void GraphicsBoard::move_piece(const libataxx::Square& source, const libataxx::Square& target) {
    GraphicsPiece* piece = piece_at(source);
    Q_ASSERT(piece != nullptr);

    m_squares[square_index(source)] = nullptr;
    set_square(target, piece);
}

int GraphicsBoard::square_index(const libataxx::Square& square) const {
    return static_cast<int>(square.rank()) * files + static_cast<int>(square.file());
}

void GraphicsBoard::clear_highlights() {
    for (auto a : m_highlights) {
        a->deleteLater();
    }
    m_highlights.clear();
}

void GraphicsBoard::set_highlights(const QList<libataxx::Square>& squares) {
    if (squares.isEmpty()) return;

    m_highlights.push_back(new TargetHighlights(this));

    QRectF rect;
    rect.setSize(QSizeF(m_square_size / 3, m_square_size / 3));
    rect.moveCenter(QPointF(0, 0));

    QRectF bigRect;
    bigRect.setSize(QSizeF(m_square_size, m_square_size));
    bigRect.moveCenter(QPointF(0, 0));

    QPen pen = QPen(Qt::white, m_square_size / 20);
    QBrush brush = QBrush(Qt::black);

    for (const auto& sq : squares) {
        QAbstractGraphicsShapeItem* dot = new QGraphicsEllipseItem(rect, m_highlights.back());

        dot->setCacheMode(DeviceCoordinateCache);
        dot->setOpacity(0.6);
        dot->setPen(pen);
        dot->setBrush(brush);
        dot->setPos(square_pos(sq));
    }
}

bool GraphicsBoard::is_flipped() const {
    return m_flipped;
}

void GraphicsBoard::set_flipped(bool flipped) {
    if (flipped == m_flipped) return;

    clear_highlights();
    m_flipped = flipped;
    update();
}
