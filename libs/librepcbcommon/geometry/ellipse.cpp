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
#include "ellipse.h"
#include "fileio/xmldomelement.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

Ellipse::Ellipse(int layerId, const Length& lineWidth, bool fill, bool isGrabArea,
                 const Point& center, const Length& radiusX, const Length& radiusY,
                 const Angle& rotation) noexcept :
    mLayerId(layerId), mLineWidth(lineWidth), mIsFilled(fill), mIsGrabArea(isGrabArea),
    mCenter(center), mRadiusX(radiusX), mRadiusY(radiusY), mRotation(rotation)
{
    Q_ASSERT(layerId >= 0);
    Q_ASSERT(lineWidth >= 0);
    Q_ASSERT(radiusX > 0);
    Q_ASSERT(radiusY > 0);
}

Ellipse::Ellipse(const XmlDomElement& domElement) throw (Exception)
{
    mLayerId = domElement.getAttribute<uint>("layer", true); // use "uint" to automatically check for >= 0
    mLineWidth = domElement.getAttribute<Length>("width", true);
    mIsFilled = domElement.getAttribute<bool>("fill", true);
    mIsGrabArea = domElement.getAttribute<bool>("grab_area", true);
    mCenter.setX(domElement.getAttribute<Length>("x", true));
    mCenter.setY(domElement.getAttribute<Length>("y", true));
    mRadiusX = domElement.getAttribute<Length>("radius_x", true);
    mRadiusY = domElement.getAttribute<Length>("radius_y", true);
    mRotation = domElement.getAttribute<Angle>("rotation", true);

    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);
}

Ellipse::~Ellipse() noexcept
{
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void Ellipse::setLayerId(int id) noexcept
{
    Q_ASSERT(id >= 0);
    mLayerId = id;
}

void Ellipse::setLineWidth(const Length& width) noexcept
{
    Q_ASSERT(width >= 0);
    mLineWidth = width;
}

void Ellipse::setIsFilled(bool isFilled) noexcept
{
    mIsFilled = isFilled;
}

void Ellipse::setIsGrabArea(bool isGrabArea) noexcept
{
    mIsGrabArea = isGrabArea;
}

void Ellipse::setCenter(const Point& center) noexcept
{
    mCenter = center;
}

void Ellipse::setRadiusX(const Length& radius) noexcept
{
    Q_ASSERT(radius > 0);
    mRadiusX = radius;
}

void Ellipse::setRadiusY(const Length& radius) noexcept
{
    Q_ASSERT(radius > 0);
    mRadiusY = radius;
}

void Ellipse::setRotation(const Angle& rotation) noexcept
{
    mRotation = rotation;
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

XmlDomElement* Ellipse::serializeToXmlDomElement() const throw (Exception)
{
    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);

    QScopedPointer<XmlDomElement> root(new XmlDomElement("ellipse"));
    root->setAttribute("layer", mLayerId);
    root->setAttribute("width", mLineWidth);
    root->setAttribute("fill", mIsFilled);
    root->setAttribute("grab_area", mIsGrabArea);
    root->setAttribute("x", mCenter.getX());
    root->setAttribute("y", mCenter.getY());
    root->setAttribute("radius_x", mRadiusX);
    root->setAttribute("radius_y", mRadiusY);
    root->setAttribute("rotation", mRotation);
    return root.take();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool Ellipse::checkAttributesValidity() const noexcept
{
    if (mLayerId <= 0)          return false;
    if (mLineWidth < 0)         return false;
    if (mRadiusX <= 0)          return false;
    if (mRadiusY <= 0)          return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
