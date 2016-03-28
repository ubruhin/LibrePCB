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

#ifndef LIBREPCB_PROJECT_CMDADDDEVICETOBOARD_H
#define LIBREPCB_PROJECT_CMDADDDEVICETOBOARD_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcbcommon/undocommandgroup.h>
#include <librepcbcommon/uuid.h>
#include <librepcbcommon/units/all_length_units.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

namespace workspace {
class Workspace;
}

namespace library {
class Device;
}

namespace project {

class Board;
class ComponentInstance;
class BI_Device;
class CmdDeviceInstanceAdd;

/*****************************************************************************************
 *  Class CmdAddDeviceToBoard
 ****************************************************************************************/

/**
 * @brief The CmdAddDeviceToBoard class
 */
class CmdAddDeviceToBoard final : public UndoCommandGroup
{
    public:

        // Constructors / Destructor
        CmdAddDeviceToBoard(workspace::Workspace& workspace, Board& board,
                            ComponentInstance& cmpInstance, const Uuid& deviceUuid,
                            const Uuid& footprintUuid, const Point& position = Point(),
                            const Angle& rotation = Angle(), bool mirror = false) noexcept;
        ~CmdAddDeviceToBoard() noexcept;

        // Getters
        BI_Device* getDeviceInstance() const noexcept;


    private:

        // Private Methods

        /// @copydoc UndoCommand::performExecute()
        bool performExecute() throw (Exception) override;


        // Private Member Variables

        // Attributes from the constructor
        workspace::Workspace& mWorkspace;
        Board& mBoard;
        ComponentInstance& mComponentInstance;
        Uuid mDeviceUuid;
        Uuid mFootprintUuid;
        Point mPosition;
        Angle mRotation;
        bool mMirror;

        // child commands
        CmdDeviceInstanceAdd* mCmdAddToBoard;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb

#endif // LIBREPCB_PROJECT_CMDADDDEVICETOBOARD_H
