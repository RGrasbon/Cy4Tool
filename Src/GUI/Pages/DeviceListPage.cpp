//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Geräteliste-Seite                                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QAction>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QKeyEvent>
#include "ui_DeviceListPage.h"
#include "MainFrame.h"
#include "Settings.h"
#include "Cy4Types.h"
#include "DeviceList.h"
#include "DeviceListPage.h"
#include "Cy4Net.h"

//////////////////////////////////////////////////////////////////////////////////////////
static int
toIndex(int row)
{
    return row;         // Aktuell 1:1
}

//////////////////////////////////////////////////////////////////////////////////////////
static int
toRow(int index)
{
    return index;       // Aktuell 1:1
}

//////////////////////////////////////////////////////////////////////////////////////////
static int
currentRow(const QTableView* view)
{
    QModelIndex modelIndex = view->currentIndex();
    int row = modelIndex.row();
    if (row < 0 || row > deviceList().size()) return INVALID;
    return row;
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
hideAllView(Ui::DeviceListPage* ui)
{
    ui->addDeviceView->hide();
    ui->changeAddressView->hide();
    ui->discoverDevicesView->hide();
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
adjustTableColumnWidth(QTableView* tableView)
{
    assert(tableView->model() != nullptr);
    // Listenelement initialisieren, Breite der Spalten festlegen
    {   QFontMetrics fontMetricsItem = tableView->fontMetrics();
        QFontMetrics fontMetricsHeader = tableView->horizontalHeader()->fontMetrics();
        static const QString cellTemplate[] = { "__000.000__", "__A00000__", "__V0.0__", "__B00000__" };
        int columnWidth[CARDINALITY(cellTemplate)];
        for (int i=0; i<CARDINALITY(cellTemplate); i++)
        {   int cellItemWidth = fontMetricsItem.horizontalAdvance(cellTemplate[i]);
            QString titleLabel = tableView->model()->headerData(i, Qt::Horizontal).toString();
            int headerItemWidth = fontMetricsHeader.horizontalAdvance(titleLabel);
            columnWidth[i] = max(headerItemWidth, cellItemWidth);
        }
        columnWidth[1] = max(columnWidth[1], columnWidth[3]);  // Spalte 1 und 3 sollen
        columnWidth[3] = columnWidth[1];                       // gleich breit sein
        for( int i=0; i<CARDINALITY(columnWidth); i++ )
        {   tableView->horizontalHeader()->resizeSection(i, columnWidth[i]);
        }
        tableView->horizontalHeader()->setStretchLastSection(true);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
DeviceListPage::RowDelegate::RowDelegate(QObject* parent)
                           : QStyledItemDelegate(parent)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::RowDelegate::paint(QPainter* painter,
                                             const QStyleOptionViewItem& option,
                                             const QModelIndex&          index) const
{
    // Trennstrich am oberen Rand der Zeile ziehen
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    painter->setPen(QPen(QColor(64, 64, 64), 1));
    painter->drawLine(option.rect.topLeft(), option.rect.topRight());
    painter->restore();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::updateSeparatorLines(void)
{
    for (int row = 0; row < deviceList().size(); row++)
    {   mUi->deviceView->setItemDelegateForRow(row, nullptr);
    }
    if (!deviceList().isEmpty())
    {   int netA = deviceList().at(toIndex(0)).address().net();
        for (int row = 1; row < deviceList().size(); row++)
        {   int netB = deviceList().at(toIndex(row)).address().net();
            if (netA != netB) mUi->deviceView->setItemDelegateForRow(row, mRowDelegate);
            netA = netB;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onChanged(void)
{
    QList<QModelIndex> list = mUi->deviceView->selectionModel()->selectedRows();
    int selectedCount = list.count();
    Cy4Address gatewayAddress = cy4Net().gateway();
    int gatewayIndex = deviceList().indexOf(cy4Net().gateway());
    // Aktuelle Situation feststellen
    struct Situation
    {   bool  listNotEmpty;       // Geräteliste besitzt Einträge
        bool  gatewayOnline;      // Gateway ist verbunden
        bool  gatewayOperable;    // Gateway ist verbunden einsatzbereit
        bool  selectedItems;      // Geräte in der Liste ausgewählt
        bool  currentOnline;      // Aktuelles Gerät ist vorhanden und verbunden
    } situation;
    {   situation.listNotEmpty    = !deviceList().isEmpty();
        situation.gatewayOnline   = gatewayAddress.isValid();
        situation.gatewayOperable = gatewayIndex == INVALID ? false : !deviceList().at(gatewayIndex).typePlate().isProgramLoader();
        situation.selectedItems   = selectedCount != 0;
        situation.currentOnline   = currentRow(mUi->deviceView) != INVALID && deviceList().isOnline(toIndex(currentRow(mUi->deviceView)));
    }
    // Steuerelemente freigeben/sperren
    {   const bool Y = true;              //   +----------------- listNotEmpty
        const bool _ = false;             //   |  +-------------- gatewayOnline
        struct                            //   |  |  +----------- gatewayOperable
        {   QObject*  object;             //   |  |  |  +-------- selectedItems
            Situation situation;          //   |  |  |  |  +----- currentOnline
        } enableXMap[] =                  //   |  |  |  |  |
        {   { mUi->refreshAllButton        , { Y, Y, Y, _, _, } },
            { mUi->discoverButton          , { _, Y, Y, _, _, } },
            { mUi->removeButton            , { Y, _, _, Y, _, } },
            { mUi->removeMenuItem          , { Y, _, _, Y, _, } },
            { mUi->refreshMenuItem         , { Y, Y, _, _, _, } },
            { mUi->restartMenuItem         , { Y, Y, _, _, Y, } },
            { mUi->uploadFirmwareButton    , { Y, Y, _, _, Y, } },
            { mUi->uploadFirmwareMenuItem  , { Y, Y, _, _, Y, } },
            { mUi->enterPgmLdrMenuItem     , { Y, Y, _, _, Y, } },
            { mUi->changeAddressMenuItem   , { Y, Y, _, _, Y, } },
            { mUi->markAsNewMenuItem       , { Y, _, _, Y, _, } },
            { mUi->exportDeviceListMenuItem, { Y, _, _, _, _, } },
            { mUi->customAccessMenuItem    , { _, Y, _, _, _, } }
        };
        for (int i=0; i<CARDINALITY(enableXMap); i++)
        {   bool enable = true;
            if (enableXMap[i].situation.listNotEmpty)    enable = enable && situation.listNotEmpty;
            if (enableXMap[i].situation.gatewayOnline)   enable = enable && situation.gatewayOnline;
            if (enableXMap[i].situation.gatewayOperable) enable = enable && situation.gatewayOperable;
            if (enableXMap[i].situation.selectedItems)   enable = enable && situation.selectedItems;
            if (enableXMap[i].situation.currentOnline)   enable = enable && situation.currentOnline;
            QWidget* widget = qobject_cast<QWidget*>(enableXMap[i].object);
            QAction* action = qobject_cast<QAction*>(enableXMap[i].object);
            if (widget != nullptr) widget->setEnabled(enable);
            if (action != nullptr) action->setEnabled(enable);
        }
    }
    // Sonstige Aktivitäten
    if (currentRow(mUi->deviceView) != INVALID && !situation.gatewayOnline)
    {   mainFrame().showStatusBarWarning(tr("Connection to gateway lost"));
    }
    emit selectedChanged();
    mUi->deviceView->setFocus();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onAdd(void)
{
    hideAllView(mUi);
    mUi->addDeviceView->show();
    mRefreshingInProgress = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onRemove(void)
{
    hideAllView(mUi);
    QList<QModelIndex> rowList = mUi->deviceView->selectionModel()->selectedRows();
    QList<Cy4Address> addressList;  // Adressen der zu löschenden Geräte auflisten
    for (int row=0; row<rowList.count(); row++)
    {   int index = toIndex(rowList[row].row());
        Cy4Address address = deviceList().at(index).address();
        if (address == cy4Net().gateway()) addressList.append(address); // Gateway ggf. hinten
        else                               addressList.prepend(address);
    }
    if (!addressList.isEmpty())
    {   QString text;
        if (addressList.last() == cy4Net().gateway())
        {   if (rowList.count() == 1) text = tr("Remove gateway? Connection will be lost");
            else                      text = tr("Remove gateway too? Connection will be lost");
        }
        else
        {   if (addressList.count() == 1)
            {   text = tr("Remove selected device from list?");
            }
            else
            {   text = tr("Remove %1 selected devices from list?").arg(rowList.count());
            }
        }
        if( QMessageBox::question(this, APPL_NAME, text, QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        {   mUi->deviceView->setFocus();
            return;
        }
    }
    for (int i=0; i<addressList.count(); i++)
    {   int index = deviceList().indexOf(addressList[i]);
        if (index != INVALID) deviceList().remove(index);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onDiscover(void)
{
    mUi->deviceView->clearSelection();
    hideAllView(mUi);
    mUi->discoverDevicesView->show();
    mRefreshingInProgress = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onUploadFirmware(void)
{
    hideAllView(mUi);
    mainFrame().showPage(Page::Firmware);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onRefresh(void)
{
    const Cy4Device* device = currentDevice();
    if (device == nullptr) return;
    int index = deviceList().indexOf(device->address());
    mainFrame().setEnabled(false);
    mUi->toolBarFrame->setEnabled(false);
    if (index != INVALID)
    {   QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        int index = deviceList().indexOf(device->address());
        mainFrame().showStatusBarInfo(tr("Refresh data ..."));
        deviceList().setBusyFlag(index);
        deviceList().clearNewFlag(index);
        deviceList().fetch(index);
        if (deviceList().isOnline(index))
        {   mainFrame().showStatusBarInfo(tr("Refresh data ... done"));
        }
        else if (cy4Net().gateway().isValid())
        {   mainFrame().showStatusBarWarning(tr("No response from device"));
        }
        else
        {   onChanged();
        }
        deviceList().clearBusyFlag(index);
        QGuiApplication::restoreOverrideCursor();
    }
    mainFrame().setEnabled(true);
    mUi->toolBarFrame->setEnabled(true);
    mUi->deviceView->setFocus();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onRefreshAll(void)
{
    hideAllView(mUi);
    mainFrame().setEnabled(false);
    mUi->toolBarFrame->setEnabled(false);
    int topVisibleRow = mUi->deviceView->rowAt(1);   // Position im Viewport merken
    mRefreshingInProgress = true;
    mainFrame().showStatusBarInfo(tr("Refresh all"));
    for (int row = 0; row < deviceList().size(); row++)
    {   int index = toIndex(row);
        QModelIndex modelIndex = deviceList().modelIndexOf(index);
        mUi->deviceView->scrollTo(modelIndex);
        deviceList().setBusyFlag(index);
        deviceList().fetch(index);
        if (!cy4Net().gateway().isValid())
        {   mRefreshingInProgress = false;
        }
        deviceList().clearBusyFlag(index);
        mainFrame().showStatusBarProgress(0, row, deviceList().size()-1);
        if (!mRefreshingInProgress)
        {   if (cy4Net().gateway().isValid())
            {   mainFrame().showStatusBarInfo(tr("Refresh stopped"));
            }
            else
            {   onChanged();
            }
            break;
        }
    }
    if (mRefreshingInProgress)
    {   mRefreshingInProgress = false;
        mainFrame().showStatusBarInfo(tr("Refresh done"));
    }
    // Ursprüngliche Position im Viewport wieder herstellen
    if (topVisibleRow != -1)
    {   QModelIndex modelIndex = deviceList().modelIndexOf(toIndex(topVisibleRow));
        mUi->deviceView->scrollTo(modelIndex, QAbstractItemView::PositionAtTop);
    }
    mainFrame().setEnabled(true);
    mUi->toolBarFrame->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onRestart(void)
{
    hideAllView(mUi);
    const Cy4Device* device = currentDevice();
    if (device == nullptr) return;
    int index = deviceList().indexOf(device->address());
    if (index != INVALID)
    {   QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        mUi->deviceView->clearSelection();
        deviceList().setBusyFlag(index);
        Cy4Address address = deviceList().at(index).address();
        mainFrame().showStatusBarInfo(tr("Restart device ..."));
        if (cy4Net().enterApplication(address))
        {   deviceList().fetch(index);
            const Cy4TypePlate& typePlate = deviceList().at(index).typePlate();
            if (typePlate.isProgramLoader())
            {   mainFrame().showStatusBarWarning(tr("No application found on device"));
            }
            else
            {   mainFrame().showStatusBarInfo(tr("Restart device ... done"));
            }
        }
        else if (cy4Net().gateway().isValid())
        {   mainFrame().showStatusBarWarning(tr("No response from device"));
        }
        else
        {   onChanged();
        }
        deviceList().clearBusyFlag(index);
        mUi->deviceView->selectRow(toRow(index));
        QGuiApplication::restoreOverrideCursor();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onEnterPgmLdr(void)
{
    hideAllView(mUi);
    const Cy4Device* device = currentDevice();
    if (device == nullptr) return;
    int index = deviceList().indexOf(device->address());
    if (index != INVALID)
    {   QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        mUi->deviceView->clearSelection();
        deviceList().setBusyFlag(index);
        Cy4Address address = deviceList().at(index).address();
        mainFrame().showStatusBarInfo(tr("Enter program loader ..."));
        if (cy4Net().enterProgramLoader(address))
        {   deviceList().fetch(index);
            const Cy4TypePlate& typePlate = deviceList().at(index).typePlate();
            if (typePlate.isProgramLoader())
            {   mainFrame().showStatusBarInfo(tr("Enter program loader ... done"));
            }
            else
            {   mainFrame().showStatusBarWarning(tr("No program loader found on device"));
            }
        }
        else if (cy4Net().gateway().isValid())
        {   mainFrame().showStatusBarWarning(tr("No response from device"));
        }
        else
        {   onChanged();
        }
        deviceList().clearBusyFlag(index);
        mUi->deviceView->selectRow(toRow(index));
        QGuiApplication::restoreOverrideCursor();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onChangeAddress(void)
{
    hideAllView(mUi);
    const Cy4Device* device = currentDevice();
    if (device == nullptr) return;
    int index = deviceList().indexOf(device->address());
    if (index != INVALID)
    {   mUi->changeAddressView->show(*device);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onMarkAsNew(void)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QList<QModelIndex> rowList = mUi->deviceView->selectionModel()->selectedRows();
    for (int row=0; row<rowList.count(); row++)
    {   int index = toIndex(rowList[row].row());
        deviceList().setNewFlag(index);
    }
    QGuiApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onImportExportDeviceList(void)
{
    hideAllView(mUi);
    QString directoryPath = settings().deviceListImportExportDirectoryPath;
    if (!QFileInfo::exists(directoryPath)) directoryPath = defaults().deviceListImportExportDirectoryPath;
    QString filter = QString("*.%1\n*.*").arg(QFileInfo(CFG::DeviceListFileName).completeSuffix());
    QObject* source = sender();
    if (source == mUi->importDeviceListMenuItem)
    {   QString title    = tr("Import list of Cy4NET devices");
        QString filePath = QFileDialog::getOpenFileName(this, title, directoryPath, filter);
        if (!filePath.isEmpty())
        {   if (QFileInfo::exists(filePath) && !deviceList().isEmpty())
            {   if (QMessageBox::question(this, APPL_NAME, tr("Replace existing device list?")) == QMessageBox::No)
                {   return;
                }
            }
            settings().deviceListImportExportDirectoryPath = QFileInfo(filePath).absolutePath();
            QString issue;
            if (deviceList().load(filePath, issue))
            {   mainFrame().showStatusBarInfo(tr("Device list importet"));
            }
            else
            {   mainFrame().showStatusBarWarning(issue);
            }
        }
    }
    else if (source == mUi->exportDeviceListMenuItem)
    {   QString title = tr("Export Cy4NET device list");
        QString baseName = QFileInfo(CFG::DeviceListFileName).baseName();
        QString date     = QDateTime::currentDateTime().toString("yyMMdd");
        QString suffix   = QFileInfo(CFG::DeviceListFileName).completeSuffix();
        QString fileNamePreset = QString("%1_%2.%3").arg(baseName, date, suffix);
        QString filePathPreset = QString("%1/%2").arg(directoryPath, fileNamePreset);
        QString filePath = QFileDialog::getSaveFileName(this, title, filePathPreset, filter);
        if (!filePath.isEmpty())
        {   settings().deviceListImportExportDirectoryPath = QFileInfo(filePath).absolutePath();
            QString issue;
            if (deviceList().saveAs(filePath, issue))
            {   mainFrame().showStatusBarInfo(tr("Device list exportet"));
            }
            else
            {   mainFrame().showStatusBarWarning(issue);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onCustomAccess(void)
{
    mainFrame().showPage(Page::CustomAccess);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onRowClicked(const QModelIndex&)
{
    mRefreshingInProgress = false;
    hideAllView(mUi);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onRowDoubleClicked(const QModelIndex&)
{
    const Cy4Device* device = currentDevice();
    if (device == nullptr) return;
    int index = deviceList().indexOf(device->address());
    if (index == INVALID) return;
    deviceList().clearNewFlag(index);
    deviceList().fetch(index);
    if (deviceList().isOnline(index))
    {   mainFrame().clearStatusBarMessage();
    }
    else if (cy4Net().gateway().isValid())
    {   mainFrame().showStatusBarWarning(tr("No response from device"));
    }
    if (deviceList().isOnline(index))
    {   if (device->typePlate().isProgramLoader()) return;
        mainFrame().showPage(Page::Dashboard);
    }
    onChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onContextMenu(QPoint)
{
    hideAllView(mUi);
    mContextMenu.exec(QCursor::pos());
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onDeviceInserted(int index)
{
    mUi->deviceView->clearSelection();
    QModelIndex modelIndex = deviceList().modelIndexOf(index);
    mUi->deviceView->setCurrentIndex(modelIndex);
    Cy4Address address = deviceList().at(index).address();
    if (address != cy4Net().gateway())
    {   mainFrame().showStatusBarInfo(tr("New device at %1").arg(address.toText().c_str()));
    }
    updateSeparatorLines();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onDeviceRemoved(Cy4Address address)
{
    if (address == cy4Net().gateway())
    {   mainFrame().showStatusBarWarning(tr("Gateway removed. Connection lost"));
    }
    else
    {   mainFrame().showStatusBarInfo(tr("Device at %1 removed").arg(address.toText().c_str()));
    }
    updateSeparatorLines();
    onChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_Escape)
    {   mRefreshingInProgress = false;
    }
    else if (key == Qt::Key_Return)
    {   QModelIndex modelIndex = mUi->deviceView->currentIndex();
        onRowDoubleClicked(modelIndex);
    }
    else if( key == Qt::Key_F5 )
    {   onRefreshAll();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::retranslateUi(void)
{
    mUi->retranslateUi(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
DeviceListPage::DeviceListPage(QWidget* parent) : AbstractPage(parent)
{
    // GUI
    {   mUi = new Ui::DeviceListPage;
        mUi->setupUi(this);
        mRowDelegate = new RowDelegate(this);
    }
    // (...)-Menü
    {   mMoreMenu.addAction(mUi->refreshMenuItem);
        mMoreMenu.addAction(mUi->restartMenuItem);
        mMoreMenu.addAction(mUi->enterPgmLdrMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->addMenuItem);
        mMoreMenu.addAction(mUi->removeMenuItem);
        mMoreMenu.addAction(mUi->changeAddressMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->markAsNewMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->uploadFirmwareMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->exportDeviceListMenuItem);
        mMoreMenu.addAction(mUi->importDeviceListMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->customAccessMenuItem);
        mUi->moreButton->setMenu(&mMoreMenu);
    }
    // Kontextmenü
    {   mContextMenu.addAction(mUi->refreshMenuItem);
        mContextMenu.addAction(mUi->restartMenuItem);
        mContextMenu.addAction(mUi->enterPgmLdrMenuItem);
        mContextMenu.addSeparator();
        mContextMenu.addAction(mUi->removeMenuItem);
        mContextMenu.addAction(mUi->changeAddressMenuItem);
        mContextMenu.addSeparator();
        mContextMenu.addAction(mUi->uploadFirmwareMenuItem);
        mContextMenu.addSeparator();
        mContextMenu.addAction(mUi->markAsNewMenuItem);
        mContextMenu.addSeparator();
        mContextMenu.addAction(mUi->customAccessMenuItem);
    }
    // Listenelement initialisieren, Breite der Spalten festlegen
    {   mUi->deviceView->setModel(&deviceList());
        adjustTableColumnWidth(mUi->deviceView);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->refreshAllButton            , &QPushButton::clicked                  , this, &DeviceListPage::onRefreshAll);
        connect(mUi->addButton                   , &QPushButton::clicked                  , this, &DeviceListPage::onAdd);
        connect(mUi->removeButton                , &QPushButton::clicked                  , this, &DeviceListPage::onRemove);
        connect(mUi->discoverButton              , &QPushButton::clicked                  , this, &DeviceListPage::onDiscover);
        connect(mUi->uploadFirmwareButton        , &QPushButton::clicked                  , this, &DeviceListPage::onUploadFirmware);
        connect(mUi->refreshMenuItem             , &QAction::triggered                    , this, &DeviceListPage::onRefresh);
        connect(mUi->restartMenuItem             , &QAction::triggered                    , this, &DeviceListPage::onRestart);
        connect(mUi->enterPgmLdrMenuItem         , &QAction::triggered                    , this, &DeviceListPage::onEnterPgmLdr);
        connect(mUi->changeAddressMenuItem       , &QAction::triggered                    , this, &DeviceListPage::onChangeAddress);
        connect(mUi->addMenuItem                 , &QAction::triggered                    , this, &DeviceListPage::onAdd);
        connect(mUi->removeMenuItem              , &QAction::triggered                    , this, &DeviceListPage::onRemove);
        connect(mUi->markAsNewMenuItem           , &QAction::triggered                    , this, &DeviceListPage::onMarkAsNew);
        connect(mUi->uploadFirmwareMenuItem      , &QAction::triggered                    , this, &DeviceListPage::onUploadFirmware);
        connect(mUi->importDeviceListMenuItem    , &QAction::triggered                    , this, &DeviceListPage::onImportExportDeviceList);
        connect(mUi->exportDeviceListMenuItem    , &QAction::triggered                    , this, &DeviceListPage::onImportExportDeviceList);
        connect(mUi->customAccessMenuItem        , &QAction::triggered                    , this, &DeviceListPage::onCustomAccess);
        connect(mUi->deviceView                  , &QTableView::customContextMenuRequested, this, &DeviceListPage::onContextMenu);
        connect(&deviceList()                    , &DeviceList::deviceInserted            , this, &DeviceListPage::onDeviceInserted);
        connect(&deviceList()                    , &DeviceList::deviceChanged             , this, &DeviceListPage::onChanged);
        connect(&deviceList()                    , &DeviceList::deviceRemoved             , this, &DeviceListPage::onDeviceRemoved);
        connect(mUi->deviceView                  , &QTableView::clicked                   , this, &DeviceListPage::onRowClicked);
        connect(mUi->deviceView                  , &QTableView::doubleClicked             , this, &DeviceListPage::onRowDoubleClicked);
        connect(mUi->deviceView->selectionModel(), &QItemSelectionModel::selectionChanged , this, &DeviceListPage::onChanged);
        connect(mUi->addDeviceView               , &AddDeviceView::hided                  , this, &DeviceListPage::onChanged);
        connect(mUi->changeAddressView           , &ChangeAddressView::hided              , this, &DeviceListPage::onChanged);
        connect(mUi->discoverDevicesView         , &DiscoverDevicesView::hided            , this, &DeviceListPage::onChanged);
    }
    // Sonstige Initialisierungen
    mUi->addDeviceView->QWidget::hide();
    mUi->changeAddressView->QWidget::hide();
    mUi->discoverDevicesView->QWidget::hide();
    mUi->deviceView->setCurrentIndex(QModelIndex());
}

//////////////////////////////////////////////////////////////////////////////////////////
DeviceListPage::~DeviceListPage()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
Page
DeviceListPage::page(void) const
{
    return Page::DeviceList;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4Device*
DeviceListPage::currentDevice() const
{
    int row = currentRow(mUi->deviceView);
    if (row == INVALID) return nullptr;
    return &deviceList().at(toIndex(row));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onShow(void)
{
    updateSeparatorLines();
    onChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceListPage::onHide(void)
{
    mRefreshingInProgress = false;
    hideAllView(mUi);
}

//////////////////////////////////////////////////////////////////////////////////////////
