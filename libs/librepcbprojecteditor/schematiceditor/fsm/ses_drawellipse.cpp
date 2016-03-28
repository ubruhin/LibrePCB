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
#include "ses_drawellipse.h"
#include "../schematiceditor.h"
#include "ui_schematiceditor.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

SES_DrawEllipse::SES_DrawEllipse(SchematicEditor& editor, Ui::SchematicEditor& editorUi,
                                 GraphicsView& editorGraphicsView, UndoStack& undoStack) :
    SES_Base(editor, editorUi, editorGraphicsView, undoStack)
{
}

SES_DrawEllipse::~SES_DrawEllipse()
{
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

SES_Base::ProcRetVal SES_DrawEllipse::process(SEE_Base* event) noexcept
{
    Q_UNUSED(event);
    return PassToParentState;
}

bool SES_DrawEllipse::entry(SEE_Base* event) noexcept
{
    Q_UNUSED(event);
    mEditorUi.actionToolDrawEllipse->setCheckable(true);
    mEditorUi.actionToolDrawEllipse->setChecked(true);
    return true;
}

bool SES_DrawEllipse::exit(SEE_Base* event) noexcept
{
    Q_UNUSED(event);
    mEditorUi.actionToolDrawEllipse->setCheckable(false);
    mEditorUi.actionToolDrawEllipse->setChecked(false);
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
} // namespace librepcb
