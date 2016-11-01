/*
 * clipboardmanager.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject>
#include <QRegion>

class QClipboard;
class QUndoStack;

namespace Tiled {

class ObjectGroup;
class Map;
class MapObject;

namespace Internal {

class MapDocument;
class MapView;

/**
 * The clipboard manager deals with interaction with the clipboard.
 */
class ClipboardManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns the clipboard manager instance. Creates the instance when it
     * doesn't exist yet.
     */
    static ClipboardManager *instance();

    /**
     * Deletes the clipboard manager instance if it exists.
     */
    static void deleteInstance();

    /**
     * Returns whether the clipboard has a map.
     */
    bool hasMap() const { return mHasMap; }

    /**
     * Retrieves the map from the clipboard. Returns 0 when there was no map or
     * loading failed.
     */
    Map *map() const;

    /**
     * Sets the given map on the clipboard.
     */
    void setMap(const Map *map);

    /**
     * Convenience method to copy the current selection to the clipboard.
     * Deals with either tile selection or object selection.
     */
    void copySelection(const MapDocument *mapDocument);

    void copySelectionAllLayers(const MapDocument *mapDocument);

    enum PasteMode {
        Standard,
        NoTileObjects,
    };

    /**
     * Convenience method that deals with some of the logic related to pasting
     * a group of objects.
     */
    void pasteObjectGroup(const ObjectGroup *objectGroup,
                          MapDocument *mapDocument,
                          const MapView *view,
                          PasteMode mode = Standard);

    void pasteAllObjectGroups(MapDocument *mapDocument,
                                const MapView *view,
                                PasteMode mode = Standard);

    void setIsCut(bool cut) { mIsCut = cut; }

    void clear();

signals:
    /**
     * Emitted when whether the clip has a map changed.
     */
    void hasMapChanged();

    void copyAreaChanged();

private slots:
    void updateHasMap();
    void restoreConnections(MapDocument *mapDocument);

private:
    ClipboardManager();

    /**
     * Check if the given object is allowd in the current layer
     */
    bool checkCurrentLayer(MapDocument *mapDocument, const MapObject *mapObject);

    Tiled::MapObject *pasteObject(MapDocument *mapDocument, MapObject *mapObject, QPointF insertPos
                    , QUndoStack *undoStack, ObjectGroup *objectGroup, QPointF areaCenter);
    bool pasteInMap(MapDocument *mapDocument, QPointF position);
    ObjectGroup *copyObjectInArea(const MapDocument *mapDocument, QList<MapObject*> objects);
    void noteRelatedObjects(const MapDocument *mapDocument, ObjectGroup *copyObjectLayer);

    Q_DISABLE_COPY(ClipboardManager)

    QClipboard *mClipboard;
    bool mHasMap;

    static ClipboardManager *mInstance;

    QRegion mSelectedArea;
    QList<MapObject *> mPastedObjects;
    QList<MapObject *> mRelatedObjects;
    bool mIsCut;
};

#endif // CLIPBOARDMANAGER_H

} // namespace Internal
} // namespace Tiled
