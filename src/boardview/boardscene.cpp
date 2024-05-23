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

#include "boardscene.hpp"
#include <qnamespace.h>
#include <QCoreApplication>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QSettings>
#include "graphicsboard.hpp"
#include "graphicspiece.hpp"
#include "pgn.hpp"

namespace {

constexpr qreal square_size = 100;

std::vector<libataxx::Square> get_sources(const libataxx::Move& move, const libataxx::Position& position) {
    if (move.from() != move.to()) {
        return {move.from()};
    }

    std::vector<libataxx::Square> froms;
    for (const auto from : (libataxx::Bitboard(move.to()).singles() & position.get_us())) {
        froms.push_back(from);
    }
    return froms;
}

bool at_single_distance(libataxx::Square a, libataxx::Square b) {
    return libataxx::Bitboard(a) & libataxx::Bitboard(b).singles();
}

}  // anonymous namespace

BoardScene::BoardScene(QObject* parent)
    : QGraphicsScene(parent), m_squares(nullptr), m_anim(nullptr), m_highlight_piece(nullptr), m_move_arrows(nullptr) {
}

BoardScene::~BoardScene() {
    clear_selection();
    stop_animation();
}

libataxx::Position BoardScene::board() const {
    return m_board;
}

void BoardScene::set_board(const libataxx::Position& board) {
    const bool was_flipped = m_squares ? m_squares->is_flipped() : false;
    stop_animation();

    clear();
    m_squares = nullptr;
    m_highlight_piece = nullptr;
    m_move_arrows = nullptr;
    m_board = board;

    delete m_squares;
    m_squares = new GraphicsBoard(square_size);
    m_squares->set_flipped(was_flipped);

    addItem(m_squares);

    setSceneRect(itemsBoundingRect());

    for (int f = 0; f < 7; ++f) {
        for (int r = 0; r < 7; ++r) {
            const auto sq = libataxx::Square{f, r};
            GraphicsPiece* piece(create_piece(m_board.get(sq)));

            if (piece != nullptr) m_squares->set_square(sq, piece);
        }
    }
    clear_selection();
    update_moves();
}

void BoardScene::reload() {
    set_board(board());
}

void BoardScene::make_move(const libataxx::Move& move, std::optional<libataxx::Square> source) {
    Q_ASSERT(move != libataxx::Move::nomove());
    Q_ASSERT(move.from() == source.value_or(move.from()) || move.is_single() || move == libataxx::Move::nullmove());
    clear_selection();
    stop_animation();
    delete m_move_arrows;
    m_move_arrows = new QGraphicsItemGroup(m_squares);
    m_move_arrows->setZValue(3);

    Q_ASSERT(m_board.is_legal_move(move));

    const auto old_board = m_board;

    m_board.makemove(move);

    if (move != libataxx::Move::nullmove()) {
        source = source.value_or(get_sources(move, old_board).back());
        apply_transition(source.value(), move.to());
    } else {
        update_moves();
    }
}

void BoardScene::on_new_move(const libataxx::Move& move) {
    make_move(move, std::nullopt);
}

void BoardScene::clear_selection() {
    m_selected_square = std::nullopt;
    if (m_squares) {
        m_squares->clear_highlights();
    }
}

void BoardScene::flip() {
    clear_selection();
    stop_animation();
    m_squares->set_flipped(!m_squares->is_flipped());

    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    connect(group, &QParallelAnimationGroup::finished, [this]() {
        this->set_board(this->m_board);
    });
    m_anim = group;

    for (int f = 0; f < 7; ++f) {
        for (int r = 0; r < 7; ++r) {
            const auto sq = libataxx::Square{f, r};
            auto pc = m_squares->piece_at(sq);
            if (!pc) continue;
            group->addAnimation(piece_animation(pc, square_pos(sq)));
        }
    }
    delete m_move_arrows;
    m_move_arrows = nullptr;

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void BoardScene::accept_move_input(bool value) {
    m_accept_move_input = value;
}

void BoardScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsScene::mouseReleaseEvent(event);

    if (!m_accept_move_input) {
        return;
    }

    const auto previous_square = m_selected_square;
    clear_selection();

    delete m_move_arrows;
    m_move_arrows = nullptr;

    if (event->button() != Qt::LeftButton) {
        // flip();
        return;
    }

    QPointF target_pos(m_squares->mapFromScene(event->scenePos()));
    const auto potential_clicked_square = m_squares->square_at(target_pos);
    if (potential_clicked_square.has_value()) {
        const auto clicked_square = potential_clicked_square.value();
        if ((!previous_square.has_value() || clicked_square != *previous_square)) {
            m_selected_square = clicked_square;

            if (libataxx::Bitboard(clicked_square) & m_board.get_us()) {
                if (m_targets.contains(clicked_square)) {
                    QList<libataxx::Square> targets;
                    for (const auto& [source, target] : m_targets) {
                        if (source == clicked_square) {
                            targets.append(target);
                        }
                    }

                    m_squares->set_highlights(targets);
                }
            }
        }
    }

    if (previous_square.has_value() && m_selected_square.has_value()) {
        const libataxx::Square target = m_selected_square.value();
        libataxx::Square source = previous_square.value();

        if (at_single_distance(target, source) && (m_board.get_us() & libataxx::Bitboard(source)) != 0) {
            source = target;
        }

        auto move = libataxx::Move(source, target);

        if (m_board.is_legal_move(move)) {
            emit human_move(move, m_board.get_turn());
        }
    }
}

void BoardScene::on_game_finished(libataxx::Result result) {
    delete m_move_arrows;
    m_move_arrows = nullptr;
    clear_selection();

    const QString result_string = [result]() {
        switch (result) {
            case libataxx::Result::BlackWin:
                return "1-0";
            case libataxx::Result::WhiteWin:
                return "0-1";
            case libataxx::Result::Draw:
                return "1/2-1/2";
            default:
                return "*";
        }
    }();

    auto text = new QGraphicsTextItem(result_string, m_squares);
    auto font = text->font();
    font.setPointSize(10);
    font.setBold(true);
    text->setFont(font);
    text->setOpacity(0.9);
    const auto rect = text->boundingRect();
    qreal x = rect.width() / 2;
    qreal y = rect.height() / 2;
    text->setPos(-x, -y);
    text->setTransformOriginPoint(x, y);
    text->setZValue(2);

    auto group = new QSequentialAnimationGroup;
    text->setParent(group);
    m_anim = group;

    auto sc_anim = new QPropertyAnimation(text, "scale");
    sc_anim->setStartValue(text->scale());
    sc_anim->setEndValue(6.5);
    sc_anim->setEasingCurve(QEasingCurve::InOutQuad);
    sc_anim->setDuration(2000);
    group->addAnimation(sc_anim);

    auto pause_anim = new QPauseAnimation(2000);
    group->addAnimation(pause_anim);

    auto op_anim = new QPropertyAnimation(text, "opacity");
    op_anim->setStartValue(text->opacity());
    op_anim->setEndValue(0.0);
    op_anim->setEasingCurve(QEasingCurve::InOutQuad);
    op_anim->setDuration(2500);
    group->addAnimation(op_anim);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

QPointF BoardScene::square_pos(const libataxx::Square& square) const {
    return m_squares->mapToScene(m_squares->square_pos(square));
}

GraphicsPiece* BoardScene::piece_at(const QPointF& pos) const {
    const auto items = this->items(pos);
    for (auto item : items) {
        GraphicsPiece* piece = qgraphicsitem_cast<GraphicsPiece*>(item);
        if (piece != nullptr) return piece;
    }

    return nullptr;
}

GraphicsPiece* BoardScene::create_piece(const libataxx::Piece& piece) {
    Q_ASSERT(m_squares != nullptr);

    if (piece == libataxx::Piece::Empty) return nullptr;

    return new GraphicsPiece(piece, square_size);
}

QPropertyAnimation* BoardScene::piece_animation(GraphicsPiece* piece, const QPointF& endPoint) const {
    Q_ASSERT(piece != nullptr);

    QPointF start_point(piece->scenePos());
    QPropertyAnimation* anim = new QPropertyAnimation(piece, "pos");
    connect(anim, &QPropertyAnimation::finished, piece, &GraphicsPiece::restore_parent);

    anim->setStartValue(start_point);
    anim->setEndValue(endPoint);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setDuration(400);

    piece->setParentItem(nullptr);
    piece->setPos(start_point);

    return anim;
}

void BoardScene::stop_animation() {
    if (m_anim != nullptr && m_anim->state() == QAbstractAnimation::Running)
        m_anim->setCurrentTime(m_anim->totalDuration());
}

void BoardScene::add_move_arrow(const QPointF& sourcePos, const QPointF& targetPos) {
    Q_ASSERT(m_move_arrows != nullptr);

    QPolygonF arrow;
    QLineF origline(sourcePos, targetPos);

    qreal size = square_size / 7.0;
    qreal skew = size * 0.5;
    qreal risex = 3;
    qreal risey = 2;
    qreal length = origline.length();

    arrow << QPointF(0, 0);
    arrow << QPointF(-risex * size, risey * size);
    arrow << QPointF(-risex * size, skew);
    arrow << QPointF(-length, skew);
    arrow << QPointF(-length, -skew);
    arrow << QPointF(-risex * size, -skew);
    arrow << QPointF(-risex * size, -risey * size);

    QGraphicsPolygonItem* item = new QGraphicsPolygonItem(arrow);
    item->setPen(QPen(QBrush(QColor(Qt::white)), 4));
    item->setBrush(QColor(Qt::black));
    item->setOpacity(0.6);
    item->setRotation(-origline.angle());
    item->setPos(targetPos);

    m_move_arrows->addToGroup(item);
}

void BoardScene::apply_transition(const libataxx::Square& source, const libataxx::Square& target) {
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    connect(group, &QParallelAnimationGroup::finished, [this, source, target]() {
        this->m_squares->move_piece(source, target);
        for (const auto sq : (this->m_board.get_them() & libataxx::Bitboard(target).singles())) {
            this->m_squares->set_square(sq, this->create_piece(this->m_board.get(sq)));
        }

        this->clear_selection();
        this->update_moves();
    });
    m_anim = group;

    GraphicsPiece* animation_piece = nullptr;
    if (!at_single_distance(source, target) && source != target) {
        animation_piece = m_squares->piece_at(source);
    }
    if (animation_piece == nullptr) {
        animation_piece = create_piece(m_board.get(target));
        m_squares->set_square(target, animation_piece);
        animation_piece->setPos(m_squares->square_pos(source));
    }

    group->addAnimation(piece_animation(animation_piece, square_pos(target)));

    add_move_arrow(m_squares->square_pos(source), m_squares->square_pos(target));

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void BoardScene::update_moves() {
    m_targets.clear();

    const auto moves = m_board.legal_moves();
    for (const auto& move : moves) {
        for (const auto from : get_sources(move, m_board)) {
            m_targets.insert(std::pair{from, move.to()});
        }
    }
    emit new_fen(QString::fromStdString(m_board.get_fen()));
}