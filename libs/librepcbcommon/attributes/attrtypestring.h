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

#ifndef LIBREPCB_ATTRTYPESTRING_H
#define LIBREPCB_ATTRTYPESTRING_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "attributetype.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Class AttrTypeString
 ****************************************************************************************/

/**
 * @brief The AttrTypeString class
 */
class AttrTypeString final : public AttributeType
{

    public:

        bool isValueValid(const QString& value) const noexcept;
        QString valueFromTr(const QString& value) const noexcept;
        QString printableValueTr(const QString& value, const AttributeUnit* unit = nullptr) const noexcept;
        static const AttrTypeString& instance() noexcept {static AttrTypeString x; return x;}


    private:

        // make some methods inaccessible...
        AttrTypeString(const AttrTypeString& other) = delete;
        AttrTypeString& operator=(const AttrTypeString& rhs) = delete;


        // Constructors / Destructor
        AttrTypeString() noexcept;
        ~AttrTypeString() noexcept;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb

#endif // LIBREPCB_ATTRTYPESTRING_H
