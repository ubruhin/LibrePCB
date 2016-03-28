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
#include "cmdboardadd.h"
#include "../board.h"
#include "../../project.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdBoardAdd::CmdBoardAdd(Project& project, const QString& name) noexcept :
    UndoCommand(tr("Add board")),
    mProject(project), mBoardToCopy(nullptr), mName(name), mBoard(nullptr), mPageIndex(-1)
{
}

CmdBoardAdd::CmdBoardAdd(Project& project, const Board& boardToCopy, const QString& name) noexcept :
    UndoCommand(tr("Copy board")),
    mProject(project), mBoardToCopy(&boardToCopy), mName(name), mBoard(nullptr),
    mPageIndex(-1)
{
}

CmdBoardAdd::~CmdBoardAdd() noexcept
{
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdBoardAdd::performExecute() throw (Exception)
{
    if (mBoardToCopy) {
        mBoard = mProject.createBoard(*mBoardToCopy, mName); // can throw
    } else {
        mBoard = mProject.createBoard(mName); // can throw
    }

    performRedo(); // can throw

    return true;
}

void CmdBoardAdd::performUndo() throw (Exception)
{
    mProject.removeBoard(*mBoard); // can throw
}

void CmdBoardAdd::performRedo() throw (Exception)
{
    mProject.addBoard(*mBoard, mPageIndex); // can throw
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
