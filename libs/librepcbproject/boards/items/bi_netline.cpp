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
#include "bi_netline.h"
#include "../board.h"
#include "bi_netpoint.h"
#include "../../project.h"
#include "../../circuit/netsignal.h"
#include "bi_footprint.h"
#include "bi_footprintpad.h"
#include <librepcbcommon/fileio/xmldomelement.h>
#include <librepcbcommon/graphics/graphicsscene.h>
#include <librepcbcommon/scopeguard.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

BI_NetLine::BI_NetLine(Board& board, const BI_NetLine& other, BI_NetPoint& startPoint,
                       BI_NetPoint& endPoint) throw (Exception) :
    BI_Base(board), mPosition(other.mPosition), mUuid(Uuid::createRandom()),
    mStartPoint(&startPoint), mEndPoint(&endPoint), mWidth(other.mWidth)
{
    init();
}

BI_NetLine::BI_NetLine(Board& board, const XmlDomElement& domElement) throw (Exception) :
    BI_Base(board), mPosition(), mUuid(), mStartPoint(nullptr), mEndPoint(nullptr),
    mWidth()
{
    mUuid = domElement.getAttribute<Uuid>("uuid", true);
    mWidth = domElement.getAttribute<Length>("width", true);

    Uuid spUuid = domElement.getAttribute<Uuid>("start_point", true);
    mStartPoint = mBoard.getNetPointByUuid(spUuid);
    if(!mStartPoint) {
        throw RuntimeError(__FILE__, __LINE__, spUuid.toStr(),
            QString(tr("Invalid net point UUID: \"%1\""))
            .arg(spUuid.toStr()));
    }

    Uuid epUuid = domElement.getAttribute<Uuid>("end_point", true);
    mEndPoint = mBoard.getNetPointByUuid(epUuid);
    if(!mEndPoint) {
        throw RuntimeError(__FILE__, __LINE__, epUuid.toStr(),
            QString(tr("Invalid net point UUID: \"%1\""))
            .arg(epUuid.toStr()));
    }

    init();
}

BI_NetLine::BI_NetLine(Board& board, BI_NetPoint& startPoint, BI_NetPoint& endPoint,
                       const Length& width) throw (Exception) :
    BI_Base(board), mPosition(), mUuid(Uuid::createRandom()), mStartPoint(&startPoint),
    mEndPoint(&endPoint), mWidth(width)
{
    init();
}

void BI_NetLine::init() throw (Exception)
{
    if(mWidth < 0) {
        throw RuntimeError(__FILE__, __LINE__, mWidth.toMmString(),
            QString(tr("Invalid trace width: \"%1\"")).arg(mWidth.toMmString()));
    }

    // check if both netpoints have the same netsignal
    if (&mStartPoint->getNetSignal() != &mEndPoint->getNetSignal()) {
        throw LogicError(__FILE__, __LINE__, QString(),
            tr("BI_NetLine: endpoints netsignal mismatch."));
    }

    // check if both netpoints have the same layer
    if (&mStartPoint->getLayer() != &mEndPoint->getLayer()) {
        throw LogicError(__FILE__, __LINE__, QString(),
            tr("BI_NetLine: endpoints layer mismatch."));
    }

    // check if both netpoints are different
    if (mStartPoint == mEndPoint) {
        throw LogicError(__FILE__, __LINE__, QString(),
            tr("BI_NetLine: both endpoints are the same."));
    }

    mGraphicsItem.reset(new BGI_NetLine(*this));
    updateLine();

    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);
}

BI_NetLine::~BI_NetLine() noexcept
{
    mGraphicsItem.reset();
}

/*****************************************************************************************
 *  Getters
 ****************************************************************************************/

BoardLayer& BI_NetLine::getLayer() const noexcept
{
    Q_ASSERT(&mStartPoint->getLayer() == &mEndPoint->getLayer());
    return mStartPoint->getLayer();
}

NetSignal& BI_NetLine::getNetSignal() const noexcept
{
    Q_ASSERT(&mStartPoint->getNetSignal() == &mEndPoint->getNetSignal());
    return mStartPoint->getNetSignal();
}

bool BI_NetLine::isAttached() const noexcept
{
    return (mStartPoint->isAttached() || mEndPoint->isAttached());
}

bool BI_NetLine::isAttachedToVia() const noexcept
{
    return (mStartPoint->isAttachedToVia() || mEndPoint->isAttachedToVia());
}

bool BI_NetLine::isAttachedToFootprint() const noexcept
{
    return (mStartPoint->isAttachedToPad() || mEndPoint->isAttachedToPad());
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void BI_NetLine::setWidth(const Length& width) noexcept
{
    Q_ASSERT(width >= 0);
    if ((width != mWidth) && (width >= 0)) {
        mWidth = width;
        mGraphicsItem->updateCacheAndRepaint();
    }
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void BI_NetLine::addToBoard(GraphicsScene& scene) throw (Exception)
{
    if (isAddedToBoard()
        || (&mStartPoint->getNetSignal() != &mEndPoint->getNetSignal())
        || (&mStartPoint->getLayer() != &mEndPoint->getLayer()))
    {
        throw LogicError(__FILE__, __LINE__);
    }
    mStartPoint->registerNetLine(*this); // can throw
    auto sg = scopeGuard([&](){mStartPoint->unregisterNetLine(*this);});
    mEndPoint->registerNetLine(*this); // can throw
    BI_Base::addToBoard(scene, *mGraphicsItem);
    sg.dismiss();
}

void BI_NetLine::removeFromBoard(GraphicsScene& scene) throw (Exception)
{
    if ((!isAddedToBoard())
        || (&mStartPoint->getNetSignal() != &mEndPoint->getNetSignal())
        || (&mStartPoint->getLayer() != &mEndPoint->getLayer()))
    {
        throw LogicError(__FILE__, __LINE__);
    }
    mStartPoint->unregisterNetLine(*this); // can throw
    auto sg = scopeGuard([&](){mEndPoint->registerNetLine(*this);});
    mEndPoint->unregisterNetLine(*this); // can throw
    BI_Base::removeFromBoard(scene, *mGraphicsItem);
    sg.dismiss();
}

void BI_NetLine::updateLine() noexcept
{
    mPosition = (mStartPoint->getPosition() + mEndPoint->getPosition()) / 2;
    mGraphicsItem->updateCacheAndRepaint();
}

XmlDomElement* BI_NetLine::serializeToXmlDomElement() const throw (Exception)
{
    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);

    QScopedPointer<XmlDomElement> root(new XmlDomElement("netline"));
    root->setAttribute("uuid", mUuid);
    root->setAttribute("start_point", mStartPoint->getUuid());
    root->setAttribute("end_point", mEndPoint->getUuid());
    root->setAttribute("width", mWidth);
    return root.take();
}

/*****************************************************************************************
 *  Inherited from BI_Base
 ****************************************************************************************/

QPainterPath BI_NetLine::getGrabAreaScenePx() const noexcept
{
    return mGraphicsItem->shape();
}

bool BI_NetLine::isSelectable() const noexcept
{
    return mGraphicsItem->isSelectable();
}

void BI_NetLine::setSelected(bool selected) noexcept
{
    BI_Base::setSelected(selected);
    mGraphicsItem->update();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool BI_NetLine::checkAttributesValidity() const noexcept
{
    if (mUuid.isNull())         return false;
    if (mStartPoint == nullptr) return false;
    if (mEndPoint == nullptr)   return false;
    if (mWidth < 0)             return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
