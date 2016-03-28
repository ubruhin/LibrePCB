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
#include "bgi_base.h"
#include <librepcbcommon/boardlayer.h>
#include "../board.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

BGI_Base::BGI_Base() noexcept
{

}

BGI_Base::~BGI_Base() noexcept
{

}

/*****************************************************************************************
 *  Protected Methods
 ****************************************************************************************/

qreal BGI_Base::getZValueOfCopperLayer(int layerId) noexcept
{
    if (BoardLayer::isCopperLayer(layerId)) {
        // 0.0 => TOP
        // 1.0 => BOTTOM
        qreal valueNormalized = qreal(layerId - BoardLayer::LayerID::TopCopper) /
                                qreal(BoardLayer::LayerID::BottomCopper - BoardLayer::LayerID::TopCopper);
        return (Board::ItemZValue::ZValue_CopperTop - valueNormalized);
    } else {
        return Board::ItemZValue::ZValue_Default;
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
