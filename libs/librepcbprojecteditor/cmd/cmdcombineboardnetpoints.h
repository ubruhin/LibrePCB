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

#ifndef LIBREPCB_PROJECT_CMDCOMBINEBOARDNETPOINTS_H
#define LIBREPCB_PROJECT_CMDCOMBINEBOARDNETPOINTS_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcbcommon/undocommandgroup.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {
namespace project {

class BI_NetPoint;

/*****************************************************************************************
 *  Class CmdCombineBoardNetPoints
 ****************************************************************************************/

/**
 * @brief The CmdCombineBoardNetPoints class
 */
class CmdCombineBoardNetPoints final : public UndoCommandGroup
{
    public:

        // Constructors / Destructor
        CmdCombineBoardNetPoints(BI_NetPoint& toBeRemoved, BI_NetPoint& result) noexcept;
        ~CmdCombineBoardNetPoints() noexcept;


    private:

        // Private Methods

        /// @copydoc UndoCommand::performExecute()
        bool performExecute() throw (Exception) override;


        // Attributes from the constructor
        BI_NetPoint& mNetPointToBeRemoved;
        BI_NetPoint& mResultingNetPoint;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb

#endif // LIBREPCB_PROJECT_CMDCOMBINEBOARDNETPOINTS_H
