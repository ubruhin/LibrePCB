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
#include "bes_addvia.h"
#include "../boardeditor.h"
#include "ui_boardeditor.h"
#include <librepcbproject/boards/board.h>
#include <librepcbproject/boards/items/bi_via.h>
#include <librepcbcommon/gridproperties.h>
#include <librepcbcommon/undostack.h>
#include <librepcbproject/boards/cmd/cmdboardviaadd.h>
#include <librepcbproject/boards/cmd/cmdboardviaedit.h>
#include <librepcbproject/project.h>
#include <librepcbproject/circuit/circuit.h>
#include <librepcbproject/circuit/netsignal.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

BES_AddVia::BES_AddVia(BoardEditor& editor, Ui::BoardEditor& editorUi,
                       GraphicsView& editorGraphicsView, UndoStack& undoStack) :
    BES_Base(editor, editorUi, editorGraphicsView, undoStack),
    mUndoCmdActive(false), mCurrentVia(nullptr), mCurrentViaShape(BI_Via::Shape::Round),
    mCurrentViaSize(700000), mCurrentViaDrillDiameter(300000), mCurrentViaNetSignal(nullptr),
    // command toolbar actions / widgets:
    mSizeLabel(nullptr), mSizeComboBox(nullptr), mDrillLabel(nullptr),
    mDrillComboBox(nullptr), mNetSignalLabel(nullptr), mNetSignalComboBox(nullptr)
{
}

BES_AddVia::~BES_AddVia()
{
    Q_ASSERT(mUndoCmdActive == false);
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

BES_Base::ProcRetVal BES_AddVia::process(BEE_Base* event) noexcept
{
    switch (event->getType())
    {
        case BEE_Base::GraphicsViewEvent:
            return processSceneEvent(event);
        default:
            return PassToParentState;
    }
}

bool BES_AddVia::entry(BEE_Base* event) noexcept
{
    Q_UNUSED(event);
    Board* board = mEditor.getActiveBoard();
    if (!board) return false;

    // clear board selection because selection does not make sense in this state
    board->clearSelection();

    // reset current via settings
    mCurrentViaNetSignal = nullptr;

    // add a new via
    if (!addVia(*board)) return false;

    // Check this state in the "tools" toolbar
    mEditorUi.actionToolAddVia->setCheckable(true);
    mEditorUi.actionToolAddVia->setChecked(true);

    // Add shape actions to the "command" toolbar
    mShapeActions.insert(static_cast<int>(BI_Via::Shape::Round),
                         mEditorUi.commandToolbar->addAction(
                         QIcon(":/img/command_toolbars/via_round.png"), ""));
    mShapeActions.insert(static_cast<int>(BI_Via::Shape::Square),
                         mEditorUi.commandToolbar->addAction(
                         QIcon(":/img/command_toolbars/via_square.png"), ""));
    mShapeActions.insert(static_cast<int>(BI_Via::Shape::Octagon),
                         mEditorUi.commandToolbar->addAction(
                         QIcon(":/img/command_toolbars/via_octagon.png"), ""));
    mActionSeparators.append(mEditorUi.commandToolbar->addSeparator());
    updateShapeActionsCheckedState();

    // connect the shape actions with the slot updateShapeActionsCheckedState()
    foreach (int shape, mShapeActions.keys()) {
        connect(mShapeActions.value(shape), &QAction::triggered,
                [this, shape](){mCurrentViaShape = static_cast<BI_Via::Shape>(shape);
                                updateShapeActionsCheckedState();});
    }

    // add the "Size:" label to the toolbar
    mSizeLabel = new QLabel(tr("Size:"));
    mSizeLabel->setIndent(10);
    mEditorUi.commandToolbar->addWidget(mSizeLabel);

    // add the size combobox to the toolbar
    mSizeComboBox = new QComboBox();
    mSizeComboBox->setMinimumContentsLength(6);
    mSizeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mSizeComboBox->setInsertPolicy(QComboBox::NoInsert);
    mSizeComboBox->setEditable(true);
    mSizeComboBox->addItem("0.7");
    mSizeComboBox->addItem("0.8");
    mSizeComboBox->addItem("1");
    mSizeComboBox->addItem("1.2");
    mSizeComboBox->setCurrentIndex(mSizeComboBox->findText(QString::number(mCurrentViaSize.toMm())));
    mEditorUi.commandToolbar->addWidget(mSizeComboBox);
    connect(mSizeComboBox, &QComboBox::currentTextChanged,
            [this](const QString& value)
            {try{ mCurrentViaSize = Length::fromMm(value); } catch (...) {} });

    // add the "Drill:" label to the toolbar
    mDrillLabel = new QLabel(tr("Drill:"));
    mDrillLabel->setIndent(10);
    mEditorUi.commandToolbar->addWidget(mDrillLabel);

    // add the drill combobox to the toolbar
    mDrillComboBox = new QComboBox();
    mDrillComboBox->setMinimumContentsLength(6);
    mDrillComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mDrillComboBox->setInsertPolicy(QComboBox::NoInsert);
    mDrillComboBox->setEditable(true);
    mDrillComboBox->addItem("0.3");
    mDrillComboBox->addItem("0.4");
    mDrillComboBox->addItem("0.6");
    mDrillComboBox->addItem("0.8");
    mDrillComboBox->setCurrentIndex(mDrillComboBox->findText(QString::number(mCurrentViaDrillDiameter.toMm())));
    mEditorUi.commandToolbar->addWidget(mDrillComboBox);
    connect(mDrillComboBox, &QComboBox::currentTextChanged,
            [this](const QString& value)
            {try{ mCurrentViaDrillDiameter = Length::fromMm(value); } catch (...) {} });

    // add the "Signal:" label to the toolbar
    mNetSignalLabel = new QLabel(tr("Signal:"));
    mNetSignalLabel->setIndent(10);
    mEditorUi.commandToolbar->addWidget(mNetSignalLabel);

    // add the netsignals combobox to the toolbar
    mNetSignalComboBox = new QComboBox();
    mNetSignalComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    mNetSignalComboBox->setInsertPolicy(QComboBox::NoInsert);
    mNetSignalComboBox->setEditable(false);
    mNetSignalComboBox->addItem(""); // no netsignal
    foreach (NetSignal* netsignal, mEditor.getProject().getCircuit().getNetSignals())
        mNetSignalComboBox->addItem(netsignal->getName(), netsignal->getUuid().toStr());
    mNetSignalComboBox->model()->sort(0);
    mNetSignalComboBox->setCurrentText(mCurrentViaNetSignal ? mCurrentViaNetSignal->getName() : "");
    mEditorUi.commandToolbar->addWidget(mNetSignalComboBox);
    connect(mNetSignalComboBox, &QComboBox::currentTextChanged,
            [this](const QString& value)
            {mCurrentViaNetSignal = mEditor.getProject().getCircuit().getNetSignalByName(value);});

    return true;
}

bool BES_AddVia::exit(BEE_Base* event) noexcept
{
    Q_UNUSED(event);

    if (mUndoCmdActive)
    {
        try
        {
            mUndoStack.abortCmdGroup();
            mUndoCmdActive = false;
        }
        catch (Exception& e)
        {
            QMessageBox::critical(&mEditor, tr("Error"), e.getUserMsg());
            return false;
        }
    }

    // Remove actions / widgets from the "command" toolbar
    delete mNetSignalComboBox;      mNetSignalComboBox = nullptr;
    delete mNetSignalLabel;         mNetSignalLabel = nullptr;
    delete mDrillComboBox;          mDrillComboBox = nullptr;
    delete mDrillLabel;             mDrillLabel = nullptr;
    delete mSizeComboBox;           mSizeComboBox = nullptr;
    delete mSizeLabel;              mSizeLabel = nullptr;
    qDeleteAll(mShapeActions);      mShapeActions.clear();
    qDeleteAll(mActionSeparators);  mActionSeparators.clear();

    // Uncheck this state in the "tools" toolbar
    mEditorUi.actionToolAddVia->setCheckable(false);
    mEditorUi.actionToolAddVia->setChecked(false);
    return true;
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

BES_Base::ProcRetVal BES_AddVia::processSceneEvent(BEE_Base* event) noexcept
{
    QEvent* qevent = BEE_RedirectedQEvent::getQEventFromBEE(event);
    Q_ASSERT(qevent); if (!qevent) return PassToParentState;
    Board* board = mEditor.getActiveBoard();
    Q_ASSERT(board); if (!board) return PassToParentState;

    switch (qevent->type())
    {
        case QEvent::GraphicsSceneMouseDoubleClick:
        case QEvent::GraphicsSceneMousePress:
        {
            QGraphicsSceneMouseEvent* sceneEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(qevent);
            Point pos = Point::fromPx(sceneEvent->scenePos(), board->getGridProperties().getInterval());
            switch (sceneEvent->button())
            {
                case Qt::LeftButton:
                {
                    fixVia(pos);
                    addVia(*board);
                    updateVia(*board, pos);
                    return ForceStayInState;
                }
                case Qt::RightButton:
                    return ForceStayInState;
                default:
                    break;
            }
            break;
        }

        /*case QEvent::GraphicsSceneMouseRelease:
        {
            QGraphicsSceneMouseEvent* sceneEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(qevent);
            Point pos = Point::fromPx(sceneEvent->scenePos(), board->getGridProperties().getInterval());
            switch (sceneEvent->button())
            {
                case Qt::RightButton:
                {
                    if (sceneEvent->screenPos() == sceneEvent->buttonDownScreenPos(Qt::RightButton)) {
                        mEditCmd->rotate(Angle::deg90(), pos, true);
                        return ForceStayInState;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }*/

        case QEvent::GraphicsSceneMouseMove:
        {
            QGraphicsSceneMouseEvent* sceneEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(qevent);
            Q_ASSERT(sceneEvent);
            Point pos = Point::fromPx(sceneEvent->scenePos(), board->getGridProperties().getInterval());
            updateVia(*board, pos);
            return ForceStayInState;
        }

        default:
            break;
    }
    return PassToParentState;
}

bool BES_AddVia::addVia(Board& board) noexcept
{
    Q_ASSERT(mUndoCmdActive == false);

    try
    {
        mUndoStack.beginCmdGroup(tr("Add via to board"));
        mUndoCmdActive = true;
        CmdBoardViaAdd* cmdAdd = new CmdBoardViaAdd(board, Point(0, 0), mCurrentViaShape,
            mCurrentViaSize, mCurrentViaDrillDiameter, mCurrentViaNetSignal);
        mUndoStack.appendToCmdGroup(cmdAdd);
        mCurrentVia = cmdAdd->getVia();
        mEditCmd.reset(new CmdBoardViaEdit(*mCurrentVia));
        return true;
    }
    catch (Exception& e)
    {
        if (mUndoCmdActive)
        {
            try {mUndoStack.abortCmdGroup();} catch (...) {}
            mUndoCmdActive = false;
        }
        QMessageBox::critical(&mEditor, tr("Error"), e.getUserMsg());
        return false;
    }
}

bool BES_AddVia::updateVia(Board& board, const Point& pos) noexcept
{
    Q_UNUSED(board);
    Q_ASSERT(mUndoCmdActive == true);

    try
    {
        mEditCmd->setPosition(pos, true);
        mEditCmd->setShape(mCurrentViaShape, true);
        mEditCmd->setSize(mCurrentViaSize, true);
        mEditCmd->setDrillDiameter(mCurrentViaDrillDiameter, true);
        mEditCmd->setNetSignal(mCurrentViaNetSignal, true); // can throw
        return true;
    }
    catch (Exception& e)
    {
        QMessageBox::critical(&mEditor, tr("Error"), e.getUserMsg());
        return false;
    }
}

bool BES_AddVia::fixVia(const Point& pos) noexcept
{
    Q_ASSERT(mUndoCmdActive == true);

    try
    {
        mEditCmd->setPosition(pos, false);
        mUndoStack.appendToCmdGroup(mEditCmd.take());
        mUndoStack.commitCmdGroup();
        mUndoCmdActive = false;
        return true;
    }
    catch (Exception& e)
    {
        if (mUndoCmdActive)
        {
            try {mUndoStack.abortCmdGroup();} catch (...) {}
            mUndoCmdActive = false;
        }
        QMessageBox::critical(&mEditor, tr("Error"), e.getUserMsg());
        return false;
    }
}

void BES_AddVia::updateShapeActionsCheckedState() noexcept
{
    foreach (int key, mShapeActions.keys())
    {
        mShapeActions.value(key)->setCheckable(key == static_cast<int>(mCurrentViaShape));
        mShapeActions.value(key)->setChecked(key == static_cast<int>(mCurrentViaShape));
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
