/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
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

#ifndef LIBREPCB_WORKSPACE_WORKSPACELIBRARYSCANNER_H
#define LIBREPCB_WORKSPACE_WORKSPACELIBRARYSCANNER_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcb/common/exceptions.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/

namespace librepcb {

class SQLiteDatabase;

namespace workspace {

class Workspace;

/*****************************************************************************************
 *  Class WorkspaceLibraryScanner
 ****************************************************************************************/

/**
 * @brief The WorkspaceLibraryScanner class
 *
 * @warning Be very careful with dependencies to other objects as the #run() method is
 *          executed in a separate thread! Keep the number of dependencies as small as
 *          possible and consider thread synchronization and object lifetimes.
 *
 * @todo    Don't really sure that the #run() method is 100% thread save ;)
 *          Maybe it would be better to put the whole library scanning code into this
 *          class instead of having references to objects from the library and workspace
 *          namespaces. This way it would be easier to guarantee thread safety.
 *
 * @author ubruhin
 * @date 2016-09-06
 */
class WorkspaceLibraryScanner final : public QThread
{
        Q_OBJECT

    public:

        // Constructors / Destructor
        explicit WorkspaceLibraryScanner(Workspace& ws) noexcept;
        WorkspaceLibraryScanner(const WorkspaceLibraryScanner& other) = delete;
        ~WorkspaceLibraryScanner() noexcept;

        // General Methods
        bool abort() noexcept;

        // Operator Overloadings
        WorkspaceLibraryScanner& operator=(const WorkspaceLibraryScanner& rhs) = delete;


    signals:

        void started();
        void progressUpdate(int percent);
        void succeeded(int elementCount);
        void failed(QString errorMsg);


    private: // Methods

        void run() noexcept override;
        void clearAllTables(SQLiteDatabase& db) throw (Exception);
        template <typename ElementType>
        int addCategoriesToDb(SQLiteDatabase& db, const QList<FilePath>& dirs,
                              const QString& table, const QString& idColumn) throw (Exception);
        template <typename ElementType>
        int addElementsToDb(SQLiteDatabase& db, const QList<FilePath>& dirs,
                            const QString& table, const QString& idColumn) throw (Exception);
        int addDevicesToDb(SQLiteDatabase& db, const QList<FilePath>& dirs,
                           const QString& table, const QString& idColumn) throw (Exception);


    private: // Data

        Workspace& mWorkspace;
        volatile bool mAbort;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace workspace
} // namespace librepcb

#endif // LIBREPCB_WORKSPACE_WORKSPACELIBRARYSCANNER_H
