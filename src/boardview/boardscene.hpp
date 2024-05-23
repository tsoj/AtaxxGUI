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

#ifndef BOARDSCENE_H
#define BOARDSCENE_H

#include <QGraphicsScene>
#include <QMultiMap>
#include <QPointer>
#include <QSettings>
#include <libataxx/move.hpp>
#include <libataxx/position.hpp>
#include <libataxx/square.hpp>
#include <play.hpp>

class QSvgRenderer;
class QAbstractAnimation;
class QPropertyAnimation;
class GraphicsBoard;
class GraphicsPiece;

/*!
 * \brief A graphical surface for displaying a chessgame.
 *
 * BoardScene is the top-level container for the board, the
 * chess pieces, etc. It also manages the deletion and creation
 * of pieces, visualising moves made in a game, emitting moves
 * made by a human player, etc.
 *
 * BoardScene is that class that connects to the players and game
 * objects to synchronize the graphical side with the internals.
 */
class BoardScene : public QGraphicsScene {
    Q_OBJECT

   public:
    /*! Creates a new BoardScene object. */
    explicit BoardScene(QObject* parent = nullptr);
    /*! Destroys the scene and all its items. */
    virtual ~BoardScene();

    /*! Returns the current internal board object. */
    [[nodiscard]] libataxx::Position board() const;

    void make_move(const libataxx::Move& move, std::optional<libataxx::Square> source);

   public slots:
    /*!
     * Clears the scene and sets \a board as the internal board.
     *
     * The scene takes ownership of the board, so it's usually
     * best to give the scene its own copy of a board.
     */
    void set_board(const libataxx::Position& board);
    void reload();
    /*! Makes the move \a move in the scene. */
    void on_new_move(const libataxx::Move& move);
    /*! Flips the board, with animation. */
    void flip();
    void accept_move_input(bool value);
    /*!
     * Cancels any ongoing user move and flashes \a result
     * over the board.
     */
    void on_game_finished(libataxx::Result result);

   signals:
    /*!
     * This signal is emitted when a human player has made a move.
     *
     * The move is guaranteed to be legal.
     * The move was made by the player on \a side side.
     */
    void human_move(const libataxx::Move move, const libataxx::Side side);
    void new_fen(QString fen);

   protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

   private:
    void clear_selection();

    enum MoveDirection
    {
        Forward,
        Backward
    };

    struct SquareComp {
        bool operator()(libataxx::Square lhs, libataxx::Square rhs) const {
            return static_cast<int>(lhs) < static_cast<int>(rhs);
        }
    };

    QPointF square_pos(const libataxx::Square& square) const;
    GraphicsPiece* piece_at(const QPointF& pos) const;
    GraphicsPiece* create_piece(const libataxx::Piece& piece);
    QPropertyAnimation* piece_animation(GraphicsPiece* piece, const QPointF& end_point) const;
    void stop_animation();
    void add_move_arrow(const QPointF& source_pos, const QPointF& target_pos);
    void apply_transition(const libataxx::Square& source, const libataxx::Square& target);
    void update_moves();

    libataxx::Position m_board;
    GraphicsBoard* m_squares;
    std::optional<libataxx::Square> m_selected_square;
    QPointer<QAbstractAnimation> m_anim;
    std::multimap<libataxx::Square, libataxx::Square, SquareComp> m_targets;
    GraphicsPiece* m_highlight_piece;
    QGraphicsItemGroup* m_move_arrows;
    bool m_accept_move_input = false;
    QSettings m_settings;
};

#endif  // BOARDSCENE_H
