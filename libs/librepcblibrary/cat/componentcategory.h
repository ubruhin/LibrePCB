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

#ifndef LIBREPCB_LIBRARY_COMPONENTCATEGORY_H
#define LIBREPCB_LIBRARY_COMPONENTCATEGORY_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "librarycategory.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Class ComponentCategory
 ****************************************************************************************/

/**
 * @brief The ComponentCategory class
 */
class ComponentCategory final : public LibraryCategory
{
        Q_OBJECT

    public:

        explicit ComponentCategory(const FilePath& elementDirectory, bool readOnly);
        virtual ~ComponentCategory();

    private:

        // make some methods inaccessible...
        ComponentCategory();
        ComponentCategory(const ComponentCategory& other);
        ComponentCategory& operator=(const ComponentCategory& rhs);


        // Private Methods
        void parseDomTree(const XmlDomElement& root) throw (Exception);

};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb

#endif // LIBREPCB_LIBRARY_COMPONENTCATEGORY_H
