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

#pragma once

#include <QGraphicsObject>
#include <QPixmap>
#include <libataxx/piece.hpp>

/*!
 * \brief A graphical representation of a libataxx piece.
 *
 * A GraphicsPiece object is a libataxx piece that can be easily
 * dragged and animated in a QGraphicsScene. Scalable Vector
 * Graphics (SVG) are used to ensure that the pieces look good
 * at any resolution, and a shared SVG renderer is used.
 *
 * For convenience reasons the boundingRect() of a piece should
 * be equal to that of a square on the libataxxboard.
 */
class GraphicsPiece : public QGraphicsObject {
    Q_OBJECT

   public:
    /*! The type value returned by type(). */
    enum
    {
        Type = UserType + 4
    };

    /*!
     * Creates a new GraphicsPiece object of type \a piece.
     *
     * The painted image is scaled to fit inside a square that is
     * \a squareSize wide and high.
     * \a elementId is the XML ID of the piece picture which is
     * rendered by \a renderer.
     */
    GraphicsPiece(const libataxx::Piece& piece, qreal square_size, QGraphicsItem* parent = nullptr);

    // Inherited from QGraphicsObject
    virtual int type() const;
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);

    /*! Returns the type of the libataxx piece. */
    libataxx::Piece piece_type() const;
    /*!
     * Returns the container of the piece.
     *
     * Usually the container is the libataxxboard or the piece reserve.
     * A piece can have a container even if it doesn't have a parent
     * item.
     */
    QGraphicsItem* container() const;
    /*! Sets the container to \a item. */
    void set_container(QGraphicsItem* item);

   public slots:
    /*!
     * Restores the parent item (container).
     *
     * If the piece doesn't have a parent item but does have a container,
     * this function sets the parent item to the container. Usually this
     * function is called after an animation or drag operation that had
     * cleared the parent item to make the piece a top-level item.
     */
    void restore_parent();

   private:
    libataxx::Piece m_piece;
    QRectF m_rect;
    QGraphicsItem* m_container;
};
