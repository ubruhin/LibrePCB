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
#include <QtWidgets>
#include "editnetclassesdialog.h"
#include "ui_editnetclassesdialog.h"
#include <librepcbproject/circuit/circuit.h>
#include <librepcbproject/project.h>
#include <librepcbcommon/undostack.h>
#include <librepcbproject/circuit/netclass.h>
#include <librepcbproject/circuit/cmd/cmdnetclassedit.h>
#include <librepcbproject/circuit/cmd/cmdnetclassadd.h>
#include <librepcbproject/circuit/cmd/cmdnetclassremove.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

EditNetClassesDialog::EditNetClassesDialog(Circuit& circuit, UndoStack& undoStack, QWidget* parent) throw (Exception) :
    QDialog(parent), mCircuit(circuit), mUi(new Ui::EditNetClassesDialog),
    mUndoStack(undoStack)
{
    mUi->setupUi(this);

    // The next line tries to begin a new command on the project's undo stack. This will
    // block all other commands (neccessary to avoid problems). If another command is
    // active at the moment, this line throws an exception and the constructor is exited.
    mUndoStack.beginCmdGroup(tr("Edit Netclasses"));

    int row = 0;
    mUi->tableWidget->setRowCount(mCircuit.getNetClasses().count());
    foreach (NetClass* netclass, mCircuit.getNetClasses())
    {
        QTableWidgetItem* uuid = new QTableWidgetItem(netclass->getUuid().toStr());
        QTableWidgetItem* name = new QTableWidgetItem(netclass->getName());
        uuid->setData(Qt::UserRole, qVariantFromValue(static_cast<void*>(netclass)));
        name->setData(Qt::UserRole, qVariantFromValue(static_cast<void*>(netclass)));
        mUi->tableWidget->setVerticalHeaderItem(row, uuid);
        mUi->tableWidget->setItem(row, 0, name);
        row++;
    }

    // load the window geometry
    QSettings clientSettings;
    restoreGeometry(clientSettings.value("netclasses_dialog/window_geometry").toByteArray());
}

EditNetClassesDialog::~EditNetClassesDialog() noexcept
{
    // save the window geometry
    QSettings clientSettings;
    clientSettings.setValue("netclasses_dialog/window_geometry", saveGeometry());

    // end the active command
    if (result() == QDialog::Accepted)
        try {mUndoStack.commitCmdGroup();} catch (...) {}
    else
        try {mUndoStack.abortCmdGroup();} catch (...) {}

    delete mUi;         mUi = 0;
}

/*****************************************************************************************
 *  Public Slots
 ****************************************************************************************/

void EditNetClassesDialog::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    switch (item->column())
    {
        case 0: // name changed
        {
            NetClass* netclass = static_cast<NetClass*>(item->data(Qt::UserRole).value<void*>());
            if (!netclass) break;
            if (item->text() == netclass->getName()) break;
            try
            {
                auto cmd = new CmdNetClassEdit(mCircuit, *netclass);
                cmd->setName(item->text());
                mUndoStack.appendToCmdGroup(cmd);
            }
            catch (Exception& e)
            {
                QMessageBox::critical(this, tr("Could not change netclass name"), e.getUserMsg());
            }
            item->setText(netclass->getName());
            break;
        }

        default:
            break;
    }
}

void EditNetClassesDialog::on_btnAdd_clicked()
{
    QString name = QInputDialog::getText(this, tr("Add Net Class"), tr("Name:"));
    try
    {
        CmdNetClassAdd* cmd = new CmdNetClassAdd(mCircuit, name);
        mUndoStack.appendToCmdGroup(cmd);

        int row = mUi->tableWidget->rowCount();
        mUi->tableWidget->insertRow(row);
        QTableWidgetItem* uuid = new QTableWidgetItem(cmd->getNetClass()->getUuid().toStr());
        QTableWidgetItem* name = new QTableWidgetItem(cmd->getNetClass()->getName());
        name->setData(Qt::UserRole, qVariantFromValue(static_cast<void*>(cmd->getNetClass())));
        mUi->tableWidget->setVerticalHeaderItem(row, uuid);
        mUi->tableWidget->setItem(row, 0, name);
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("Could not add netclass"), e.getUserMsg());
    }
}

void EditNetClassesDialog::on_btnRemove_clicked()
{
    int row = mUi->tableWidget->currentRow();
    if (row < 0) return;
    NetClass* netclass = static_cast<NetClass*>(mUi->tableWidget->verticalHeaderItem(row)->data(Qt::UserRole).value<void*>());
    if (!netclass) return;

    try
    {
        CmdNetClassRemove* cmd = new CmdNetClassRemove(mCircuit, *netclass);
        mUndoStack.appendToCmdGroup(cmd);

        mUi->tableWidget->removeRow(row);
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("Could not remove netclass"), e.getUserMsg());
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
