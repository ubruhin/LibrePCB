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
#include "cmdremoveunusednetsignals.h"
#include <librepcbproject/circuit/circuit.h>
#include <librepcbproject/circuit/netsignal.h>
#include <librepcbproject/circuit/cmd/cmdnetsignalremove.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdRemoveUnusedNetSignals::CmdRemoveUnusedNetSignals(Circuit& circuit) noexcept :
    UndoCommandGroup(tr("Remove Unused Net Signals")), mCircuit(circuit)
{
}

CmdRemoveUnusedNetSignals::~CmdRemoveUnusedNetSignals() noexcept
{
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdRemoveUnusedNetSignals::performExecute() throw (Exception)
{
    foreach (NetSignal* netsignal, mCircuit.getNetSignals()) {
        if (!netsignal->isUsed()) {
            appendChild(new CmdNetSignalRemove(mCircuit, *netsignal));
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
