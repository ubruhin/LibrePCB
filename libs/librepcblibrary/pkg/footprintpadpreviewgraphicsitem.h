/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 Urban Bruhin
 * http://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBREPCB_LIBRARY_FOOTPRINTPADPREVIEWGRAPHICSITEM_H
#define LIBREPCB_LIBRARY_FOOTPRINTPADPREVIEWGRAPHICSITEM_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <QtWidgets>
#include <librepcbcommon/graphics/graphicsitem.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

class BoardLayer;
class IF_BoardLayerProvider;

namespace library {

class PackagePad;
class FootprintPad;

/*****************************************************************************************
 *  Class FootprintPadPreviewGraphicsItem
 ****************************************************************************************/

/**
 * @brief The FootprintPadPreviewGraphicsItem class
 *
 * @author ubruhin
 * @date 2016-01-09
 */
class FootprintPadPreviewGraphicsItem final : public GraphicsItem
{
    public:

        // Constructors / Destructor
        explicit FootprintPadPreviewGraphicsItem(const IF_BoardLayerProvider& layerProvider,
                                                 const FootprintPad& fptPad,
                                                 const PackagePad* pkgPad = nullptr) noexcept;
        ~FootprintPadPreviewGraphicsItem() noexcept;

        // Setters
        void setDrawBoundingRect(bool enable) noexcept {mDrawBoundingRect = enable;}

        // General Methods
        void updateCacheAndRepaint() noexcept;

        // Inherited from QGraphicsItem
        QRectF boundingRect() const noexcept {return mBoundingRect;}
        QPainterPath shape() const noexcept {return mShape;}
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);


    private:

        // make some methods inaccessible...
        FootprintPadPreviewGraphicsItem() = delete;
        FootprintPadPreviewGraphicsItem(const FootprintPadPreviewGraphicsItem& other) = delete;
        FootprintPadPreviewGraphicsItem& operator=(const FootprintPadPreviewGraphicsItem& rhs) = delete;


        // General Attributes
        const FootprintPad& mFootprintPad;
        const PackagePad* mPackagePad;
        BoardLayer* mLayer;
        bool mDrawBoundingRect;

        // Cached Attributes
        QRectF mBoundingRect;
        QPainterPath mShape;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb

#endif // LIBREPCB_LIBRARY_FOOTPRINTPADPREVIEWGRAPHICSITEM_H
