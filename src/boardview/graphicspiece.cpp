/*
    This file is part of Cute libataxx.
    Copyright (C) 2008-2018 Cute libataxx authors

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

#include "graphicspiece.hpp"
#include <QPainter>
#include "images.hpp"

GraphicsPiece::GraphicsPiece(const libataxx::Piece& piece, qreal squareSize, QGraphicsItem* parent)
    : QGraphicsObject(parent),
      m_piece(piece),
      m_rect(-squareSize / 2, -squareSize / 2, squareSize, squareSize),
      m_container(nullptr) {
    setAcceptedMouseButtons(Qt::LeftButton);
    setCacheMode(DeviceCoordinateCache);
}

int GraphicsPiece::type() const {
    return Type;
}

QRectF GraphicsPiece::boundingRect() const {
    return m_rect;
}

void GraphicsPiece::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->drawPixmap(m_rect.toRect(), PieceImages::piece_images(m_piece));
}

libataxx::Piece GraphicsPiece::piece_type() const {
    return m_piece;
}

QGraphicsItem* GraphicsPiece::container() const {
    return m_container;
}

void GraphicsPiece::set_container(QGraphicsItem* item) {
    m_container = item;
}

void GraphicsPiece::restore_parent() {
    if (parentItem() == nullptr && m_container != nullptr) {
        QPointF point(m_container->mapFromScene(pos()));
        setParentItem(m_container);
        setPos(point);
    }
}
