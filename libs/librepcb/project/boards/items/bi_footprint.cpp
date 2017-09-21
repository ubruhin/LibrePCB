/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
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
#include "bi_footprint.h"
#include "bi_footprintpad.h"
#include "../board.h"
#include "../../project.h"
#include "../../circuit/circuit.h"
#include "../../library/projectlibrary.h"
#include <librepcb/library/pkg/footprint.h>
#include <librepcb/library/pkg/package.h>
#include <librepcb/library/dev/device.h>
#include <librepcb/common/graphics/graphicsscene.h>
#include <librepcb/common/scopeguardlist.h>
#include "bi_device.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

BI_Footprint::BI_Footprint(BI_Device& device, const BI_Footprint& other) :
    BI_Base(device.getBoard()), mDevice(device)
{
    Q_UNUSED(other);
    init();
}

BI_Footprint::BI_Footprint(BI_Device& device, const DomElement& domElement) :
    BI_Base(device.getBoard()), mDevice(device)
{
    Q_UNUSED(domElement);
    init();
}

BI_Footprint::BI_Footprint(BI_Device& device) :
    BI_Base(device.getBoard()), mDevice(device)
{
    init();
}

void BI_Footprint::init()
{
    mGraphicsItem.reset(new BGI_Footprint(*this));
    mGraphicsItem->setPos(mDevice.getPosition().toPxQPointF());
    updateGraphicsItemTransform();

    const library::Device& libDev = mDevice.getLibDevice();
    for (const library::FootprintPad& libPad : getLibFootprint().getPads()) {
        BI_FootprintPad* pad = new BI_FootprintPad(*this, libPad.getPackagePadUuid());
        if (mPads.contains(libPad.getPackagePadUuid())) {
            throw RuntimeError(__FILE__, __LINE__,
                QString(tr("The footprint pad UUID \"%1\" is defined multiple times."))
                .arg(libPad.getPackagePadUuid().toStr()));
        }
        if (!libDev.getPadSignalMap().contains(libPad.getPackagePadUuid())) {
            throw RuntimeError(__FILE__, __LINE__,
                QString(tr("Footprint pad \"%1\" not found in pad-signal-map of device \"%2\"."))
                .arg(libPad.getPackagePadUuid().toStr(), libDev.getUuid().toStr()));
        }
        mPads.insert(libPad.getPackagePadUuid(), pad);
    }

    // connect to the "attributes changed" signal of device instance
    connect(&mDevice, &BI_Device::attributesChanged,
            this, &BI_Footprint::deviceInstanceAttributesChanged);
    connect(&mDevice, &BI_Device::moved,
            this, &BI_Footprint::deviceInstanceMoved);
    connect(&mDevice, &BI_Device::rotated,
            this, &BI_Footprint::deviceInstanceRotated);
    connect(&mDevice, &BI_Device::mirrored,
            this, &BI_Footprint::deviceInstanceMirrored);

    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);
}

BI_Footprint::~BI_Footprint() noexcept
{
    qDeleteAll(mPads);              mPads.clear();
    mGraphicsItem.reset();
}

/*****************************************************************************************
 *  Getters
 ****************************************************************************************/

const Uuid& BI_Footprint::getComponentInstanceUuid() const noexcept
{
    return mDevice.getComponentInstanceUuid();
}

const library::Footprint& BI_Footprint::getLibFootprint() const noexcept
{
    return mDevice.getLibFootprint();
}

const Angle& BI_Footprint::getRotation() const noexcept
{
    return mDevice.getRotation();
}

bool BI_Footprint::isUsed() const noexcept
{
    foreach (const BI_FootprintPad* pad, mPads) {
        if (pad->isUsed()) return true;
    }
    return false;
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void BI_Footprint::addToBoard(GraphicsScene& scene)
{
    if (isAddedToBoard()) {
        throw LogicError(__FILE__, __LINE__);
    }
    ScopeGuardList sgl(mPads.count());
    foreach (BI_FootprintPad* pad, mPads) {
        pad->addToBoard(scene); // can throw
        sgl.add([pad, &scene](){pad->removeFromBoard(scene);});
    }
    BI_Base::addToBoard(scene, *mGraphicsItem);
    sgl.dismiss();
}

void BI_Footprint::removeFromBoard(GraphicsScene& scene)
{
    if (!isAddedToBoard()) {
        throw LogicError(__FILE__, __LINE__);
    }
    ScopeGuardList sgl(mPads.count());
    foreach (BI_FootprintPad* pad, mPads) {
        pad->removeFromBoard(scene); // can throw
        sgl.add([pad, &scene](){pad->addToBoard(scene);});
    }
    BI_Base::removeFromBoard(scene, *mGraphicsItem);
    sgl.dismiss();
}

void BI_Footprint::serialize(DomElement& root) const
{
    if (!checkAttributesValidity()) throw LogicError(__FILE__, __LINE__);

    Q_UNUSED(root);
    //root.setAttribute("uuid", mUuid);
    //root.setAttribute("gen_comp_instance", mComponentInstance->getUuid());
    //root.setAttribute("symbol_item", mSymbVarItem->getUuid());
}

/*****************************************************************************************
 *  Helper Methods
 ****************************************************************************************/

Point BI_Footprint::mapToScene(const Point& relativePos) const noexcept
{
    if (mDevice.getIsMirrored()) {
        return (mDevice.getPosition() + relativePos)
                .rotated(mDevice.getRotation(), mDevice.getPosition())
                .mirrored(Qt::Horizontal, mDevice.getPosition());
    } else {
        return (mDevice.getPosition() + relativePos)
                .rotated(mDevice.getRotation(), mDevice.getPosition());
    }
}

/*****************************************************************************************
 *  Inherited from AttributeProvider
 ****************************************************************************************/

QVector<const AttributeProvider*> BI_Footprint::getAttributeProviderParents() const noexcept
{
    return QVector<const AttributeProvider*>{&mDevice};
}

/*****************************************************************************************
 *  Inherited from BI_Base
 ****************************************************************************************/

const Point& BI_Footprint::getPosition() const noexcept
{
    return mDevice.getPosition();
}

bool BI_Footprint::getIsMirrored() const noexcept
{
    return mDevice.getIsMirrored();
}

QPainterPath BI_Footprint::getGrabAreaScenePx() const noexcept
{
    return mGraphicsItem->sceneTransform().map(mGraphicsItem->shape());
}

bool BI_Footprint::isSelectable() const noexcept
{
    return mGraphicsItem->isSelectable();
}

void BI_Footprint::setSelected(bool selected) noexcept
{
    BI_Base::setSelected(selected);
    mGraphicsItem->update();
    foreach (BI_FootprintPad* pad, mPads)
        pad->setSelected(selected);
}

/*****************************************************************************************
 *  Private Slots
 ****************************************************************************************/

void BI_Footprint::deviceInstanceAttributesChanged()
{
    mGraphicsItem->updateCacheAndRepaint();
    emit attributesChanged();
}

void BI_Footprint::deviceInstanceMoved(const Point& pos)
{
    mGraphicsItem->setPos(pos.toPxQPointF());
    mGraphicsItem->updateCacheAndRepaint();
    foreach (BI_FootprintPad* pad, mPads) {
        pad->updatePosition();
    }
}

void BI_Footprint::deviceInstanceRotated(const Angle& rot)
{
    Q_UNUSED(rot);
    updateGraphicsItemTransform();
    mGraphicsItem->updateCacheAndRepaint();
    foreach (BI_FootprintPad* pad, mPads) {
        pad->updatePosition();
    }
}

void BI_Footprint::deviceInstanceMirrored(bool mirrored)
{
    Q_UNUSED(mirrored);
    updateGraphicsItemTransform();
    mGraphicsItem->updateCacheAndRepaint();
    foreach (BI_FootprintPad* pad, mPads) {
        pad->updatePosition();
    }
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void BI_Footprint::updateGraphicsItemTransform() noexcept
{
    QTransform t;
    if (mDevice.getIsMirrored()) t.scale(qreal(-1), qreal(1));
    t.rotate(-mDevice.getRotation().toDeg());
    mGraphicsItem->setTransform(t);
}

bool BI_Footprint::checkAttributesValidity() const noexcept
{
    //if (mUuid.isNull())                 return false;
    //if (mComponentInstance == nullptr)    return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
