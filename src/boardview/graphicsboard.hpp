/*
    This file is part of Cute Chess.
    Copyright (C) 2008-2018 Cute Chess authors

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    This file was heavily modified from its original Cute Chess source by tsoj.
*/

#pragma once

#include <QColor>
#include <QGraphicsItem>
#include <QVector>
#include <libataxx/piece.hpp>
#include <libataxx/square.hpp>

class GraphicsPiece;
class QPropertyAnimation;

/*!
 * \brief A graphical chessboard.
 *
 * GraphicsBoard is a graphical representation of the squares on a
 * chessboard. It also has ownership of the chess pieces on the
 * board, ie. it is the pieces' parent item and container.
 */
class GraphicsBoard : public QGraphicsItem {
   public:
    /*! The type value returned by type(). */
    enum
    {
        Type = UserType + 1
    };

    /*!
     * Creates a new GraphicsBoard object.
     *
     * The board will have \a files files/columns and \a ranks
     * ranks/rows, and the squares' width and height will be
     * \a squareSize.
     */
    explicit GraphicsBoard(qreal square_size, QGraphicsItem* parent = nullptr);
    /*! Destroys the GraphicsBoard object. */
    virtual ~GraphicsBoard();

    // Inherited from QGraphicsItem
    virtual int type() const;

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);

    /*!
     * Returns the chess square at \a point.
     *
     * \a point is in item coordinates.
     * Returns a null square if \a point is not on the board.
     */
    std::optional<libataxx::Square> square_at(const QPointF& point) const;
    /*!
     * Returns the position of \a square.
     *
     * The returned position is in item coordinates.
     * Returns a null point if \a square is not on the board.
     */
    QPointF square_pos(const libataxx::Square& square) const;
    /*!
     * Returns the type of piece at \a square.
     *
     * Returns a null piece if \a square is not on the board or
     * if there's no piece placed on it.
     */
    libataxx::Piece piece_type_at(const libataxx::Square& square) const;
    /*!
     * Returns the GraphicsPiece object at \a square.
     *
     * Returns 0 if \a square is not on the board or if there's
     * no piece placed on it.
     */
    GraphicsPiece* piece_at(const libataxx::Square& square) const;
    /*!
     * Removes the GraphicsPiece object at \a square and returns it.
     *
     * Returns 0 if \a square is not on the board or if there's
     * no piece placed on it.
     */
    GraphicsPiece* take_piece_at(const libataxx::Square& square);

    /*! Deletes all pieces and removes them from the scene. */
    void clear_squares();
    /*!
     * Sets the piece at \a square to \a piece.
     *
     * If \a square already contains a piece, it is deleted.
     * If \a piece is 0, the square becomes empty.
     */
    void set_square(const libataxx::Square& square, GraphicsPiece* piece);
    /*!
     * Moves the piece from \a source to \a target.
     *
     * If \a target already contains a piece, it is deleted.
     */
    void move_piece(const libataxx::Square& source, const libataxx::Square& target);

    /*! Clears all highlights. */
    void clear_highlights();
    /*!
     * Highlights squares.
     *
     * This function clears all previous highlights and marks the
     * squares in \a squares as possible target squares of a chess move.
     */
    void set_highlights(const QList<libataxx::Square>& squares);

    /*!
     * Returns true if the board is flipped;
     * otherwise returns false.
     */
    bool is_flipped() const;

    /*! Sets board flipping to \a flipped. */
    void set_flipped(bool flipped);

   private:
    class TargetHighlights : public QGraphicsObject {
       public:
        TargetHighlights(QGraphicsItem* parentItem = nullptr) : QGraphicsObject(parentItem) {
            setFlag(ItemHasNoContents);
        }
        virtual QRectF boundingRect() const {
            return QRectF();
        }
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
            Q_UNUSED(painter);
            Q_UNUSED(option);
            Q_UNUSED(widget);
        }
    };

    int square_index(const libataxx::Square& square) const;

    static constexpr int files = 7;
    static constexpr int ranks = 7;
    qreal m_square_size;
    qreal m_coord_size;
    QRectF m_rect;
    QColor m_text_color;
    QVector<GraphicsPiece*> m_squares;
    std::vector<TargetHighlights*> m_highlights;
    bool m_flipped;
};
