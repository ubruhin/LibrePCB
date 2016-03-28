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
#include "undostack.h"
#include "undocommand.h"
#include "undocommandgroup.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

UndoStack::UndoStack() noexcept :
    QObject(nullptr), mCurrentIndex(0), mCleanIndex(0), mActiveCommandGroup(nullptr)
{
}

UndoStack::~UndoStack() noexcept
{
    clear();
}

/*****************************************************************************************
 *  Getters
 ****************************************************************************************/

QString UndoStack::getUndoText() const noexcept
{
    if (canUndo())
        return QString(tr("Undo: %1")).arg(mCommands[mCurrentIndex-1]->getText());
    else
        return tr("Undo");
}

QString UndoStack::getRedoText() const noexcept
{
    if (canRedo())
        return QString(tr("Redo: %1")).arg(mCommands[mCurrentIndex]->getText());
    else
        return tr("Redo");
}

bool UndoStack::canUndo() const noexcept
{
    return (mCurrentIndex > 0);
}

bool UndoStack::canRedo() const noexcept
{
    return (mCurrentIndex < mCommands.count());
}

bool UndoStack::isClean() const noexcept
{
    return (mCurrentIndex == mCleanIndex);
}

bool UndoStack::isCommandGroupActive() const noexcept
{
    return (mActiveCommandGroup != nullptr);
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

void UndoStack::setClean() noexcept
{
    if (isClean()) return;

    mCleanIndex = mCurrentIndex;

    emit cleanChanged(true);
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void UndoStack::execCmd(UndoCommand* cmd, bool forceKeepCmd) throw (Exception)
{
    // make sure "cmd" is deleted when going out of scope (e.g. because of an exception)
    QScopedPointer<UndoCommand> cmdScopeGuard(cmd);

    if (isCommandGroupActive()) {
        throw RuntimeError(__FILE__, __LINE__, QString(), tr("Another command is active "
                           "at the moment. Please finish that command to continue."));
    }

    bool commandHasDoneSomething = cmd->execute(); // can throw

    if (commandHasDoneSomething || forceKeepCmd) {
        // the clean state will no longer exist -> make the index invalid
        if (mCleanIndex > mCurrentIndex) {
            mCleanIndex = -1;
        }

        // delete all commands above the current index (make redoing them impossible)
        // --> in reverse order (from top to bottom)!
        while (mCurrentIndex < mCommands.count()) {
            delete mCommands.takeLast();
        }
        Q_ASSERT(mCurrentIndex == mCommands.count());

        // add command to the command stack and emit signals
        mCommands.append(cmdScopeGuard.take()); // move ownership of "cmd" to "mCommands"
        mCurrentIndex++;

        // emit signals
        emit undoTextChanged(QString(tr("Undo: %1")).arg(cmd->getText()));
        emit redoTextChanged(tr("Redo"));
        emit canUndoChanged(true);
        emit canRedoChanged(false);
        emit cleanChanged(false);
    } else {
        // the command has done nothing, so we will just discard it
        cmd->undo(); // only to be sure the command has executed nothing...
    }
}

void UndoStack::beginCmdGroup(const QString& text) throw (Exception)
{
    if (isCommandGroupActive()) {
        throw RuntimeError(__FILE__, __LINE__, QString(), tr("Another command is active "
                           "at the moment. Please finish that command to continue."));
    }

    UndoCommandGroup* cmd = new UndoCommandGroup(text);
    execCmd(cmd, true); // throws an exception on error; emits all signals
    Q_ASSERT(mCommands.last() == cmd);
    mActiveCommandGroup = cmd;

    // emit signals
    emit canUndoChanged(false);
}

void UndoStack::appendToCmdGroup(UndoCommand* cmd) throw (Exception)
{
    // make sure "cmd" is deleted when going out of scope (e.g. because of an exception)
    QScopedPointer<UndoCommand> cmdScopeGuard(cmd);

    if (!isCommandGroupActive()) {
        throw LogicError(__FILE__, __LINE__, QString(), tr("No command group active!"));
    }
    Q_ASSERT(mCurrentIndex == mCommands.count());
    Q_ASSERT(mActiveCommandGroup);

    // append new command as a child of active command group
    // note: this will also execute the new command!
    mActiveCommandGroup->appendChild(cmdScopeGuard.take()); // can throw
}

void UndoStack::commitCmdGroup() throw (Exception)
{
    if (!isCommandGroupActive()) {
        throw LogicError(__FILE__, __LINE__, QString(), tr("No command group active!"));
    }
    Q_ASSERT(mCurrentIndex == mCommands.count());
    Q_ASSERT(mActiveCommandGroup);

    if (mActiveCommandGroup->getChildCount() == 0) {
        // the last command is empty --> remove it from the stack!
        abortCmdGroup();
        return;
    }

    // To finish the active command group, we only need to reset the pointer to the
    // currently active command group
    mActiveCommandGroup = nullptr;

    // emit signals
    emit canUndoChanged(canUndo());
    emit commandGroupEnded();
}

void UndoStack::abortCmdGroup() throw (Exception)
{
    if (!isCommandGroupActive()) {
        throw LogicError(__FILE__, __LINE__, QString(), tr("No command group active!"));
    }
    Q_ASSERT(mCurrentIndex == mCommands.count());
    Q_ASSERT(mActiveCommandGroup);
    Q_ASSERT(mCommands.last() == mActiveCommandGroup);

    try {
        mActiveCommandGroup->undo(); // can throw (but should usually not)
        mActiveCommandGroup = nullptr;
        mCurrentIndex--;
        delete mCommands.takeLast(); // delete and remove the aborted command group from the stack
    } catch (Exception& e) {
        qCritical() << "UndoCommand::undo() has thrown an exception:" << e.getUserMsg();
        throw;
    }

    // emit signals
    emit undoTextChanged(getUndoText());
    emit redoTextChanged(tr("Redo"));
    emit canUndoChanged(canUndo());
    emit canRedoChanged(false);
    emit cleanChanged(isClean());
    emit commandGroupAborted(); // this is important!
}

void UndoStack::undo() throw (Exception)
{
    if ((!canUndo()) || (isCommandGroupActive())) {
        return; // if a command group is active, undo() is not allowed
    }

    try {
        mCommands[mCurrentIndex-1]->undo(); // can throw (but should usually not)
        mCurrentIndex--;
    } catch (Exception& e) {
        qCritical() << "UndoCommand::undo() has thrown an exception:" << e.getUserMsg();
        throw;
    }

    // emit signals
    emit undoTextChanged(getUndoText());
    emit redoTextChanged(getRedoText());
    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit cleanChanged(isClean());
}

void UndoStack::redo() throw (Exception)
{
    if (!canRedo()) {
        return;
    }

    try {
        mCommands[mCurrentIndex]->redo(); // can throw (but should usually not)
        mCurrentIndex++;
    } catch (Exception& e) {
        qCritical() << "UndoCommand::redo() has thrown an exception:" << e.getUserMsg();
        throw;
    }

    // emit signals
    emit undoTextChanged(getUndoText());
    emit redoTextChanged(getRedoText());
    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit cleanChanged(isClean());
}

void UndoStack::clear() noexcept
{
    if (mCommands.isEmpty()) {
        return;
    }

    if (isCommandGroupActive()) {
        try {
            abortCmdGroup();
        } catch (...) {
            qCritical() << "Could not abort the currently active command group!";
        }
    }

    // delete all commands in the stack from top to bottom (newest first, oldest last)!
    while (!mCommands.isEmpty()) {
        delete mCommands.takeLast();
    }

    mCurrentIndex = 0;
    mCleanIndex = 0;
    mActiveCommandGroup = nullptr;

    // emit signals
    emit undoTextChanged(tr("Undo"));
    emit redoTextChanged(tr("Redo"));
    emit canUndoChanged(false);
    emit canRedoChanged(false);
    emit cleanChanged(true);
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace librepcb
