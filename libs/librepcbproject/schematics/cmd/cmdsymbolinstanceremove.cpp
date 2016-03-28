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
#include "cmdsymbolinstanceremove.h"
#include "../schematic.h"
#include "../items/si_symbol.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdSymbolInstanceRemove::CmdSymbolInstanceRemove(Schematic& schematic, SI_Symbol& symbol) noexcept :
    UndoCommand(tr("Remove symbol")),
    mSchematic(schematic), mSymbol(symbol)
{
}

CmdSymbolInstanceRemove::~CmdSymbolInstanceRemove() noexcept
{
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdSymbolInstanceRemove::performExecute() throw (Exception)
{
    performRedo(); // can throw

    return true;
}

void CmdSymbolInstanceRemove::performUndo() throw (Exception)
{
    mSchematic.addSymbol(mSymbol); // can throw
}

void CmdSymbolInstanceRemove::performRedo() throw (Exception)
{
    mSchematic.removeSymbol(mSymbol); // can throw
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
