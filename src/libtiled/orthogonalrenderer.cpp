/*
 * orthogonalrenderer.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "orthogonalrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"

#include <cmath>

using namespace Tiled;

QSize OrthogonalRenderer::mapSize() const
{
    return QSize(map()->width() * map()->tileWidth(),
                 map()->height() * map()->tileHeight());
}

QRect OrthogonalRenderer::boundingRect(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    return QRect(rect.x() * tileWidth,
                 rect.y() * tileHeight,
                 rect.width() * tileWidth,
                 rect.height() * tileHeight);
}

QRectF OrthogonalRenderer::boundingRect(const MapObject *object) const
{
    const QRectF bounds = object->bounds();
    const QRectF rect(tileToPixelCoords(bounds.topLeft()),
                      tileToPixelCoords(bounds.bottomRight()));

    // The -2 and +3 are to account for the pen width and shadow
    if (object->tile()) {
        const QPointF bottomLeft = rect.topLeft();
        const QPixmap &img = object->tile()->image();
        return QRectF(bottomLeft.x(),
                      bottomLeft.y() - img.height(),
                      img.width(),
                      img.height()).adjusted(-1, -1, 1, 1);
    } else if (rect.isNull()) {
        return rect.adjusted(-10 - 2, -10 - 2, 10 + 3, 10 + 3);
    } else {
        const int nameHeight = object->name().isEmpty() ? 0 : 15;
        return rect.adjusted(-2, -nameHeight - 2, 3, 3);
    }
}

QPainterPath OrthogonalRenderer::shape(const MapObject *object) const
{
    const QRectF bounds = object->bounds();
    const QRectF rect(tileToPixelCoords(bounds.topLeft()),
                      tileToPixelCoords(bounds.bottomRight()));

    QPainterPath path;
    if (object->tile()) {
        path.addRect(boundingRect(object));
    } else if (rect.isNull()) {
        path.addEllipse(rect.topLeft(), 20, 20);
    } else {
        path.addRoundedRect(rect, 10, 10);
    }
    return path;
}

void OrthogonalRenderer::drawGrid(QPainter *painter, const QRectF &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const int startX = qMax(0, (int) (rect.x() / tileWidth) * tileWidth);
    const int startY = qMax(0, (int) (rect.y() / tileHeight) * tileHeight);
    const int endX = qMin((int) std::ceil(rect.right()),
                          map()->width() * tileWidth + 1);
    const int endY = qMin((int) std::ceil(rect.bottom()),
                          map()->height() * tileHeight + 1);

    QColor gridColor(Qt::black);
    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);

    if (startY < endY) {
        gridPen.setDashOffset(startY);
        painter->setPen(gridPen);
        for (int x = startX; x < endX; x += tileWidth)
            painter->drawLine(x, startY, x, endY - 1);
    }

    if (startX < endX) {
        gridPen.setDashOffset(startX);
        painter->setPen(gridPen);
        for (int y = startY; y < endY; y += tileHeight)
            painter->drawLine(startX, y, endX - 1, y);
    }
}

void OrthogonalRenderer::drawTileLayer(QPainter *painter,
                                       const TileLayer *layer,
                                       const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const QPointF layerPos(layer->x() * tileWidth,
                           layer->y() * tileHeight);

    painter->translate(layerPos);

    int startX = 0;
    int startY = 0;
    int endX = layer->width();
    int endY = layer->height();

    if (!exposed.isNull()) {
        const QSize maxTileSize = layer->maxTileSize();
        const int extraWidth = maxTileSize.width() - tileWidth;
        const int extraHeight = maxTileSize.height() - tileHeight;
        QRectF rect = exposed.adjusted(-extraWidth, 0, 0, extraHeight);
        rect.translate(-layerPos);

        startX = qMax((int) rect.x() / tileWidth, 0);
        startY = qMax((int) rect.y() / tileHeight, 0);
        endX = qMin((int) std::ceil(rect.right()) / tileWidth + 1, endX);
        endY = qMin((int) std::ceil(rect.bottom()) / tileHeight + 1, endY);
    }

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Tile *tile = layer->tileAt(x, y);
            if (!tile)
                continue;

            const QPixmap &img = tile->image();
            painter->drawPixmap(x * tileWidth,
                                (y + 1) * tileHeight - img.height(),
                                img);
        }
    }

    painter->translate(-layerPos);
}

void OrthogonalRenderer::drawTileSelection(QPainter *painter,
                                           const QRegion &region,
                                           const QColor &color,
                                           const QRectF &exposed) const
{
    foreach (const QRect &r, region.rects()) {
        const QRectF toFill = QRectF(boundingRect(r)).intersected(exposed);
        if (!toFill.isEmpty())
            painter->fillRect(toFill, color);
    }
}

void OrthogonalRenderer::drawMapObject(QPainter *painter,
                                       const MapObject *object,
                                       const QColor &color) const
{
    painter->save();

    const QRectF bounds = object->bounds();
    QRectF rect(tileToPixelCoords(bounds.topLeft()),
                tileToPixelCoords(bounds.bottomRight()));

    painter->translate(rect.topLeft());
    rect.moveTopLeft(QPointF(0, 0));

    if (object->tile())
    {
        const QPixmap &img = object->tile()->image();
        const QPoint paintOrigin(0, -img.height());
        painter->drawPixmap(paintOrigin, img);

        QPen pen(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawRect(QRect(paintOrigin, img.size()));
        pen.setStyle(Qt::DotLine);
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawRect(QRect(paintOrigin, img.size()));
    }
    else
    {
        if (rect.isNull())
            rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

        const QFontMetrics fm = painter->fontMetrics();
        QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                     rect.width() + 2);

        painter->setRenderHint(QPainter::Antialiasing);

        // Draw the shadow
        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->drawRect(rect.translated(QPointF(1, 1)));
        if (!name.isEmpty())
            painter->drawText(QPoint(1, -5 + 1), name);

        QColor brushColor = color;
        brushColor.setAlpha(50);
        QBrush brush(brushColor);

        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRect(rect);
        if (!name.isEmpty())
            painter->drawText(QPoint(0, -5), name);
    }

    painter->restore();
}

QPointF OrthogonalRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    return QPointF(x / map()->tileWidth(),
                   y / map()->tileHeight());
}

QPointF OrthogonalRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x * map()->tileWidth(),
                   y * map()->tileHeight());
}