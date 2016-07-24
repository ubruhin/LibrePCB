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

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/
#include <QtCore>
#include "cmdremoveselectedschematicitems.h"
#include <librepcb/common/scopeguard.h>
#include <librepcb/project/project.h>
#include <librepcb/project/circuit/netsignal.h>
#include <librepcb/project/circuit/componentinstance.h>
#include <librepcb/project/circuit/componentsignalinstance.h>
#include <librepcb/project/circuit/cmd/cmdcomponentinstanceremove.h>
#include <librepcb/project/circuit/cmd/cmdnetsignalremove.h>
#include <librepcb/project/circuit/cmd/cmdcompsiginstsetnetsignal.h>
#include <librepcb/project/circuit/cmd/cmdnetsignaladd.h>
#include <librepcb/project/schematics/schematic.h>
#include <librepcb/project/schematics/items/si_symbol.h>
#include <librepcb/project/schematics/items/si_symbolpin.h>
#include <librepcb/project/schematics/items/si_netpoint.h>
#include <librepcb/project/schematics/items/si_netline.h>
#include <librepcb/project/schematics/items/si_netlabel.h>
#include <librepcb/project/schematics/items/si_netsegment.h>
#include <librepcb/project/schematics/cmd/cmdsymbolinstanceremove.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetsegmentremoveelements.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetsegmentaddelements.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetlabelremove.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetpointedit.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetsegmentedit.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetsegmentadd.h>
#include <librepcb/project/schematics/cmd/cmdschematicnetsegmentremove.h>
#include <librepcb/project/boards/board.h>
#include <librepcb/project/boards/items/bi_footprint.h>
#include <librepcb/project/boards/items/bi_footprintpad.h>
#include <librepcb/project/boards/items/bi_netpoint.h>
#include <librepcb/project/boards/items/bi_netline.h>
#include <librepcb/project/boards/items/bi_device.h>
#include <librepcb/project/boards/cmd/cmddeviceinstanceremove.h>
#include <librepcb/project/boards/cmd/cmdboardnetpointremove.h>
#include <librepcb/project/boards/cmd/cmdboardnetlineremove.h>
#include "cmdremoveunusednetsignals.h"
#include <librepcb/project/schematics/schematicselectionquery.h>
#include "cmdchangenetsignalofschematicnetsegment.h"
#include "cmdremovedevicefromboard.h"

/*****************************************************************************************
 *  Namespace
 ****************************************************************************************/
namespace librepcb {
namespace project {
namespace editor {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

CmdRemoveSelectedSchematicItems::CmdRemoveSelectedSchematicItems(Schematic& schematic) noexcept :
    UndoCommandGroup(tr("Remove Schematic Elements")), mSchematic(schematic)
{
}

CmdRemoveSelectedSchematicItems::~CmdRemoveSelectedSchematicItems() noexcept
{
}

/*****************************************************************************************
 *  Inherited from UndoCommand
 ****************************************************************************************/

bool CmdRemoveSelectedSchematicItems::performExecute()
{
    // if an error occurs, undo all already executed child commands
    auto undoScopeGuard = scopeGuard([&](){performUndo();});

    // get all selected items
    QScopedPointer<SchematicSelectionQuery> query(mSchematic.createSelectionQuery());
    query->addSelectedSymbols();
    query->addSelectedNetLines(SchematicSelectionQuery::NetLineFilter::All);
    query->addSelectedNetLabels();
    query->addNetPointsOfNetLines(SchematicSelectionQuery::NetLineFilter::All,
                                  SchematicSelectionQuery::NetPointFilter::AllConnectedLinesSelected);

    // clear selection because these items will be removed now
    mSchematic.clearSelection();

    // determine all affected netsegments and their items
    NetSegmentItemList netSegmentItems;
    foreach (SI_NetPoint* netpoint, query->getNetPoints()) {
        NetSegmentItems& items = netSegmentItems[&netpoint->getNetSegment()];
        items.netpoints.insert(netpoint);
    }
    foreach (SI_NetLine* netline, query->getNetLines()) {
        NetSegmentItems& items = netSegmentItems[&netline->getNetSegment()];
        items.netlines.insert(netline);
    }
    foreach (SI_NetLabel* netlabel, query->getNetLabels()) {
        NetSegmentItems& items = netSegmentItems[&netlabel->getNetSegment()];
        items.netlabels.insert(netlabel);
    }

    // remove netlines/netpoints/netlabels/netsegments
    foreach (SI_NetSegment* netsegment, netSegmentItems.keys()) {
        const NetSegmentItems& items = netSegmentItems.value(netsegment);
        if (items.netlines.count() == 0) {
            // only netlabels of this netsegment are selected
            Q_ASSERT(items.netpoints.count() == 0);
            foreach (SI_NetLabel* netlabel, items.netlabels) { Q_ASSERT(netlabel);
                removeNetLabel(*netlabel); // can throw
            }
        } else if (items.netlines.count() == netsegment->getNetLines().count()) {
            // all lines of the netsegment are selected --> remove the whole netsegment
            removeNetSegment(*netsegment); // can throw
        } else if (items.netlines.count() < netsegment->getNetLines().count()) {
            // only some of the netsegment's lines are selected --> split up the netsegment
            splitUpNetSegment(*netsegment, items); // can throw
        } else {
            throw new LogicError(__FILE__, __LINE__);
        }
    }

    // remove all symbols, devices and component instances
    foreach (SI_Symbol* symbol, query->getSymbols()) {
        removeSymbol(*symbol); // can throw
    }

    // remove netsignals which are no longer required
    if (getChildCount() > 0) {
        execNewChildCmd(new CmdRemoveUnusedNetSignals(mSchematic.getProject().getCircuit())); // can throw
    }

    undoScopeGuard.dismiss(); // no undo required
    return (getChildCount() > 0);
}

void CmdRemoveSelectedSchematicItems::removeNetSegment(SI_NetSegment& netsegment)
{
    // determine component signal instances to be disconnected
    QList<ComponentSignalInstance*> cmpSigInstToBeDisconnected;
    foreach (SI_NetPoint* netpoint, netsegment.getNetPoints()) {
        // check if this was the last symbol pin of the component signal
        ComponentSignalInstance* cmpSig = getCmpSigInstToBeDisconnected(*netpoint);
        if (cmpSig) {
            cmpSigInstToBeDisconnected.append(cmpSig);
        }
    }

    // remove netsegment
    execNewChildCmd(new CmdSchematicNetSegmentRemove(netsegment)); // can throw

    // disconnect component signal instances
    foreach (ComponentSignalInstance* cmpSig, cmpSigInstToBeDisconnected) {
        disconnectComponentSignalInstance(*cmpSig); // can throw
    }
}

void CmdRemoveSelectedSchematicItems::splitUpNetSegment(SI_NetSegment& netsegment,
                                                        const NetSegmentItems& selectedItems)
{
    // determine all resulting sub-netsegments
    QList<NetSegmentItems> subsegments = getNonCohesiveNetSegmentSubSegments(
                                             netsegment, selectedItems);

    // remove all selected netlines & netpoints
    CmdSchematicNetSegmentRemoveElements* cmd = new CmdSchematicNetSegmentRemoveElements(netsegment);
    QSet<ComponentSignalInstance*> cmpSigsToBeDisconnected;
    foreach (SI_NetPoint* netpoint, selectedItems.netpoints) {
        cmd->removeNetPoint(*netpoint);
        // check if this was the last symbol pin of the component signal
        ComponentSignalInstance* cmpSig = getCmpSigInstToBeDisconnected(*netpoint);
        if (cmpSig) {
            cmpSigsToBeDisconnected.insert(cmpSig);
        }
    }
    foreach (SI_NetLine* netline, selectedItems.netlines) {
        cmd->removeNetLine(*netline);
    }
    //execNewChildCmd(cmd); // can throw

    // remove the whole netsegment
    execNewChildCmd(new CmdSchematicNetSegmentRemove(netsegment));

    // disconnect component signal instances
    foreach (ComponentSignalInstance* cmpSig, cmpSigsToBeDisconnected) {
        disconnectComponentSignalInstance(*cmpSig); // can throw
    }

    // create new sub-netsegments
    foreach (const NetSegmentItems& subsegment, subsegments) {
        createNewSubNetSegment(netsegment, subsegment); // can throw
    }
}

void CmdRemoveSelectedSchematicItems::createNewSubNetSegment(SI_NetSegment& netsegment,
                                                             const NetSegmentItems& items)
{
    // TODO: create new netsignal
    //CmdNetSignalAdd* cmdAddNetSignal = new CmdNetSignalAdd(netsegment.getCircuit(),
    //                                   netsegment.getNetSignal().getNetClass());
    //execNewChildCmd(cmdAddNetSignal); // can throw
    //NetSignal* newNetSignal = cmdAddNetSignal->getNetSignal(); Q_ASSERT(newNetSignal);
    NetSignal* newNetSignal = &netsegment.getNetSignal();

    // create new netsegment
    CmdSchematicNetSegmentAdd* cmdAddNetSegment = new CmdSchematicNetSegmentAdd(
                                                  netsegment.getSchematic(), *newNetSignal);
    execNewChildCmd(cmdAddNetSegment); // can throw
    SI_NetSegment* newNetSegment = cmdAddNetSegment->getNetSegment(); Q_ASSERT(newNetSegment);

    // create new netpoints and netlines
    CmdSchematicNetSegmentAddElements* cmdAddElements = new CmdSchematicNetSegmentAddElements(*newNetSegment);
    QHash<const SI_NetPoint*, SI_NetPoint*> netPointMap;
    foreach (const SI_NetPoint* netpoint, items.netpoints) {
        SI_NetPoint* newNetPoint;
        if (netpoint->isAttachedToPin()) {
            SI_SymbolPin* pin = netpoint->getSymbolPin(); Q_ASSERT(pin);
            newNetPoint = cmdAddElements->addNetPoint(*pin);
        } else {
            newNetPoint = cmdAddElements->addNetPoint(netpoint->getPosition());
        }
        Q_ASSERT(newNetPoint);
        netPointMap.insert(netpoint, newNetPoint);
    }
    foreach (const SI_NetLine* netline, items.netlines) {
        SI_NetPoint* p1 = netPointMap.value(&netline->getStartPoint()); Q_ASSERT(p1);
        SI_NetPoint* p2 = netPointMap.value(&netline->getEndPoint()); Q_ASSERT(p2);
        SI_NetLine* newNetLine = cmdAddElements->addNetLine(*p1, *p2);
        Q_ASSERT(newNetLine);
    }
    execNewChildCmd(cmdAddElements); // can throw
}

void CmdRemoveSelectedSchematicItems::removeNetLabel(SI_NetLabel& netlabel)
{
    // remove the netlabel
    execNewChildCmd(new CmdSchematicNetLabelRemove(netlabel)); // can throw

    // was this the last label of the netsegment?
    if (netlabel.getNetSegment().getNetLabels().isEmpty()) {
        // are there any forced net names of the net segment?
        CmdNetSignalAdd* cmd;
        NetClass& netclass = netlabel.getNetSignalOfNetSegment().getNetClass();
        QSet<QString> names = netlabel.getNetSegment().getForcedNetNames();
        if (names.isEmpty()) {
            // create new netsignal with auto-name
            cmd = new CmdNetSignalAdd(mSchematic.getProject().getCircuit(), netclass);
        } else {
            // create new netsignal with (first) forced name
            cmd = new CmdNetSignalAdd(mSchematic.getProject().getCircuit(), netclass, names.values().first());
        }
        execNewChildCmd(cmd); // can throw
        NetSignal* netsignal = cmd->getNetSignal(); Q_ASSERT(netsignal);
        // change the netsignal of the netsegment
        execNewChildCmd(new CmdChangeNetSignalOfSchematicNetSegment(netlabel.getNetSegment(), *netsignal)); // can throw
    }
}


void CmdRemoveSelectedSchematicItems::removeSymbol(SI_Symbol& symbol)
{
    // disconnect all netpoints from pins
    foreach (SI_SymbolPin* pin, symbol.getPins()) {
        SI_NetPoint* netpoint = pin->getNetPoint();
        if (netpoint) {
            detachNetPointFromSymbolPin(*netpoint); // can throw
        }
    }

    // remove symbol
    execNewChildCmd(new CmdSymbolInstanceRemove(mSchematic, symbol)); // can throw

    // do we also need to remove the component instance?
    ComponentInstance& component = symbol.getComponentInstance();
    if (component.getPlacedSymbolsCount() == 0) {
        foreach (Board* board, mSchematic.getProject().getBoards()) {
            BI_Device* device = board->getDeviceInstanceByComponentUuid(component.getUuid());
            if (device) {
                execNewChildCmd(new CmdRemoveDeviceFromBoard(*device)); // can throw
            }
        }
        execNewChildCmd(new CmdComponentInstanceRemove(mSchematic.getProject().getCircuit(),
                                                      component)); // can throw
    }
}

void CmdRemoveSelectedSchematicItems::detachNetPointFromSymbolPin(SI_NetPoint& netpoint)
{
    Q_ASSERT(netpoint.isAttachedToPin());
    SI_SymbolPin* pin = netpoint.getSymbolPin(); Q_ASSERT(pin);

    // remove netsegment
    execNewChildCmd(new CmdSchematicNetSegmentRemove(netpoint.getNetSegment())); // can throw

    // detach netpoint from symbol pin
    CmdSchematicNetPointEdit* cmd = new CmdSchematicNetPointEdit(netpoint);
    cmd->setPinToAttach(nullptr);
    execNewChildCmd(cmd); // can throw

    // re-add netsegment
    execNewChildCmd(new CmdSchematicNetSegmentAdd(netpoint.getNetSegment())); // can throw
}

ComponentSignalInstance* CmdRemoveSelectedSchematicItems::getCmpSigInstToBeDisconnected(
        SI_NetPoint& netpoint) const noexcept
{
    SI_SymbolPin* symbolPin = netpoint.getSymbolPin();
    if (!symbolPin) return nullptr;

    ComponentSignalInstance* cmpSig = symbolPin->getComponentSignalInstance();
    if (!cmpSig) return nullptr;

    foreach (const SI_SymbolPin* pin, cmpSig->getRegisteredSymbolPins()) {
        if ((pin != symbolPin) && (pin->getNetPoint())) {
            return nullptr;
        }
    }
    return cmpSig;
}

void CmdRemoveSelectedSchematicItems::disconnectComponentSignalInstance(
        ComponentSignalInstance& signal)
{
    // disconnect board items
    foreach (BI_FootprintPad* pad, signal.getRegisteredFootprintPads()) {
        foreach (BI_NetPoint* netpoint, pad->getNetPoints()) {
            foreach (BI_NetLine* netline, netpoint->getLines()) {
                execNewChildCmd(new CmdBoardNetLineRemove(*netline)); // can throw
            }
            execNewChildCmd(new CmdBoardNetPointRemove(*netpoint)); // can throw
        }
    }

    // disconnect the component signal instance from the net signal
    execNewChildCmd(new CmdCompSigInstSetNetSignal(signal, nullptr)); // can throw
}

QList<CmdRemoveSelectedSchematicItems::NetSegmentItems>
CmdRemoveSelectedSchematicItems::getNonCohesiveNetSegmentSubSegments(
        SI_NetSegment& segment, const NetSegmentItems& removedItems) noexcept
{
    // get all netpoints, netlines and netlabels of the segment
    QSet<SI_NetPoint*> netpoints = segment.getNetPoints().toSet() - removedItems.netpoints;
    QSet<SI_NetLine*> netlines = segment.getNetLines().toSet() - removedItems.netlines;
    //QList<SI_NetLabel*> netlabels = segment.getNetLabels();

    // find all separate segments of the netsegment
    QList<NetSegmentItems> segments;
    while (netpoints.count() > 0) {
        NetSegmentItems seg;
        findAllConnectedNetPointsAndNetLines(*netpoints.values().first(),
                                             netpoints, netlines,
                                             seg.netpoints, seg.netlines);
        foreach (SI_NetPoint* p, seg.netpoints) netpoints.remove(p);
        foreach (SI_NetLine* l, seg.netlines) netlines.remove(l);
        segments.append(seg);
    }

    return segments;
}

void CmdRemoveSelectedSchematicItems::findAllConnectedNetPointsAndNetLines(SI_NetPoint& netpoint,
        QSet<SI_NetPoint*>& availableNetPoints, QSet<SI_NetLine*>& availableNetLines,
        QSet<SI_NetPoint*>& netpoints, QSet<SI_NetLine*>& netlines) const noexcept
{
    Q_ASSERT(!netpoints.contains(&netpoint));
    Q_ASSERT(availableNetPoints.contains(&netpoint));
    netpoints.insert(&netpoint);
    foreach (SI_NetLine* line, netpoint.getLines()) {
        if (availableNetLines.contains(line)) {
            netlines.insert(line);
            SI_NetPoint* p2 = line->getOtherPoint(netpoint); Q_ASSERT(p2);
            if ((availableNetPoints.contains(p2)) && (!netpoints.contains(p2))) {
                findAllConnectedNetPointsAndNetLines(*p2, availableNetPoints,
                                                     availableNetLines, netpoints, netlines);
            }
        }
    }
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace editor
} // namespace project
} // namespace librepcb
