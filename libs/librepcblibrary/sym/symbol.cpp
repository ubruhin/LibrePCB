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
#include "symbol.h"
#include <librepcbcommon/fileio/xmldomelement.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

Symbol::Symbol(const Uuid& uuid, const Version& version, const QString& author,
               const QString& name_en_US, const QString& description_en_US,
               const QString& keywords_en_US) throw (Exception) :
    LibraryElement("sym", "symbol", uuid, version, author, name_en_US, description_en_US, keywords_en_US)
{
}

Symbol::Symbol(const FilePath& elementDirectory, bool readOnly) throw (Exception) :
    LibraryElement(elementDirectory, "sym", "symbol", readOnly)
{
    try
    {
        readFromFile();
    }
    catch (Exception& e)
    {
        qDeleteAll(mTexts);         mTexts.clear();
        qDeleteAll(mEllipses);      mEllipses.clear();
        qDeleteAll(mPolygons);      mPolygons.clear();
        qDeleteAll(mPins);          mPins.clear();
        throw;
    }
}

Symbol::~Symbol() noexcept
{

    qDeleteAll(mEllipses);      mEllipses.clear();
    qDeleteAll(mPolygons);      mPolygons.clear();
    qDeleteAll(mPins);          mPins.clear();
}

/*****************************************************************************************
 *  FootprintPad Methods
 ****************************************************************************************/

void Symbol::addPin(SymbolPin& pin) noexcept
{
    Q_ASSERT(!mPins.contains(pin.getUuid()));
    mPins.insert(pin.getUuid(), &pin);
}

void Symbol::removePin(SymbolPin& pin) noexcept
{
    Q_ASSERT(mPins.contains(pin.getUuid()));
    Q_ASSERT(mPins.value(pin.getUuid()) == &pin);
    mPins.remove(pin.getUuid());
}

/*****************************************************************************************
 *  Polygon Methods
 ****************************************************************************************/

void Symbol::addPolygon(Polygon& polygon) noexcept
{
    Q_ASSERT(!mPolygons.contains(&polygon));
    mPolygons.append(&polygon);
}

void Symbol::removePolygon(Polygon& polygon) noexcept
{
    Q_ASSERT(mPolygons.contains(&polygon));
    mPolygons.removeAll(&polygon);
}

/*****************************************************************************************
 *  Ellipse Methods
 ****************************************************************************************/

void Symbol::addEllipse(Ellipse& ellipse) noexcept
{
    Q_ASSERT(!mEllipses.contains(&ellipse));
    mEllipses.append(&ellipse);
}

void Symbol::removeEllipse(Ellipse& ellipse) noexcept
{
    Q_ASSERT(mEllipses.contains(&ellipse));
    mEllipses.removeAll(&ellipse);
}

/*****************************************************************************************
 *  Text Methods
 ****************************************************************************************/

void Symbol::addText(Text& text) noexcept
{
    Q_ASSERT(!mTexts.contains(&text));
    mTexts.append(&text);
}

void Symbol::removeText(Text& text) noexcept
{
    Q_ASSERT(mTexts.contains(&text));
    mTexts.removeAll(&text);
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void Symbol::parseDomTree(const XmlDomElement& root) throw (Exception)
{
    LibraryElement::parseDomTree(root);

    // Load all pins
    for (XmlDomElement* node = root.getFirstChild("pins/pin", true, false);
         node; node = node->getNextSibling("pin"))
    {
        SymbolPin* pin = new SymbolPin(*node);
        if (mPins.contains(pin->getUuid()))
        {
            throw RuntimeError(__FILE__, __LINE__, pin->getUuid().toStr(),
                QString(tr("The pin \"%1\" exists multiple times in \"%2\"."))
                .arg(pin->getUuid().toStr(), root.getDocFilePath().toNative()));
        }
        mPins.insert(pin->getUuid(), pin);
    }

    // Load all geometry elements
    for (XmlDomElement* node = root.getFirstChild("geometry/*", true, false);
         node; node = node->getNextSibling())
    {
        if (node->getName() == "polygon")
        {
            mPolygons.append(new Polygon(*node));
        }
        else if (node->getName() == "text")
        {
            mTexts.append(new Text(*node));
        }
        else if (node->getName() == "ellipse")
        {
            mEllipses.append(new Ellipse(*node));
        }
        else
        {
            throw RuntimeError(__FILE__, __LINE__, node->getName(),
                QString(tr("Unknown geometry element \"%1\" in \"%2\"."))
                .arg(node->getName(), root.getDocFilePath().toNative()));
        }
    }
}

XmlDomElement* Symbol::serializeToXmlDomElement() const throw (Exception)
{
    QScopedPointer<XmlDomElement> root(LibraryElement::serializeToXmlDomElement());
    XmlDomElement* pins = root->appendChild("pins");
    foreach (const SymbolPin* pin, mPins)
        pins->appendChild(pin->serializeToXmlDomElement());
    XmlDomElement* geometry = root->appendChild("geometry");
    foreach (const Polygon* polygon, mPolygons)
        geometry->appendChild(polygon->serializeToXmlDomElement());
    foreach (const Text* text, mTexts)
        geometry->appendChild(text->serializeToXmlDomElement());
    foreach (const Ellipse* ellipse, mEllipses)
        geometry->appendChild(ellipse->serializeToXmlDomElement());
    return root.take();
}

bool Symbol::checkAttributesValidity() const noexcept
{
    if (!LibraryElement::checkAttributesValidity())                 return false;
    if (mPins.isEmpty() && mTexts.isEmpty() &&
        mPolygons.isEmpty() && mEllipses.isEmpty())                 return false;
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb
