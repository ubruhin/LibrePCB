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
#include "component.h"
#include <librepcbcommon/fileio/xmldomelement.h>

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace library {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

Component::Component(const Uuid& uuid, const Version& version, const QString& author,
                     const QString& name_en_US, const QString& description_en_US,
                     const QString& keywords_en_US) throw (Exception) :
    LibraryElement("cmp", "component", uuid, version, author, name_en_US, description_en_US, keywords_en_US),
    mSchematicOnly(false), mDefaultSymbolVariantUuid()
{
    Q_ASSERT(mUuid.isNull() == false);
}

Component::Component(const FilePath& elementDirectory, bool readOnly) throw (Exception) :
    LibraryElement(elementDirectory, "cmp", "component", readOnly),
    mSchematicOnly(false), mDefaultSymbolVariantUuid()
{
    try
    {
        readFromFile();
    }
    catch (Exception& e)
    {
        qDeleteAll(mSymbolVariants);    mSymbolVariants.clear();
        qDeleteAll(mSignals);           mSignals.clear();
        qDeleteAll(mAttributes);        mAttributes.clear();
        throw;
    }
}

Component::~Component() noexcept
{
    qDeleteAll(mSymbolVariants);    mSymbolVariants.clear();
    qDeleteAll(mSignals);           mSignals.clear();
    qDeleteAll(mAttributes);        mAttributes.clear();
}

/*****************************************************************************************
 *  Attribute Methods
 ****************************************************************************************/

LibraryElementAttribute* Component::getAttributeByKey(const QString& key) noexcept
{
    const Component* const_this = this;
    return const_cast<LibraryElementAttribute*>(const_this->getAttributeByKey(key));
}

const LibraryElementAttribute* Component::getAttributeByKey(const QString& key) const noexcept
{
    foreach (LibraryElementAttribute* attr, mAttributes) {
        if (attr->getKey() == key) return attr;
    }
    return nullptr;
}

void Component::addAttribute(LibraryElementAttribute& attr) noexcept
{
    Q_ASSERT(!mAttributes.contains(&attr));
    mAttributes.append(&attr);
}

void Component::removeAttribute(LibraryElementAttribute& attr) noexcept
{
    Q_ASSERT(mAttributes.contains(&attr));
    mAttributes.removeAll(&attr);
}

/*****************************************************************************************
 *  Default Value Methods
 ****************************************************************************************/

QString Component::getDefaultValue(const QStringList& localeOrder) const throw (Exception)
{
    return LibraryBaseElement::localeStringFromList(mDefaultValues, localeOrder);
}

void Component::addDefaultValue(const QString& locale, const QString& value) noexcept
{
    mDefaultValues.insert(locale, value);
}

void Component::removeDefaultValue(const QString& locale) noexcept
{
    mDefaultValues.remove(locale);
}

/*****************************************************************************************
 *  Prefix Methods
 ****************************************************************************************/

QString Component::getPrefix(const QStringList& normOrder) const noexcept
{
    // search in the specified norm order
    foreach (const QString& norm, normOrder) {
        if (mPrefixes.contains(norm))
            return mPrefixes.value(norm);
    }

    // return the prefix of the default norm
    return getDefaultPrefix();
}

QString Component::getDefaultPrefix() const noexcept
{
    return mPrefixes.value(QString(""));
}

void Component::addPrefix(const QString& norm, const QString& prefix) noexcept
{
    mPrefixes.insert(norm, prefix);
}

/*****************************************************************************************
 *  Signal Methods
 ****************************************************************************************/

ComponentSignal* Component::getSignalByUuid(const Uuid& uuid) noexcept
{
    const Component* const_this = this;
    return const_cast<ComponentSignal*>(const_this->getSignalByUuid(uuid));
}

const ComponentSignal* Component::getSignalByUuid(const Uuid& uuid) const noexcept
{
    foreach (const ComponentSignal* signal, mSignals) {
        if (signal->getUuid() == uuid)
            return signal;
    }
    return nullptr;
}

ComponentSignal* Component::getSignalOfPin(const Uuid& symbVar, const Uuid& item,
                                           const Uuid& pin) noexcept
{
    const Component* const_this = this;
    return const_cast<ComponentSignal*>(const_this->getSignalOfPin(symbVar, item, pin));
}

const ComponentSignal* Component::getSignalOfPin(const Uuid& symbVar, const Uuid& item,
                                                 const Uuid& pin) const noexcept
{
    const ComponentSymbolVariantItem* i = getSymbVarItem(symbVar, item);
    Q_ASSERT(i); if (!i) return nullptr;
    const ComponentPinSignalMapItem* map = i->getPinSignalMapItemOfPin(pin);
    Q_ASSERT(map); if (!map) return nullptr;
    Uuid signalUuid = map->getSignalUuid();
    if (signalUuid.isNull()) return nullptr;
    return getSignalByUuid(signalUuid);
}

void Component::addSignal(ComponentSignal& signal) noexcept
{
    Q_ASSERT(!mSignals.contains(&signal));
    mSignals.append(&signal);
}

void Component::removeSignal(ComponentSignal& signal) noexcept
{
    Q_ASSERT(mSignals.contains(&signal));
    mSignals.removeAll(&signal);
}

/*****************************************************************************************
 *  Symbol Variant Methods
 ****************************************************************************************/

ComponentSymbolVariant* Component::getSymbolVariantByUuid(const Uuid& uuid) noexcept
{
    const Component* const_this = this;
    return const_cast<ComponentSymbolVariant*>(const_this->getSymbolVariantByUuid(uuid));
}

const ComponentSymbolVariant* Component::getSymbolVariantByUuid(const Uuid& uuid) const noexcept
{
    foreach (const ComponentSymbolVariant* var, mSymbolVariants) {
        if (var->getUuid() == uuid)
            return var;
    }
    return nullptr;
}

ComponentSymbolVariant* Component::getDefaultSymbolVariant() noexcept
{
    const Component* const_this = this;
    return const_cast<ComponentSymbolVariant*>(const_this->getDefaultSymbolVariant());
}

const ComponentSymbolVariant* Component::getDefaultSymbolVariant() const noexcept
{
    return getSymbolVariantByUuid(mDefaultSymbolVariantUuid);
}

void Component::addSymbolVariant(ComponentSymbolVariant& symbolVariant) noexcept
{
    Q_ASSERT(!mSymbolVariants.contains(&symbolVariant));
    mSymbolVariants.append(&symbolVariant);
}

void Component::removeSymbolVariant(ComponentSymbolVariant& symbolVariant) noexcept
{
    Q_ASSERT(mSymbolVariants.contains(&symbolVariant));
    mSymbolVariants.removeAll(&symbolVariant);
}

/*****************************************************************************************
 *  Symbol Variant Item Methods
 ****************************************************************************************/

ComponentSymbolVariantItem* Component::getSymbVarItem(const Uuid& symbVar, const Uuid& item) noexcept
{
    const Component* const_this = this;
    return const_cast<ComponentSymbolVariantItem*>(const_this->getSymbVarItem(symbVar, item));
}

const ComponentSymbolVariantItem* Component::getSymbVarItem(const Uuid& symbVar,
                                                            const Uuid& item) const noexcept
{
    const ComponentSymbolVariant* var = getSymbolVariantByUuid(symbVar);
    Q_ASSERT(var); if (!var) return nullptr;
    return var->getItemByUuid(item);
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

void Component::parseDomTree(const XmlDomElement& root) throw (Exception)
{
    LibraryElement::parseDomTree(root);

    mSchematicOnly = root.getFirstChild("properties/schematic_only", true, true)->getText<bool>(true);

    // Load all attributes
    for (XmlDomElement* node = root.getFirstChild("attributes/attribute", true, false);
         node; node = node->getNextSibling("attribute"))
    {
        LibraryElementAttribute* attribute = new LibraryElementAttribute(*node); // throws an exception on error
        if (getAttributeByKey(attribute->getKey()))
        {
            throw RuntimeError(__FILE__, __LINE__, attribute->getKey(),
                QString(tr("The attribute \"%1\" exists multiple times in \"%2\"."))
                .arg(attribute->getKey(), root.getDocFilePath().toNative()));
        }
        mAttributes.append(attribute);
    }

    // Load default values in all available languages
    readLocaleDomNodes(*root.getFirstChild("properties", true, true), "default_value", mDefaultValues);

    // Load all prefixes
    for (XmlDomElement* node = root.getFirstChild("properties/prefix", true, false);
         node; node = node->getNextSibling("prefix"))
    {
        if (mPrefixes.contains(node->getAttribute<QString>("norm", false)))
        {
            throw RuntimeError(__FILE__, __LINE__, node->getAttribute<QString>("norm", false),
                QString(tr("The prefix \"%1\" exists multiple times in \"%2\"."))
                .arg(node->getAttribute<QString>("norm", false), root.getDocFilePath().toNative()));
        }
        mPrefixes.insert(node->getAttribute<QString>("norm", false), node->getText<QString>(false));
    }
    if (!mPrefixes.contains(QString("")))
    {
        throw RuntimeError(__FILE__, __LINE__, root.getDocFilePath().toStr(),
            QString(tr("The file \"%1\" has no default prefix defined."))
            .arg(root.getDocFilePath().toNative()));
    }

    // Load all signals
    for (XmlDomElement* node = root.getFirstChild("signals/signal", true, false);
         node; node = node->getNextSibling("signal"))
    {
        ComponentSignal* signal = new ComponentSignal(*node);
        if (getSignalByUuid(signal->getUuid()))
        {
            throw RuntimeError(__FILE__, __LINE__, signal->getUuid().toStr(),
                QString(tr("The signal \"%1\" exists multiple times in \"%2\"."))
                .arg(signal->getUuid().toStr(), root.getDocFilePath().toNative()));
        }
        mSignals.append(signal);
    }

    // Load all symbol variants
    XmlDomElement* symbolVariantsNode = root.getFirstChild("symbol_variants", true);
    for (XmlDomElement* node = symbolVariantsNode->getFirstChild("variant", false);
         node; node = node->getNextSibling("variant"))
    {
        ComponentSymbolVariant* variant = new ComponentSymbolVariant(*node);
        if (getSymbolVariantByUuid(variant->getUuid()))
        {
            throw RuntimeError(__FILE__, __LINE__, variant->getUuid().toStr(),
                QString(tr("The symbol variant \"%1\" exists multiple times in \"%2\"."))
                .arg(variant->getUuid().toStr(), root.getDocFilePath().toNative()));
        }
        mSymbolVariants.append(variant);
    }
    if (mSymbolVariants.isEmpty()) {
        throw RuntimeError(__FILE__, __LINE__, root.getDocFilePath().toStr(),
            QString(tr("The file \"%1\" has no symbol variants defined."))
            .arg(root.getDocFilePath().toNative()));
    }
    mDefaultSymbolVariantUuid = symbolVariantsNode->getAttribute<Uuid>("default", true);
    if (!getSymbolVariantByUuid(mDefaultSymbolVariantUuid)) {
        throw RuntimeError(__FILE__, __LINE__, root.getDocFilePath().toStr(),
            QString(tr("The file \"%1\" has no valid default symbol variant defined."))
            .arg(root.getDocFilePath().toNative()));
    }
}

XmlDomElement* Component::serializeToXmlDomElement() const throw (Exception)
{
    QScopedPointer<XmlDomElement> root(LibraryElement::serializeToXmlDomElement());
    XmlDomElement* attributes = root->appendChild("attributes");
    foreach (const LibraryElementAttribute* attribute, mAttributes)
        attributes->appendChild(attribute->serializeToXmlDomElement());
    XmlDomElement* properties = root->appendChild("properties");
    properties->appendTextChild("schematic_only", mSchematicOnly);
    foreach (const QString& locale, mDefaultValues.keys()) {
        XmlDomElement* child = properties->appendTextChild("default_value", mDefaultValues.value(locale));
        child->setAttribute("locale", locale);
    }
    foreach (const QString& norm, mPrefixes.keys()) {
        XmlDomElement* child = properties->appendTextChild("prefix", mPrefixes.value(norm));
        child->setAttribute("norm", norm);
    }
    XmlDomElement* signalsNode = root->appendChild("signals");
    foreach (const ComponentSignal* signal, mSignals)
        signalsNode->appendChild(signal->serializeToXmlDomElement());
    XmlDomElement* symbol_variants = root->appendChild("symbol_variants");
    symbol_variants->setAttribute("default", mDefaultSymbolVariantUuid);
    foreach (const ComponentSymbolVariant* variant, mSymbolVariants)
        symbol_variants->appendChild(variant->serializeToXmlDomElement());
    //XmlDomElement* spice_models = root->appendChild("spice_models");
    //Q_UNUSED(spice_models);
    return root.take();
}

bool Component::checkAttributesValidity() const noexcept
{
    if (!LibraryElement::checkAttributesValidity())             return false;
    if (!mDefaultValues.contains("en_US"))                      return false;
    if (!mPrefixes.contains(QString("")))                       return false;
    if (mSymbolVariants.isEmpty())                              return false;
    if (mDefaultSymbolVariantUuid.isNull())                     return false;
    if (!getSymbolVariantByUuid(mDefaultSymbolVariantUuid))     return false;
    foreach (ComponentSymbolVariant* var, mSymbolVariants) {
        foreach (ComponentSymbolVariantItem* item, var->getItems()) {
            foreach (ComponentPinSignalMapItem* map, item->getPinSignalMappings()) {
                if (!getSignalByUuid(map->getSignalUuid()))     return false;
            }
        }
    }
    return true;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace library
} // namespace librepcb
