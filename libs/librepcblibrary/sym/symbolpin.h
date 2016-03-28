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

#ifndef LIBREPCB_LIBRARY_SYMBOLPIN_H
#define LIBREPCB_LIBRARY_SYMBOLPIN_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcbcommon/uuid.h>
#include <librepcbcommon/units/all_length_units.h>
#include <librepcbcommon/fileio/if_xmlserializableobject.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Class SymbolPin
 ****************************************************************************************/

/**
 * @brief The SymbolPin class
 */
class SymbolPin final : public IF_XmlSerializableObject
{
        Q_DECLARE_TR_FUNCTIONS(SymbolPin)

    public:

        // Constructors / Destructor
        explicit SymbolPin(const Uuid& uuid, const QString& name, const Point& position,
                           const Length& length, const Angle& rotation) noexcept;
        explicit SymbolPin(const XmlDomElement& domElement) throw (Exception);
        ~SymbolPin() noexcept;

        // Getters
        const Uuid& getUuid() const noexcept {return mUuid;}
        const QString& getName() const noexcept {return mName;}
        const Point& getPosition() const noexcept {return mPosition;}
        const Length& getLength() const noexcept {return mLength;}
        const Angle& getRotation() const noexcept {return mRotation;}

        // Setters
        void setPosition(const Point& pos) noexcept;
        void setLength(const Length& length) noexcept;
        void setRotation(const Angle& rotation) noexcept;
        void setName(const QString& name) noexcept;

        // General Methods

        /// @copydoc IF_XmlSerializableObject#serializeToXmlDomElement()
        XmlDomElement* serializeToXmlDomElement() const throw (Exception) override;

    private:

        // make some methods inaccessible...
        SymbolPin() = delete;
        SymbolPin(const SymbolPin& other) = delete;
        SymbolPin& operator=(const SymbolPin& rhs) = delete;

        // Private Methods

        /// @copydoc IF_XmlSerializableObject#checkAttributesValidity()
        bool checkAttributesValidity() const noexcept override;


        // Pin Attributes
        Uuid mUuid;
        QString mName;
        Point mPosition;
        Length mLength;
        Angle mRotation;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb

#endif // LIBREPCB_LIBRARY_SYMBOLPIN_H
