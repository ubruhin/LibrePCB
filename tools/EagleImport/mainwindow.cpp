#include <QtCore>
#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <librepcbcommon/fileio/smartxmlfile.h>
#include <librepcbcommon/fileio/xmldomdocument.h>
#include <librepcblibrary/sym/symbol.h>
#include <librepcblibrary/pkg/footprint.h>
#include <librepcblibrary/pkg/package.h>
#include <librepcblibrary/dev/device.h>
#include <librepcblibrary/cmp/component.h>
#include <librepcbcommon/boardlayer.h>
#include <librepcbcommon/schematiclayer.h>
#include "polygonsimplifier.h"

namespace librepcb {
using namespace library;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lblUuidList->setText(UUID_LIST_FILEPATH);

    QSettings s;
    restoreGeometry(s.value("mainwindow/geometry").toByteArray());
    restoreState(s.value("mainwindow/state").toByteArray());
    mlastInputDirectory = s.value("mainwindow/last_input_directory").toString();
    ui->edtDuplicateFolders->setText(s.value("mainwindow/last_library_directory").toString());
    ui->input->addItems(s.value("mainwindow/input").toStringList());
    ui->output->setText(s.value("mainwindow/output").toString());

    reset();
}

MainWindow::~MainWindow()
{
    QStringList inputList;
    for (int i = 0; i < ui->input->count(); i++)
        inputList.append(ui->input->item(i)->text());

    QSettings s;
    s.setValue("mainwindow/geometry", saveGeometry());
    s.setValue("mainwindow/state", saveState());
    s.setValue("mainwindow/last_input_directory", mlastInputDirectory);
    s.setValue("mainwindow/last_library_directory", ui->edtDuplicateFolders->text());
    s.setValue("mainwindow/input", QVariant::fromValue(inputList));
    s.setValue("mainwindow/output", ui->output->text());

    delete ui;
}

void MainWindow::reset()
{
    mAbortConversion = false;
    mReadedElementsCount = 0;
    mConvertedElementsCount = 0;

    ui->errors->clear();
    ui->pbarElements->setValue(0);
    ui->pbarElements->setMaximum(0);
    ui->pbarFiles->setValue(0);
    ui->pbarFiles->setMaximum(ui->input->count());
    ui->lblConvertedElements->setText("0 of 0");
}

void MainWindow::addError(const QString& msg, const FilePath& inputFile, int inputLine)
{
    ui->errors->addItem(QString("%1 (%2:%3)").arg(msg).arg(inputFile.toNative()).arg(inputLine));
}

Uuid MainWindow::getOrCreateUuid(QSettings& outputSettings, const FilePath& filepath,
                                  const QString& cat, const QString& key1, const QString& key2)
{
    QString allowedChars("_-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    QString settingsKey = filepath.getFilename() % '_' % key1 % '_' % key2;
    settingsKey.replace("{", "");
    settingsKey.replace("}", "");
    settingsKey.replace(" ", "_");
    for (int i=0; i<settingsKey.length(); i++)
    {
        if (!allowedChars.contains(settingsKey[i]))
            settingsKey.replace(i, 1, QString("__U%1__").arg(QString::number(settingsKey[i].unicode(), 16).toUpper()));
    }
    settingsKey.prepend(cat % '/');

    Uuid uuid = Uuid::createRandom();
    QString value = outputSettings.value(settingsKey).toString();
    if (!value.isEmpty()) uuid = Uuid(value); //Uuid(QString("{%1}").arg(value));

    if (uuid.isNull())
    {
        addError("Invalid UUID in *.ini file: " % settingsKey, filepath);
        return Uuid::createRandom();
    }
    outputSettings.setValue(settingsKey, uuid.toStr());
    return uuid;
}

QString MainWindow::createDescription(const FilePath& filepath, const QString& name)
{
    return QString("\n\nThis element was automatically imported from Eagle\n"
                   "Filepath: %1\nName: %2\n"
                   "NOTE: Remove this text after manual rework!")
            .arg(filepath.getFilename(), name);
}

int MainWindow::convertSchematicLayerId(int eagleLayerId)
{
    switch (eagleLayerId)
    {
        case 93: return SchematicLayer::LayerID::SymbolPinNames;
        case 94: return SchematicLayer::LayerID::SymbolOutlines;
        case 95: return SchematicLayer::LayerID::ComponentNames;
        case 96: return SchematicLayer::LayerID::ComponentValues;
        case 99: return SchematicLayer::LayerID::OriginCrosses; // ???
        default: throw Exception(__FILE__, __LINE__, QString("Invalid schematic layer: %1").arg(eagleLayerId));
    }
}

int MainWindow::convertBoardLayerId(int eagleLayerId)
{
    switch (eagleLayerId)
    {
        case 1:  return BoardLayer::LayerID::TopCopper;
        case 16: return BoardLayer::LayerID::BottomCopper;
        case 20: return BoardLayer::LayerID::BoardOutlines;
        case 21: return BoardLayer::LayerID::TopOverlay;
        case 22: return BoardLayer::LayerID::BottomDeviceOutlines;
        case 25: return BoardLayer::LayerID::TopOverlayNames;
        case 27: return BoardLayer::LayerID::TopOverlayValues;
        case 29: return BoardLayer::LayerID::TopStopMask;
        case 31: return BoardLayer::LayerID::TopPaste;
        case 35: return BoardLayer::LayerID::TopGlue;
        case 39: return BoardLayer::LayerID::TopDeviceKeepout;
        case 41: return BoardLayer::LayerID::TopCopperRestrict;
        case 42: return BoardLayer::LayerID::BottomCopperRestrict;
        case 43: return BoardLayer::LayerID::ViaRestrict;
        case 46: return BoardLayer::LayerID::BoardOutlines; // milling
        case 48: return BoardLayer::LayerID::TopDeviceOutlines; // document
        case 49: return BoardLayer::LayerID::TopDeviceOriginCrosses; // reference
        case 51: return BoardLayer::LayerID::TopDeviceOutlines;
        case 52: return BoardLayer::LayerID::BottomDeviceOutlines;
        default: throw Exception(__FILE__, __LINE__, QString("Invalid board layer: %1").arg(eagleLayerId));
    }
}

void MainWindow::convertAllFiles(ConvertFileType_t type)
{
    reset();

    // create output directory
    FilePath outputDir(ui->output->text());
    outputDir.mkPath();

    QSettings outputSettings(UUID_LIST_FILEPATH, QSettings::IniFormat);

    for (int i = 0; i < ui->input->count(); i++)
    {
        FilePath filepath(ui->input->item(i)->text());
        if (!filepath.isExistingFile())
        {
            addError("File not found: " % filepath.toNative());
            continue;
        }

        convertFile(type, outputSettings, filepath);
        ui->pbarFiles->setValue(i + 1);

        if (mAbortConversion)
            break;
    }
}

void MainWindow::convertFile(ConvertFileType_t type, QSettings& outputSettings, const FilePath& filepath)
{
    try
    {
        // Check input file and read XML content
        SmartXmlFile file(filepath, false, true);
        QSharedPointer<XmlDomDocument> doc = file.parseFileAndBuildDomTree(false);
        XmlDomElement* node = doc->getRoot().getFirstChild("drawing/library", true, true);

        switch (type)
        {
            case ConvertFileType_t::Symbols_to_Symbols:
                node = node->getFirstChild("symbols", true);
                break;
            case ConvertFileType_t::Packages_to_PackagesAndDevices:
                node = node->getFirstChild("packages", true);
                break;
            case ConvertFileType_t::Devices_to_Components:
                node = node->getFirstChild("devicesets", true);
                break;
            default:
                throw Exception(__FILE__, __LINE__);
        }

        ui->pbarElements->setValue(0);
        ui->pbarElements->setMaximum(node->getChildCount());

        // Convert Elements
        for (XmlDomElement* child = node->getFirstChild(); child; child = child->getNextSibling())
        {
            bool success;
            if (child->getName() == "symbol")
                success = convertSymbol(outputSettings, filepath, child);
            else if (child->getName() == "package")
                success = convertPackage(outputSettings, filepath, child);
            else if (child->getName() == "deviceset")
                success = convertDevice(outputSettings, filepath, child);
            else
                throw Exception(__FILE__, __LINE__, child->getName());

            mReadedElementsCount++;
            if (success) mConvertedElementsCount++;
            ui->pbarElements->setValue(ui->pbarElements->value() + 1);
            ui->lblConvertedElements->setText(QString("%1 of %2").arg(mConvertedElementsCount)
                                                                 .arg(mReadedElementsCount));
        }
    }
    catch (Exception& e)
    {
        addError(e.getUserMsg() % " [" % e.getDebugMsg() % "]");
        return;
    }
}

bool MainWindow::convertSymbol(QSettings& outputSettings, const FilePath& filepath, XmlDomElement* node)
{
    try
    {
        QString name = node->getAttribute<QString>("name", true);
        QString desc = createDescription(filepath, name);
        Uuid uuid = getOrCreateUuid(outputSettings, filepath, "symbols", name);
        bool rotate180 = false;
        if (filepath.getFilename() == "con-lsta.lbr" && name.startsWith("FE")) rotate180 = true;
        if (filepath.getFilename() == "con-lstb.lbr" && name.startsWith("MA")) rotate180 = true;

        // create symbol
        Symbol* symbol = new Symbol(uuid, Version("0.1"), "LibrePCB", name, desc, "");

        for (XmlDomElement* child = node->getFirstChild(); child; child = child->getNextSibling())
        {
            if (child->getName() == "wire")
            {
                int layerId = convertSchematicLayerId(child->getAttribute<uint>("layer", true));
                bool fill = false;
                bool isGrabArea = true;
                Length lineWidth = child->getAttribute<Length>("width", true);
                Point startpos = Point(child->getAttribute<Length>("x1", true), child->getAttribute<Length>("y1", true));
                Point endpos = Point(child->getAttribute<Length>("x2", true), child->getAttribute<Length>("y2", true));
                Angle angle = child->hasAttribute("curve") ? child->getAttribute<Angle>("curve", true) : Angle(0);
                if (rotate180) {
                    startpos = Point(-startpos.getX(), -startpos.getY());
                    endpos = Point(-endpos.getX(), -endpos.getY());
                }
                Polygon* polygon = Polygon::createCurve(layerId, lineWidth, fill, isGrabArea,
                                                        startpos, endpos, angle);
                symbol->addPolygon(*polygon);
            }
            else if (child->getName() == "rectangle")
            {
                int layerId = convertSchematicLayerId(child->getAttribute<uint>("layer", true));
                bool fill = true;
                bool isGrabArea = true;
                Length lineWidth(0);
                if (child->hasAttribute("width")) lineWidth = child->getAttribute<Length>("width", true);
                Point p1(child->getAttribute<Length>("x1", true), child->getAttribute<Length>("y1", true));
                Point p2(child->getAttribute<Length>("x2", true), child->getAttribute<Length>("y1", true));
                Point p3(child->getAttribute<Length>("x2", true), child->getAttribute<Length>("y2", true));
                Point p4(child->getAttribute<Length>("x1", true), child->getAttribute<Length>("y2", true));
                Polygon* polygon = new Polygon(layerId, lineWidth, fill, isGrabArea, p1);
                polygon->appendSegment(*new PolygonSegment(p2, Angle::deg0()));
                polygon->appendSegment(*new PolygonSegment(p3, Angle::deg0()));
                polygon->appendSegment(*new PolygonSegment(p4, Angle::deg0()));
                polygon->appendSegment(*new PolygonSegment(p1, Angle::deg0()));
                symbol->addPolygon(*polygon);
            }
            else if (child->getName() == "polygon")
            {
                int layerId = convertSchematicLayerId(child->getAttribute<uint>("layer", true));
                bool fill = false;
                bool isGrabArea = true;
                Length lineWidth(0);
                if (child->hasAttribute("width")) lineWidth = child->getAttribute<Length>("width", true);
                Polygon* polygon = new Polygon(layerId, lineWidth, fill, isGrabArea, Point(0, 0));
                for (XmlDomElement* vertex = child->getFirstChild(); vertex; vertex = vertex->getNextSibling()) {
                    Point p(vertex->getAttribute<Length>("x", true), vertex->getAttribute<Length>("y", true));
                    if (vertex == child->getFirstChild())
                        polygon->setStartPos(p);
                    else
                        polygon->appendSegment(*new PolygonSegment(p, Angle::deg0()));
                }
                polygon->close();
                symbol->addPolygon(*polygon);
            }
            else if (child->getName() == "circle")
            {
                int layerId = convertSchematicLayerId(child->getAttribute<uint>("layer", true));
                Length radius(child->getAttribute<Length>("radius", true));
                Point center(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                Length lineWidth = child->getAttribute<Length>("width", true);
                bool fill = (lineWidth == 0);
                bool isGrabArea = true;
                Ellipse* ellipse = new Ellipse(layerId, lineWidth, fill, isGrabArea,
                                               center, radius, radius, Angle::deg0());
                symbol->addEllipse(*ellipse);
            }
            else if (child->getName() == "text")
            {
                int layerId = convertSchematicLayerId(child->getAttribute<uint>("layer", true));
                QString textStr = child->getText<QString>(true);
                Length height = child->getAttribute<Length>("size", true)*2;
                if (textStr == ">NAME") {
                    textStr = "${SYM::NAME}";
                    height = Length::fromMm(3.175);
                } else if (textStr == ">VALUE") {
                    textStr = "${CMP::VALUE}";
                    height = Length::fromMm(2.5);
                }
                Point pos = Point(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                int angleDeg = 0;
                if (child->hasAttribute("rot")) angleDeg = child->getAttribute<QString>("rot", true).remove("R").toInt();
                if (rotate180) {
                    pos = Point(-pos.getX(), -pos.getY());
                    angleDeg += 180;
                }
                Angle rot = Angle::fromDeg(angleDeg);
                Alignment align(HAlign::left(), VAlign::bottom());
                Text* text = new Text(layerId, textStr, pos, rot, height, align);
                symbol->addText(*text);
            }
            else if (child->getName() == "pin")
            {
                Uuid pinUuid = getOrCreateUuid(outputSettings, filepath, "symbol_pins", uuid.toStr(), child->getAttribute<QString>("name", true));
                QString name = child->getAttribute<QString>("name", true);
                Point pos = Point(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                Length len(7620000);
                if (child->hasAttribute("length")) {
                    if (child->getAttribute<QString>("length", true) == "point")
                        len.setLengthNm(0);
                    else if (child->getAttribute<QString>("length", true) == "short")
                        len.setLengthNm(2540000);
                    else if (child->getAttribute<QString>("length", true) == "middle")
                        len.setLengthNm(5080000);
                    else if (child->getAttribute<QString>("length", true) == "long")
                        len.setLengthNm(7620000);
                    else
                        throw Exception(__FILE__, __LINE__, "Invalid symbol pin length: " % child->getAttribute<QString>("length", false));
                }
                int angleDeg = 0;
                if (child->hasAttribute("rot")) angleDeg = child->getAttribute<QString>("rot", true).remove("R").toInt();
                if (rotate180) {
                    pos = Point(-pos.getX(), -pos.getY());
                    angleDeg += 180;
                }
                Angle rot = Angle::fromDeg(angleDeg);
                SymbolPin* pin = new SymbolPin(pinUuid, name, pos, len, rot);
                symbol->addPin(*pin);
            }
            else
            {
                addError(QString("Unknown node name: %1/%2").arg(node->getName()).arg(child->getName()), filepath);
                return false;
            }
        }

        // convert line rects to polygon rects
        PolygonSimplifier<Symbol> polygonSimplifier(*symbol);
        polygonSimplifier.convertLineRectsToPolygonRects(false, true);

        // save symbol to file
        symbol->saveTo(FilePath(QString("%1/sym").arg(ui->output->text())));
        delete symbol;
    }
    catch (Exception& e)
    {
        addError(e.getUserMsg() % " [" % e.getDebugMsg() % "]");
        return false;
    }

    return true;
}

bool MainWindow::convertPackage(QSettings& outputSettings, const FilePath& filepath, XmlDomElement* node)
{
    try
    {
        QString name = node->getAttribute<QString>("name", true);
        QString desc = node->getFirstChild("description", false) ? node->getFirstChild("description", true)->getText<QString>(false) : "";
        desc.append(createDescription(filepath, name));
        bool rotate180 = false;
        //if (filepath.getFilename() == "con-lsta.lbr" && name.startsWith("FE")) rotate180 = true;
        //if (filepath.getFilename() == "con-lstb.lbr" && name.startsWith("MA")) rotate180 = true;

        // create footprint
        Uuid fptUuid = getOrCreateUuid(outputSettings, filepath, "packages_to_footprints", name);
        Footprint* footprint = new Footprint(fptUuid, "default", "");

        // create package
        Uuid pkgUuid = getOrCreateUuid(outputSettings, filepath, "packages_to_packages", name);
        Package* package = new Package(pkgUuid, Version("0.1"), "LibrePCB", name, desc, "");
        package->addFootprint(*footprint);

        for (XmlDomElement* child = node->getFirstChild(); child; child = child->getNextSibling())
        {
            if (child->getName() == "description")
            {
                // nothing to do
            }
            else if (child->getName() == "wire")
            {
                int layerId = convertBoardLayerId(child->getAttribute<uint>("layer", true));
                bool fill = false;
                bool isGrabArea = true;
                Length lineWidth = child->getAttribute<Length>("width", true);
                Point startpos = Point(child->getAttribute<Length>("x1", true), child->getAttribute<Length>("y1", true));
                Point endpos = Point(child->getAttribute<Length>("x2", true), child->getAttribute<Length>("y2", true));
                Angle angle = child->hasAttribute("curve") ? child->getAttribute<Angle>("curve", true) : Angle(0);
                if (rotate180) {
                    startpos = Point(-startpos.getX(), -startpos.getY());
                    endpos = Point(-endpos.getX(), -endpos.getY());
                }
                Polygon* polygon = Polygon::createCurve(layerId, lineWidth, fill, isGrabArea,
                                                        startpos, endpos, angle);
                footprint->addPolygon(*polygon);
            }
            else if (child->getName() == "rectangle")
            {
                int layerId = convertBoardLayerId(child->getAttribute<uint>("layer", true));
                bool fill = true;
                bool isGrabArea = true;
                Length lineWidth(0);
                if (child->hasAttribute("width")) lineWidth = child->getAttribute<Length>("width", true);
                Point p1(child->getAttribute<Length>("x1", true), child->getAttribute<Length>("y1", true));
                Point p2(child->getAttribute<Length>("x2", true), child->getAttribute<Length>("y1", true));
                Point p3(child->getAttribute<Length>("x2", true), child->getAttribute<Length>("y2", true));
                Point p4(child->getAttribute<Length>("x1", true), child->getAttribute<Length>("y2", true));
                Polygon* polygon = new Polygon(layerId, lineWidth, fill, isGrabArea, p1);
                polygon->appendSegment(*new PolygonSegment(p2, Angle::deg0()));
                polygon->appendSegment(*new PolygonSegment(p3, Angle::deg0()));
                polygon->appendSegment(*new PolygonSegment(p4, Angle::deg0()));
                polygon->appendSegment(*new PolygonSegment(p1, Angle::deg0()));
                footprint->addPolygon(*polygon);
            }
            else if (child->getName() == "polygon")
            {
                int layerId = convertBoardLayerId(child->getAttribute<uint>("layer", true));
                bool fill = false;
                bool isGrabArea = true;
                Length lineWidth(0);
                if (child->hasAttribute("width")) lineWidth = child->getAttribute<Length>("width", true);
                Polygon* polygon = new Polygon(layerId, lineWidth, fill, isGrabArea, Point(0, 0));
                for (XmlDomElement* vertex = child->getFirstChild(); vertex; vertex = vertex->getNextSibling()) {
                    Point p(vertex->getAttribute<Length>("x", true), vertex->getAttribute<Length>("y", true));
                    if (vertex == child->getFirstChild())
                        polygon->setStartPos(p);
                    else
                        polygon->appendSegment(*new PolygonSegment(p, Angle::deg0()));
                }
                polygon->close();
                footprint->addPolygon(*polygon);
            }
            else if (child->getName() == "circle")
            {
                int layerId = convertBoardLayerId(child->getAttribute<uint>("layer", true));
                Length radius(child->getAttribute<Length>("radius", true));
                Point center(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                Length lineWidth = child->getAttribute<Length>("width", true);
                bool fill = (lineWidth == 0);
                bool isGrabArea = true;
                Ellipse* ellipse = new Ellipse(layerId, lineWidth, fill, isGrabArea,
                                               center, radius, radius, Angle::deg0());
                footprint->addEllipse(*ellipse);
            }
            else if (child->getName() == "text")
            {
                int layerId = convertBoardLayerId(child->getAttribute<uint>("layer", true));
                QString textStr = child->getText<QString>(true);
                Length height = child->getAttribute<Length>("size", true)*2;
                if (textStr == ">NAME") {
                    textStr = "${CMP::NAME}";
                    height = Length::fromMm(2.5);
                } else if (textStr == ">VALUE") {
                    textStr = "${CMP::VALUE}";
                    height = Length::fromMm(2.0);
                }
                Point pos = Point(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                int angleDeg = 0;
                if (child->hasAttribute("rot")) angleDeg = child->getAttribute<QString>("rot", true).remove("R").toInt();
                if (rotate180) {
                    pos = Point(-pos.getX(), -pos.getY());
                    angleDeg += 180;
                }
                Angle rot = Angle::fromDeg(angleDeg);
                Alignment align(HAlign::left(), VAlign::bottom());
                Text* text = new Text(layerId, textStr, pos, rot, height, align);
                footprint->addText(*text);
            }
            else if (child->getName() == "pad")
            {
                Uuid padUuid = getOrCreateUuid(outputSettings, filepath, "package_pads", fptUuid.toStr(), child->getAttribute<QString>("name", true));
                QString name = child->getAttribute<QString>("name", true);
                // add package pad
                PackagePad* pkgPad = new PackagePad(padUuid, name);
                package->addPad(*pkgPad);
                // add footprint pad
                Point pos = Point(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                Length drillDiameter = child->getAttribute<Length>("drill", true);
                Length padDiameter = drillDiameter * 2;
                if (child->hasAttribute("diameter")) padDiameter = child->getAttribute<Length>("diameter", true);
                Length width = padDiameter;
                Length height = padDiameter;
                FootprintPadTht::Shape_t shape;
                QString shapeStr = child->hasAttribute("shape") ? child->getAttribute<QString>("shape", true) : "round";
                if (shapeStr == "square") {
                    shape = FootprintPadTht::Shape_t::RECT;
                } else if (shapeStr == "octagon") {
                    shape = FootprintPadTht::Shape_t::OCTAGON;
                } else if (shapeStr == "round") {
                    shape = FootprintPadTht::Shape_t::ROUND;
                } else if (shapeStr == "long") {
                    shape = FootprintPadTht::Shape_t::ROUND;
                    width = padDiameter * 2;
                } else {
                    throw Exception(__FILE__, __LINE__, "Invalid shape: " % shapeStr % " :: " % filepath.toStr());
                }
                int angleDeg = 0;
                if (child->hasAttribute("rot")) angleDeg = child->getAttribute<QString>("rot", true).remove("R").toInt();
                if (rotate180) {
                    pos = Point(-pos.getX(), -pos.getY());
                    angleDeg += 180;
                }
                Angle rot = Angle::fromDeg(angleDeg);
                FootprintPad* fptPad = new FootprintPadTht(padUuid, pos, rot, width,
                                                           height, shape, drillDiameter);
                footprint->addPad(*fptPad);
            }
            else if (child->getName() == "smd")
            {
                Uuid padUuid = getOrCreateUuid(outputSettings, filepath, "package_pads", fptUuid.toStr(), child->getAttribute<QString>("name", true));
                QString name = child->getAttribute<QString>("name", true);
                // add package pad
                PackagePad* pkgPad = new PackagePad(padUuid, name);
                package->addPad(*pkgPad);
                // add footprint pad
                int layerId = convertBoardLayerId(child->getAttribute<uint>("layer", true));
                FootprintPadSmt::BoardSide_t side;
                switch (layerId)
                {
                    case BoardLayer::TopCopper:     side = FootprintPadSmt::BoardSide_t::TOP; break;
                    case BoardLayer::BottomCopper:  side = FootprintPadSmt::BoardSide_t::BOTTOM; break;
                    default: throw Exception(__FILE__, __LINE__, QString("Invalid pad layer: %1").arg(layerId));
                }
                Point pos = Point(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                int angleDeg = 0;
                if (child->hasAttribute("rot")) angleDeg = child->getAttribute<QString>("rot", true).remove("R").toInt();
                if (rotate180) {
                    pos = Point(-pos.getX(), -pos.getY());
                    angleDeg += 180;
                }
                Angle rot = Angle::fromDeg(angleDeg);
                Length width = child->getAttribute<Length>("dx", true);
                Length height = child->getAttribute<Length>("dy", true);
                FootprintPad* fptPad = new FootprintPadSmt(padUuid, pos, rot, width,
                                                           height, side);
                footprint->addPad(*fptPad);
            }
            else if (child->getName() == "hole")
            {
                Point pos(child->getAttribute<Length>("x", true), child->getAttribute<Length>("y", true));
                Length diameter(child->getAttribute<Length>("drill", true));
                Hole* hole = new Hole(pos, diameter);
                footprint->addHole(*hole);
            }
            else
            {
                addError(QString("Unknown node name: %1/%2").arg(node->getName()).arg(child->getName()), filepath);
                return false;
            }
        }

        // convert line rects to polygon rects
        PolygonSimplifier<Footprint> polygonSimplifier(*footprint);
        polygonSimplifier.convertLineRectsToPolygonRects(false, true);

        // save package to file
        package->saveTo(FilePath(QString("%1/pkg").arg(ui->output->text())));

        // clean up
        delete package;
    }
    catch (Exception& e)
    {
        addError(e.getUserMsg() % " [" % e.getDebugMsg() % "]");
        return false;
    }

    return true;
}

bool MainWindow::convertDevice(QSettings& outputSettings, const FilePath& filepath, XmlDomElement* node)
{
    try
    {
        QString name = node->getAttribute<QString>("name", true);

        // abort if device name ends with "-US"
        if (name.endsWith("-US")) return false;

        Uuid uuid = getOrCreateUuid(outputSettings, filepath, "devices_to_components", name);
        QString desc = node->getFirstChild("description", false) ? node->getFirstChild("description", true)->getText<QString>(false) : "";
        desc.append(createDescription(filepath, name));

        // create  component
        Component* component = new Component(uuid, Version("0.1"), "LibrePCB", name, desc, "");

        // properties
        component->addDefaultValue("en_US", "");
        component->addPrefix("", node->hasAttribute("prefix") ? node->getAttribute<QString>("prefix", false) : "");

        // symbol variant
        Uuid symbVarUuid = getOrCreateUuid(outputSettings, filepath, "component_symbolvariants", uuid.toStr());
        ComponentSymbolVariant* symbvar = new ComponentSymbolVariant(symbVarUuid, "", "default", "");
        component->addSymbolVariant(*symbvar);

        // signals
        XmlDomElement* device = node->getFirstChild("devices/device", true, true);
        for (XmlDomElement* connect = device->getFirstChild("connects/connect", false, false);
             connect; connect = connect->getNextSibling())
        {
            QString gateName = connect->getAttribute<QString>("gate", true);
            QString pinName = connect->getAttribute<QString>("pin", true);
            if (pinName.contains("@")) pinName.truncate(pinName.indexOf("@"));
            if (pinName.contains("#")) pinName.truncate(pinName.indexOf("#"));
            Uuid signalUuid = getOrCreateUuid(outputSettings, filepath, "gatepins_to_componentsignals", uuid.toStr(), gateName % pinName);

            if (!component->getSignalByUuid(signalUuid))
            {
                // create signal
                ComponentSignal* signal = new ComponentSignal(signalUuid, pinName);
                component->addSignal(*signal);
            }
        }

        // symbol variant items
        for (XmlDomElement* gate = node->getFirstChild("gates/*", true, true); gate; gate = gate->getNextSibling())
        {
            QString gateName = gate->getAttribute<QString>("name", true);
            QString symbolName = gate->getAttribute<QString>("symbol", true);
            Uuid symbolUuid = getOrCreateUuid(outputSettings, filepath, "symbols", symbolName);

            // create symbol variant item
            Uuid symbVarItemUuid = getOrCreateUuid(outputSettings, filepath, "symbolgates_to_symbvaritems", uuid.toStr(), gateName);
            ComponentSymbolVariantItem* item = new ComponentSymbolVariantItem(symbVarItemUuid, symbolUuid, true, (gateName == "G$1") ? "" : gateName);

            // connect pins
            for (XmlDomElement* connect = device->getFirstChild("connects/connect", false, false);
                 connect; connect = connect->getNextSibling())
            {
                if (connect->getAttribute<QString>("gate", true) == gateName)
                {
                    QString pinName = connect->getAttribute<QString>("pin", true);
                    Uuid pinUuid = getOrCreateUuid(outputSettings, filepath, "symbol_pins", symbolUuid.toStr(), pinName);
                    if (pinName.contains("@")) pinName.truncate(pinName.indexOf("@"));
                    if (pinName.contains("#")) pinName.truncate(pinName.indexOf("#"));
                    Uuid signalUuid = getOrCreateUuid(outputSettings, filepath, "gatepins_to_componentsignals", uuid.toStr(), gateName % pinName);
                    ComponentPinSignalMapItem* map = new ComponentPinSignalMapItem(pinUuid,
                        signalUuid, ComponentPinSignalMapItem::PinDisplayType_t::COMPONENT_SIGNAL);
                    item->addPinSignalMapItem(*map);
                }
            }

            symbvar->addItem(*item);
        }

        // create devices
        for (XmlDomElement* deviceNode = node->getFirstChild("devices/*", true, true); deviceNode; deviceNode = deviceNode->getNextSibling())
        {
            if (!deviceNode->hasAttribute("package")) continue;

            QString deviceName = deviceNode->getAttribute<QString>("name", false);
            QString packageName = deviceNode->getAttribute<QString>("package", true);
            Uuid pkgUuid = getOrCreateUuid(outputSettings, filepath, "packages_to_packages", packageName);
            Uuid fptUuid = getOrCreateUuid(outputSettings, filepath, "packages_to_footprints", packageName);

            Uuid compUuid = getOrCreateUuid(outputSettings, filepath, "devices_to_devices", name, deviceName);
            QString compName = deviceName.isEmpty() ? name : QString("%1_%2").arg(name, deviceName);
            Device* device = new Device(compUuid, Version("0.1"), "LibrePCB", compName, desc, "");
            device->setComponentUuid(component->getUuid());
            device->setPackageUuid(pkgUuid);

            // connect pads
            for (XmlDomElement* connect = deviceNode->getFirstChild("connects/*", false, false);
                 connect; connect = connect->getNextSibling())
            {
                QString gateName = connect->getAttribute<QString>("gate", true);
                QString pinName = connect->getAttribute<QString>("pin", true);
                QString padNames = connect->getAttribute<QString>("pad", true);
                if (pinName.contains("@")) pinName.truncate(pinName.indexOf("@"));
                if (pinName.contains("#")) pinName.truncate(pinName.indexOf("#"));
                if (connect->hasAttribute("route"))
                {
                    if (connect->getAttribute<QString>("route", true) != "any")
                        addError(QString("Unknown connect route: %1/%2").arg(node->getName()).arg(connect->getAttribute<QString>("route", false)), filepath);
                }
                foreach (const QString& padName, padNames.split(" ", QString::SkipEmptyParts))
                {
                    Uuid padUuid = getOrCreateUuid(outputSettings, filepath, "package_pads", fptUuid.toStr(), padName);
                    Uuid signalUuid = getOrCreateUuid(outputSettings, filepath, "gatepins_to_componentsignals", uuid.toStr(), gateName % pinName);
                    device->addPadSignalMapping(padUuid, signalUuid);
                }
            }

            // save device
            device->saveTo(FilePath(QString("%1/dev").arg(ui->output->text())));
            delete device;
        }

        // save component to file
        component->saveTo(FilePath(QString("%1/cmp").arg(ui->output->text())));
        delete component;
    }
    catch (Exception& e)
    {
        addError(e.getUserMsg() % " [" % e.getDebugMsg() % "]");
        return false;
    }

    return true;
}

void MainWindow::on_inputBtn_clicked()
{
    ui->input->addItems(QFileDialog::getOpenFileNames(this, "Select Eagle Library Files",
                                                    mlastInputDirectory, "*.lbr"));
    ui->pbarFiles->setMaximum(ui->input->count());

    if (ui->input->count() > 0)
        mlastInputDirectory = QFileInfo(ui->input->item(0)->text()).absolutePath();
}

void MainWindow::on_outputBtn_clicked()
{
    ui->output->setText(QFileDialog::getExistingDirectory(this, "Select Output Directory",
                                                          ui->output->text()));
}

void MainWindow::on_btnAbort_clicked()
{
    mAbortConversion = true;
}

void MainWindow::on_btnConvertSymbols_clicked()
{
    convertAllFiles(ConvertFileType_t::Symbols_to_Symbols);
}

void MainWindow::on_btnConvertDevices_clicked()
{
    convertAllFiles(ConvertFileType_t::Devices_to_Components);
}

void MainWindow::on_pushButton_2_clicked()
{
    convertAllFiles(ConvertFileType_t::Packages_to_PackagesAndDevices);
}

void MainWindow::on_btnPathsFromIni_clicked()
{
    FilePath inputDir(QFileDialog::getExistingDirectory(this, "Select Input Folder", mlastInputDirectory));
    if (!inputDir.isExistingDir()) return;

    QSettings outputSettings(UUID_LIST_FILEPATH, QSettings::IniFormat);

    foreach (QString key, outputSettings.allKeys())
    {
        key.remove(0, key.indexOf("/")+1);
        key.remove(key.indexOf(".lbr")+4, key.length() - key.indexOf(".lbr") - 4);
        QString filepath = inputDir.getPathTo(key).toNative();

        bool exists = false;
        for (int i = 0; i < ui->input->count(); i++)
        {
            FilePath fp(ui->input->item(i)->text());
            if (fp.toNative() == filepath) exists = true;
        }
        if (!exists) ui->input->addItem(filepath);
    }
}

void MainWindow::on_toolButton_clicked()
{
    ui->input->clear();
}

void MainWindow::on_toolButton_2_clicked()
{
    qDeleteAll(ui->input->selectedItems());
}

void MainWindow::on_toolButton_3_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Library Folder", ui->edtDuplicateFolders->text());
    if (dir.isEmpty()) return;
    ui->edtDuplicateFolders->setText(dir);
}

void MainWindow::on_toolButton_4_clicked()
{
    if (ui->edtDuplicateFolders->text().isEmpty()) return;
    if (ui->output->text().isEmpty()) return;

    QDir libDir(ui->edtDuplicateFolders->text());
    QDir outDir(ui->output->text());

    uint count = 0;

    foreach (const QString& subdir1, libDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        if (subdir1 == "EagleImport") continue;
        if (subdir1 == "Staging_Area") continue;
        QDir repoDir = libDir.absoluteFilePath(subdir1);
        foreach (const QString& subdir2, repoDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            QDir repoSubDir = repoDir.absoluteFilePath(subdir2);
            foreach (const QString& elementDir, repoSubDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            {
                Uuid elementUuid(elementDir);
                if (elementUuid.isNull()) continue;

                QDir outDirElement = outDir.absoluteFilePath(subdir2 % "/" % elementDir);
                if (outDirElement.exists())
                {
                    qDebug() << outDirElement.absolutePath();
                    if (outDirElement.removeRecursively())
                        count++;
                    else
                        qDebug() << "Failed!";
                }
            }
        }
    }

    QMessageBox::information(this, "Duplicates Removed", QString("%1 duplicates removed.").arg(count));
}

}
