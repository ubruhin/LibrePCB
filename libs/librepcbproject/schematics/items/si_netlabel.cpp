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
#include "si_netlabel.h"
#include "../schematic.h"
#include "../../circuit/netsignal.h"
#include "../../circuit/circuit.h"
#include "../../project.h"
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

SI_NetLabel::SI_NetLabel(Schematic& schematic, const XmlDomElement& domElement) throw (Exception) :
    SI_Base(schematic), mUuid(), mPosition(), mRotation(), mNetSignal(nullptr)
{
    // read attributes
    mUuid = domElement.getAttribute<Uuid>("uuid", true);
    Uuid netSignalUuid = domElement.getAttribute<Uuid>("netsignal", true);
    mNetSignal = mSchematic.getProject().getCircuit().getNetSignalByUuid(netSignalUuid);
    if(!mNetSignal) {
        throw RuntimeError(__FILE__, __LINE__, netSignalUuid.toStr(),
            QString(tr("Invalid net signal UUID: \"%1\"")).arg(netSignalUuid.toStr()));
    }
    mPosition.setX(domElement.getAttribute<Length>("x", true));
    mPosition.setY(domElement.getAttribute<Length>("y", true));
    mRotation = domElement.getAttribute<Angle>("rotation", true);

    init();
}

SI_NetLabel::SI_NetLabel(Schematic& schematic, NetSignal& netsignal, const Point& position) throw (Exception) :
    SI_Base(schematic), mUuid(Uuid::createRandom()), mPosition(position), mRotation(0),
    mNetSignal(&netsignal)
{
    init();
}

void SI_NetLabel::init() throw (Exception)
{
    connect(mNetSignal, &NetSignal::nameChanged, this, &SI_NetLabel::netSignalNameChanged);

    // create the graphics item
    mGraphicsItem.reset(new SGI_NetLabel(*this));
    mGraphicsItem->setPos(mPosition.toPxQPointF());
    mGraphicsItem->setRotation(-mRotation.toDeg());

    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);
}

SI_NetLabel::~SI_NetLabel() noexcept
{
    mGraphicsItem.reset();
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void SI_NetLabel::setNetSignal(NetSignal& netsignal) noexcept
{
    if (&netsignal != mNetSignal) {
        if (netsignal.getCircuit() != getCircuit()) {
            throw LogicError(__FILE__, __LINE__);
        }
        if (isAddedToSchematic()) {
            mNetSignal->unregisterSchematicNetLabel(*this); // can throw
            auto sg = scopeGuard([&](){mNetSignal->registerSchematicNetLabel(*this);});
            netsignal.registerSchematicNetLabel(*this); // can throw
            sg.dismiss();
        }
        disconnect(mNetSignal, &NetSignal::nameChanged, this, &SI_NetLabel::netSignalNameChanged);
        connect(&netsignal, &NetSignal::nameChanged, this, &SI_NetLabel::netSignalNameChanged);
        mNetSignal = &netsignal;
        mGraphicsItem->updateCacheAndRepaint();
    }
}

void SI_NetLabel::setPosition(const Point& position) noexcept
{
    if (position != mPosition) {
        mPosition = position;
        mGraphicsItem->setPos(mPosition.toPxQPointF());
    }
}

void SI_NetLabel::setRotation(const Angle& rotation) noexcept
{
    if (rotation != mRotation) {
        mRotation = rotation;
        mGraphicsItem->setRotation(-mRotation.toDeg());
        mGraphicsItem->updateCacheAndRepaint();
    }
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void SI_NetLabel::addToSchematic(GraphicsScene& scene) throw (Exception)
{
    if (isAddedToSchematic()) {
        throw LogicError(__FILE__, __LINE__);
    }
    mNetSignal->registerSchematicNetLabel(*this); // can throw
    mHighlightChangedConnection = connect(mNetSignal, &NetSignal::highlightedChanged,
                                          [this](){mGraphicsItem->update();});
    SI_Base::addToSchematic(scene, *mGraphicsItem);
}

void SI_NetLabel::removeFromSchematic(GraphicsScene& scene) throw (Exception)
{
    if (!isAddedToSchematic()) {
        throw LogicError(__FILE__, __LINE__);
    }
    mNetSignal->unregisterSchematicNetLabel(*this); // can throw
    disconnect(mHighlightChangedConnection);
    SI_Base::removeFromSchematic(scene, *mGraphicsItem);
}

XmlDomElement* SI_NetLabel::serializeToXmlDomElement() const throw (Exception)
{
    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);

    QScopedPointer<XmlDomElement> root(new XmlDomElement("netlabel"));
    root->setAttribute("uuid", mUuid);
    root->setAttribute("x", mPosition.getX());
    root->setAttribute("y", mPosition.getY());
    root->setAttribute("rotation", mRotation);
    root->setAttribute("netsignal", mNetSignal->getUuid());
    return root.take();
}

/*****************************************************************************************
 *  Inherited from SI_Base
 ****************************************************************************************/

QPainterPath SI_NetLabel::getGrabAreaScenePx() const noexcept
{
    return mGraphicsItem->sceneTransform().map(mGraphicsItem->shape());
}

void SI_NetLabel::setSelected(bool selected) noexcept
{
    SI_Base::setSelected(selected);
    mGraphicsItem->update();
}

/*****************************************************************************************
 *  Private Slots
 ****************************************************************************************/

void SI_NetLabel::netSignalNameChanged(const QString& newName) noexcept
{
    Q_UNUSED(newName);
    mGraphicsItem->updateCacheAndRepaint();
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool SI_NetLabel::checkAttributesValidity() const noexcept
{
    if (mUuid.isNull())                             return false;
    if (mNetSignal == nullptr)                      return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
