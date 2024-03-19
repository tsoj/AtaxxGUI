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

#include "boardview.hpp"
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>

BoardView::BoardView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent), m_initialized(false), m_resize_timer(new QTimer(this)) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);

    QSizePolicy sp(sizePolicy());
    sp.setHeightForWidth(true);
    setSizePolicy(sp);

    m_resize_timer->setSingleShot(true);
    m_resize_timer->setInterval(300);

    connect(m_resize_timer, &QTimer::timeout, this, &BoardView::fit_to_rect);
    connect(scene, &QGraphicsScene::sceneRectChanged, this, &BoardView::on_scene_rect_changed);
}

QSize BoardView::sizeHint() const {
    QSize size(sceneRect().size().toSize());
    if (!size.isEmpty()) return size;

    return QSize(200, 200);
}

int BoardView::heightForWidth(int width) const {
    QSizeF size(sceneRect().size());
    if (!size.isEmpty()) {
        qreal ar = size.width() / size.height();
        return width / ar;
    }

    return width;
}

void BoardView::paintEvent(QPaintEvent* event) {
    if (!m_resize_pixmap.isNull()) {
        QRect rect(viewport()->rect());
        qreal src_ar = qreal(m_resize_pixmap.width()) / m_resize_pixmap.height();
        qreal trg_ar = qreal(rect.width()) / rect.height();

        if (src_ar > trg_ar)
            rect.setHeight(rect.width() / src_ar);
        else if (src_ar < trg_ar)
            rect.setWidth(rect.height() * src_ar);
        rect.moveCenter(viewport()->rect().center());

        QPainter painter(viewport());
        painter.drawPixmap(rect, m_resize_pixmap);
    } else
        QGraphicsView::paintEvent(event);
}

void BoardView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (!m_initialized) return;

    if (m_resize_pixmap.isNull()) {
        m_resize_pixmap = QPixmap(sceneRect().toRect().size());
        m_resize_pixmap.fill(Qt::transparent);
        QPainter painter(&m_resize_pixmap);
        scene()->render(&painter);
    }

    m_resize_timer->start();
}

void BoardView::fit_to_rect() {
    m_initialized = true;
    m_resize_pixmap = QPixmap();
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void BoardView::on_scene_rect_changed() {
    updateGeometry();
    fit_to_rect();
}
