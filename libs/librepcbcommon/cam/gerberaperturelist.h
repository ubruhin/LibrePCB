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

#ifndef LIBREPCB_GERBERAPERTURELIST_H
#define LIBREPCB_GERBERAPERTURELIST_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "../exceptions.h"
#include "../fileio/filepath.h"
#include "../units/all_length_units.h"
#include "../uuid.h"

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Class GerberApertureList
 ****************************************************************************************/

/**
 * @brief The GerberApertureList class
 *
 * @author ubruhin
 * @date 2016-03-31
 */
class GerberApertureList final
{
        Q_DECLARE_TR_FUNCTIONS(GerberApertureList)

    public:

        // Constructors / Destructor
        //GerberApertureList() = delete;
        GerberApertureList(const GerberApertureList& other) = delete;
        GerberApertureList() noexcept;
        ~GerberApertureList() noexcept;

        // Getters
        QString generateString() const noexcept;

        // General Methods
        int setCircle(const Length& dia, const Length& hole);
        int setRect(const Length& w, const Length& h, const Angle& rot, const Length& hole) noexcept;
        int setObround(const Length& w, const Length& h, const Angle& rot, const Length& hole) noexcept;
        int setRegularPolygon(const Length& dia, int n, const Angle& rot, const Length& hole) noexcept;
        void reset() noexcept;

        // Operator Overloadings
        GerberApertureList& operator=(const GerberApertureList& rhs) = delete;


    private:

        // Private Methods
        int setCurrentAperture(const QString& aperture) noexcept;
        void addMacro(const QString& macro) noexcept;

        // Aperture Generator Methods
        static QString generateCircle(const Length& dia, const Length& hole) noexcept;
        static QString generateRect(const Length& w, const Length& h, const Length& hole) noexcept;
        static QString generateObround(const Length& w, const Length& h, const Length& hole) noexcept;
        static QString generateRegularPolygon(const Length& dia, int n, const Angle& rot, const Length& hole) noexcept;
        static QString generateRotatedRectMacro();
        static QString generateRotatedRectMacroWithHole();
        static QString generateRotatedObroundMacro();
        static QString generateRotatedObroundMacroWithHole();
        static QString generateRotatedRect(const Length& w, const Length& h, const Angle& rot, const Length& hole) noexcept;
        static QString generateRotatedObround(const Length& w, const Length& h, const Angle& rot, const Length& hole) noexcept;


        QList<QString> mApertureMacros;
        QMap<int, QString> mApertures; ///< key: aperture number (>= 10); value: aperture definition
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb

#endif // LIBREPCB_GERBERAPERTURELIST_H
