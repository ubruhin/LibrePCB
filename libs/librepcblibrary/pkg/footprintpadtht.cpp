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

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcbcommon/fileio/xmldomelement.h>
#include <librepcbcommon/boardlayer.h>
#include "footprintpadtht.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

FootprintPadTht::FootprintPadTht(const Uuid& padUuid, const Point& pos, const Angle& rot,
                                 const Length& width, const Length& height,
                                 Shape_t shape, const Length& drillDiameter) noexcept :
    FootprintPad(Technology_t::THT, padUuid, pos, rot, width, height),
    mShape(shape), mDrillDiameter(drillDiameter)
{
}

FootprintPadTht::FootprintPadTht(const XmlDomElement& domElement) throw (Exception) :
    FootprintPad(domElement)
{
    // read attributes
    mShape = stringToShape(domElement.getAttribute<QString>("shape", true));
    mDrillDiameter = domElement.getAttribute<Length>("drill", true);

    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);
}

FootprintPadTht::~FootprintPadTht() noexcept
{
}

/*****************************************************************************************
 *  Getters
 ****************************************************************************************/

int FootprintPadTht::getLayerId() const noexcept
{
    return BoardLayer::LayerID::ThtPads;
}

bool FootprintPadTht::isOnLayer(int id) const noexcept
{
    return BoardLayer::isCopperLayer(id);
}

const QPainterPath& FootprintPadTht::toQPainterPathPx() const noexcept
{
    if (mPainterPathPx.isEmpty()) {
        mPainterPathPx.setFillRule(Qt::OddEvenFill); // important to subtract the hole!
        QRectF rect = getBoundingRectPx();
        switch (mShape)
        {
            case Shape_t::ROUND: {
                qreal radius = qMin(mWidth.toPx(), mHeight.toPx())/2;
                mPainterPathPx.addRoundedRect(rect, radius, radius);
                break;
            }
            case Shape_t::RECT: {
                mPainterPathPx.addRect(rect);
                break;
            }
            case Shape_t::OCTAGON: {
                qreal rx = mWidth.toPx()/2;
                qreal ry = mHeight.toPx()/2;
                qreal a = qMin(rx, ry) * (2 - qSqrt(2));
                QPolygonF octagon;
                octagon.append(QPointF(rx, ry-a));
                octagon.append(QPointF(rx-a, ry));
                octagon.append(QPointF(a-rx, ry));
                octagon.append(QPointF(-rx, ry-a));
                octagon.append(QPointF(-rx, a-ry));
                octagon.append(QPointF(a-rx, -ry));
                octagon.append(QPointF(rx-a, -ry));
                octagon.append(QPointF(rx, a-ry));
                mPainterPathPx.addPolygon(octagon);
                break;
            }
            default: Q_ASSERT(false); break;
        }
        // remove hole
        mPainterPathPx.addEllipse(QPointF(0, 0), mDrillDiameter.toPx()/2, mDrillDiameter.toPx()/2);
    }
    return mPainterPathPx;
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void FootprintPadTht::setShape(Shape_t shape) noexcept
{
    mShape = shape;
    mPainterPathPx = QPainterPath(); // invalidate painter path
}

void FootprintPadTht::setDrillDiameter(const Length& diameter) noexcept
{
    mDrillDiameter = diameter;
    mPainterPathPx = QPainterPath(); // invalidate painter path
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

XmlDomElement* FootprintPadTht::serializeToXmlDomElement() const throw (Exception)
{
    QScopedPointer<XmlDomElement> root(FootprintPad::serializeToXmlDomElement());
    root->setAttribute("shape", shapeToString(mShape));
    root->setAttribute("drill", mDrillDiameter);
    return root.take();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool FootprintPadTht::checkAttributesValidity() const noexcept
{
    if (!FootprintPad::checkAttributesValidity())   return false;
    if (mDrillDiameter < 0)                         return false;
    return true;
}

/*****************************************************************************************
 *  Static Methods
 ****************************************************************************************/

FootprintPadTht::Shape_t FootprintPadTht::stringToShape(const QString& shape) throw (Exception)
{
    if      (shape == QLatin1String("round"))   return Shape_t::ROUND;
    else if (shape == QLatin1String("rect"))    return Shape_t::RECT;
    else if (shape == QLatin1String("octagon")) return Shape_t::OCTAGON;
    else throw RuntimeError(__FILE__, __LINE__, shape, shape);
}

QString FootprintPadTht::shapeToString(Shape_t shape) noexcept
{
    switch (shape)
    {
        case Shape_t::ROUND:    return QString("round");
        case Shape_t::RECT:     return QString("rect");
        case Shape_t::OCTAGON:  return QString("octagon");
        default: Q_ASSERT(false); return QString();
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb
