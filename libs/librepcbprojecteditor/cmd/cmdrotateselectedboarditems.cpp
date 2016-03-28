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
#include "cmdrotateselectedboarditems.h"
#include <librepcbcommon/gridproperties.h>
#include <librepcbproject/project.h>
#include <librepcbproject/boards/board.h>
#include <librepcbproject/boards/items/bi_device.h>
#include <librepcbproject/boards/items/bi_footprint.h>
#include <librepcbproject/boards/items/bi_netpoint.h>
#include <librepcbproject/boards/items/bi_via.h>
#include <librepcbproject/boards/cmd/cmddeviceinstanceedit.h>
#include <librepcbproject/boards/cmd/cmdboardviaedit.h>
#include <librepcbproject/boards/cmd/cmdboardnetpointedit.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdRotateSelectedBoardItems::CmdRotateSelectedBoardItems(Board& board, const Angle& angle) noexcept :
    UndoCommandGroup(tr("Rotate Board Elements")), mBoard(board), mAngle(angle)
{
}

CmdRotateSelectedBoardItems::~CmdRotateSelectedBoardItems() noexcept
{
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdRotateSelectedBoardItems::performExecute() throw (Exception)
{
    // get all selected items
    QList<BI_Base*> items = mBoard.getSelectedItems(true, false, true, false, true, false,
                                                    false, false, false, false, false, false);

    // no items selected --> nothing to do here
    if (items.isEmpty()) {
        return false;
    }

    // find the center of all elements
    Point center = Point(0, 0);
    foreach (BI_Base* item, items) {
        center += item->getPosition();
    }
    center /= items.count();
    center.mapToGrid(mBoard.getGridProperties().getInterval());

    // rotate all selected elements
    foreach (BI_Base* item, items) {
        switch (item->getType())
        {
            case BI_Base::Type_t::Footprint: {
                BI_Footprint* footprint = dynamic_cast<BI_Footprint*>(item); Q_ASSERT(footprint);
                BI_Device& device = footprint->getDeviceInstance();
                CmdDeviceInstanceEdit* cmd = new CmdDeviceInstanceEdit(device);
                cmd->rotate(mAngle, center, false);
                appendChild(cmd);
                break;
            }
            case BI_Base::Type_t::Via: {
                BI_Via* via = dynamic_cast<BI_Via*>(item); Q_ASSERT(via);
                CmdBoardViaEdit* cmd = new CmdBoardViaEdit(*via);
                cmd->setPosition(via->getPosition().rotated(mAngle, center), false);
                appendChild(cmd);
                break;
            }
            case BI_Base::Type_t::NetPoint: {
                BI_NetPoint* point = dynamic_cast<BI_NetPoint*>(item); Q_ASSERT(point);
                CmdBoardNetPointEdit* cmd = new CmdBoardNetPointEdit(*point);
                cmd->setPosition(point->getPosition().rotated(mAngle, center), false);
                appendChild(cmd);
                break;
            }
            default: {
                qCritical() << "Unknown board item type:" << static_cast<int>(item->getType());
                break;
            }
        }
    }

    // execute all child commands
    return UndoCommandGroup::performExecute(); // can throw
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
