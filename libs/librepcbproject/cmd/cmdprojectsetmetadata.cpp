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
#include "cmdprojectsetmetadata.h"
#include "../project.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdProjectSetMetadata::CmdProjectSetMetadata(Project& project) noexcept :
    UndoCommand(tr("Change Project Metadata")),
    mProject(project),
    mOldName(mProject.getName()),               mNewName(mProject.getName()),
    mOldDescription(mProject.getDescription()), mNewDescription(mProject.getDescription()),
    mOldAuthor(mProject.getAuthor()),           mNewAuthor(mProject.getAuthor()),
    mOldCreated(mProject.getCreated()),         mNewCreated(mProject.getCreated())
{
}

CmdProjectSetMetadata::~CmdProjectSetMetadata() noexcept
{
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void CmdProjectSetMetadata::setName(const QString& newName) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewName = newName;
}

void CmdProjectSetMetadata::setDescription(const QString& newDescription) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewDescription = newDescription;
}

void CmdProjectSetMetadata::setAuthor(const QString& newAuthor) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewAuthor = newAuthor;
}

void CmdProjectSetMetadata::setCreated(const QDateTime& newCreated) noexcept
{
    Q_ASSERT(!wasEverExecuted());
    mNewCreated = newCreated;
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdProjectSetMetadata::performExecute() throw (Exception)
{
    performRedo(); // can throw

    return true; // TODO: determine if the metadata was really modified
}

void CmdProjectSetMetadata::performUndo() throw (Exception)
{
    mProject.setName(mOldName);
    mProject.setDescription(mOldDescription);
    mProject.setAuthor(mOldAuthor);
    mProject.setCreated(mOldCreated);
}

void CmdProjectSetMetadata::performRedo() throw (Exception)
{
    mProject.setName(mNewName);
    mProject.setDescription(mNewDescription);
    mProject.setAuthor(mNewAuthor);
    mProject.setCreated(mNewCreated);
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
