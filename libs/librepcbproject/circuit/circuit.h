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

#ifndef LIBREPCB_PROJECT_CIRCUIT_H
#define LIBREPCB_PROJECT_CIRCUIT_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <librepcbcommon/uuid.h>
#include <librepcbcommon/fileio/if_xmlserializableobject.h>
#include <librepcbcommon/exceptions.h>
#include <librepcbcommon/fileio/filepath.h>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

class SmartXmlFile;

namespace library {
class Component;
}

namespace project {

class Project;
class NetClass;
class NetSignal;
class ComponentInstance;

/*****************************************************************************************
 *  Class Circuit
 ****************************************************************************************/

/**
 * @brief   The Circuit class represents all electrical connections in a project (drawed
 *          in the schematics)
 *
 * Each #project#Project object contains exactly one #Circuit object which contains the
 * whole electrical components and connections. They are created with the schematic editor
 * and used by the board editor. The whole circuit is saved in the file "circuit.xml" in
 * the project's "core" directory.
 *
 * Each #Circuit object contains:
 *  - All net classes (project#NetClass objects)
 *  - All net signals (project#NetSignal objects)
 *  - All component instances (project#ComponentInstance objects)
 *
 * @author ubruhin
 * @date 2014-07-03
 */
class Circuit final : public QObject, public IF_XmlSerializableObject
{
        Q_OBJECT

    public:

        // Constructors / Destructor
        Circuit() = delete;
        Circuit(const Circuit& other) = delete;
        Circuit(Project& project, bool restore, bool readOnly, bool create) throw (Exception);
        ~Circuit() noexcept;

        // Getters
        Project& getProject() const noexcept {return mProject;}

        // NetClass Methods
        const QMap<Uuid, NetClass*>& getNetClasses() const noexcept {return mNetClasses;}
        NetClass* getNetClassByUuid(const Uuid& uuid) const noexcept;
        NetClass* getNetClassByName(const QString& name) const noexcept;
        void addNetClass(NetClass& netclass) throw (Exception);
        void removeNetClass(NetClass& netclass) throw (Exception);
        void setNetClassName(NetClass& netclass, const QString& newName) throw (Exception);

        // NetSignal Methods
        QString generateAutoNetSignalName() const noexcept;
        const QMap<Uuid, NetSignal*>& getNetSignals() const noexcept {return mNetSignals;}
        NetSignal* getNetSignalByUuid(const Uuid& uuid) const noexcept;
        NetSignal* getNetSignalByName(const QString& name) const noexcept;
        void addNetSignal(NetSignal& netsignal) throw (Exception);
        void removeNetSignal(NetSignal& netsignal) throw (Exception);
        void setNetSignalName(NetSignal& netsignal, const QString& newName, bool isAutoName) throw (Exception);
        void setHighlightedNetSignal(NetSignal* signal) noexcept;

        // ComponentInstance Methods
        QString generateAutoComponentInstanceName(const QString& cmpPrefix) const noexcept;
        const QMap<Uuid, ComponentInstance*>& getComponentInstances() const noexcept {return mComponentInstances;}
        ComponentInstance* getComponentInstanceByUuid(const Uuid& uuid) const noexcept;
        ComponentInstance* getComponentInstanceByName(const QString& name) const noexcept;
        void addComponentInstance(ComponentInstance& cmp) throw (Exception);
        void removeComponentInstance(ComponentInstance& cmp) throw (Exception);
        void setComponentInstanceName(ComponentInstance& cmp, const QString& newName) throw (Exception);

        // General Methods
        bool save(bool toOriginal, QStringList& errors) noexcept;

        // Operator Overloadings
        Circuit& operator=(const Circuit& rhs) = delete;
        bool operator==(const Circuit& rhs) noexcept {return (this == &rhs);}
        bool operator!=(const Circuit& rhs) noexcept {return (this != &rhs);}


    signals:

        void netClassAdded(NetClass& netclass);
        void netClassRemoved(NetClass& netclass);
        void netSignalAdded(NetSignal& netsignal);
        void netSignalRemoved(NetSignal& netsignal);
        void componentAdded(ComponentInstance& cmp);
        void componentRemoved(ComponentInstance& cmp);


    private:

        /// @copydoc IF_XmlSerializableObject#checkAttributesValidity()
        bool checkAttributesValidity() const noexcept override;

        /// @copydoc IF_XmlSerializableObject#serializeToXmlDomElement()
        XmlDomElement* serializeToXmlDomElement() const throw (Exception) override;


        // General
        Project& mProject; ///< A reference to the Project object (from the ctor)

        // File "core/circuit.xml"
        FilePath mXmlFilepath;
        SmartXmlFile* mXmlFile;

        QMap<Uuid, NetClass*> mNetClasses;
        QMap<Uuid, NetSignal*> mNetSignals;
        QMap<Uuid, ComponentInstance*> mComponentInstances;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb

#endif // LIBREPCB_PROJECT_CIRCUIT_H
