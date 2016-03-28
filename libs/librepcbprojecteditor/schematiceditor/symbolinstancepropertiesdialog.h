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

#ifndef LIBREPCB_PROJECT_SYMBOLINSTANCEPROPERTIESDIALOG_H
#define LIBREPCB_PROJECT_SYMBOLINSTANCEPROPERTIESDIALOG_H

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include <QtWidgets>

/*****************************************************************************************
 *  Namespace / Forward Declarations
 ****************************************************************************************/
namespace librepcb {

class UndoStack;
class UndoCommand;
class AttributeType;
class AttributeUnit;

namespace project {

class Project;
class ComponentInstance;
class SI_Symbol;

namespace Ui {
class SymbolInstancePropertiesDialog;
}

/*****************************************************************************************
 *  Class SymbolInstancePropertiesDialog
 ****************************************************************************************/

/**
 * @brief The SymbolInstancePropertiesDialog class
 */
class SymbolInstancePropertiesDialog final : public QDialog
{
        Q_OBJECT

    public:

        // Constructors / Destructor
        explicit SymbolInstancePropertiesDialog(Project& project, ComponentInstance& cmp,
                                                SI_Symbol& symbol, UndoStack& undoStack,
                                                QWidget* parent) noexcept;
        ~SymbolInstancePropertiesDialog() noexcept;


    private slots:

        // GUI Events
        void on_tblCompInstAttributes_currentCellChanged(int currentRow, int currentColumn,
                                                         int previousRow, int previousColumn);
        void on_cbxAttrType_currentIndexChanged(int index);
        void on_cbxAttrUnit_currentIndexChanged(int index);
        void on_btnAttrApply_clicked();
        void on_btnAttrAdd_clicked();
        void on_btnAttrRemove_clicked();


    private:

        // Types
        struct AttrItem_t {
            QString key;
            const AttributeType* type;
            QString value;
            const AttributeUnit* unit;
        };

        // make some methods inaccessible...
        SymbolInstancePropertiesDialog();
        SymbolInstancePropertiesDialog(const SymbolInstancePropertiesDialog& other);
        SymbolInstancePropertiesDialog& operator=(const SymbolInstancePropertiesDialog& rhs);

        // Private Methods
        void updateAttrTable() noexcept;
        void keyPressEvent(QKeyEvent* e);
        void accept();
        bool applyChanges() noexcept;
        void execCmd(UndoCommand* cmd);
        void endCmd();
        void abortCmd();


        // General
        Project& mProject;
        ComponentInstance& mComponentInstance;
        SI_Symbol& mSymbol;
        Ui::SymbolInstancePropertiesDialog* mUi;
        UndoStack& mUndoStack;
        bool mCommandActive;
        bool mAttributesEdited;
        QList<AttrItem_t*> mAttrItems;
        AttrItem_t* mSelectedAttrItem;
        const AttributeType* mSelectedAttrType;
        const AttributeUnit* mSelectedAttrUnit;
};

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb

#endif // LIBREPCB_PROJECT_SYMBOLINSTANCEPROPERTIESDIALOG_H
