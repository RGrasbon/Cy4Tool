//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Diensteliste-Seite                                                                  //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QKeyEvent>
#include <QClipboard>
#include <QFileDialog>
#include "ui_ServiceListPage.h"
#include "Global.h"
#include "Settings.h"
#include "MainFrame.h"
#include "AbstractServiceWidget.h"
#include "ServiceListPage.h"
#include "DashboardPage.h"
#include "DeviceList.h"
#include "ServiceMap.h"
#include "Downloader.h"

//////////////////////////////////////////////////////////////////////////////////////////
static void
adjustTableColumnWidth(QTableWidget* tableWidget)
{
    QFontMetrics fontMetricsItem = tableWidget->fontMetrics();
    QFontMetrics fontMetricsHeader = tableWidget->horizontalHeader()->fontMetrics();
    QString headerSpace[][2] = { { tableWidget->horizontalHeaderItem(0)->text(), "000000h"},
                                 { tableWidget->horizontalHeaderItem(1)->text(), "0000"   },
                                 { tableWidget->horizontalHeaderItem(2)->text(), "0/0"    }
                               };
    int columnWidth[CARDINALITY(headerSpace)];
    for (int i=0; i<CARDINALITY(headerSpace); i++)
    {   int cellItemWidth  = max(fontMetricsItem.horizontalAdvance(QString("__%1__").arg(headerSpace[i][0])),
                                 fontMetricsItem.horizontalAdvance(QString("__%1__").arg(headerSpace[i][1])));
        QString titleLabel = tableWidget->model()->headerData(i, Qt::Horizontal).toString();
        int headerItemWidth = fontMetricsHeader.horizontalAdvance(titleLabel);
        columnWidth[i] = max(headerItemWidth, cellItemWidth);
    }
    for( int i=0; i<CARDINALITY(columnWidth); i++ )
    {   tableWidget->horizontalHeader()->resizeSection(i, columnWidth[i]);
    }
    tableWidget->horizontalHeader()->setSectionsClickable(false);
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
createToolTip(const Service& service)
{
    QString issueText;
    {   QString issue = service.issue();
        if (!issue.isEmpty())
        {   issueText = QString("<b><u>%1:</u></b><p>"
                                "<b>%2</b> %3<p><hr>").arg(QObject::tr("Invalid service"))
                                                      .arg(QChar(0x2192)).arg(issue);
        }
    }
    QString signatureText;
    {   signatureText = QString("%1h:%2:%3").arg(service.location().toText())
                                            .arg(service.size())
                                            .arg(Global::print(service.access()));
    }
    QString briefText;
    {   QStringList list = service.brief().split("\n");
        if (list.isEmpty())
        {   briefText = QObject::tr("n.a.");
        }
        if (list.count() == 1)
        {   briefText = list[0];
        }
        else
        {   briefText.append("<ul>");
            for (const QString& listText: static_cast<const QStringList>(list))
            {   briefText.append(QString("<li>%1</li>").arg(listText));
            }
            briefText.append("<ul>");
        }
    }
    QString text = QString("%1"
                           "<table>"
                           "<tr><td><b>%2: </b></td> <td><i>%3</i></td></tr>"
                           "<tr><td><b>%4: </b></td> <td><tt>%5</tt></td></tr>"
                           "<tr><td><b>%6: </b></td> <td>%7</td></tr>"
                           "</table>")
                           .arg(issueText,
                                QObject::tr("Service")  , service.name(),
                                QObject::tr("Signature"), signatureText,
                                QObject::tr("Brief")    , briefText);
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::populateList(void)
{
    static const QBrush normalRowColor    = QColor(245, 245, 245);
    static const QBrush mandatoryRowColor = QColor(235, 235, 240);
    static const QBrush invalidRowColor   = QColor(255, 150,  25);

    QTableWidget* tableWidget = mUi->serviceList;
    tableWidget->clearContents();
    tableWidget->setRowCount(serviceMap().size());
    for (int i=0; i<serviceMap().size(); i++)
    {   const Service& service = serviceMap().at(i);
        ServiceMap::ItemTag tag = serviceMap().tag(i);
        QBrush rowBackgroundColor;
        if (tag == ServiceMap::Valid)
        {   bool isMandatory = service.location() >= 0xFFFFF0;
            if(isMandatory) rowBackgroundColor = mandatoryRowColor;
            else            rowBackgroundColor = normalRowColor;
        }
        else
        {   rowBackgroundColor = invalidRowColor;
        }
        // Text bei "Maus über ..."
        QString toolTipText = createToolTip(service);
        // Bestimmungsort
        {   QTableWidgetItem* item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            Cy4Location location = service.location();
            QString text = QString("%1h").arg(location.toText().c_str());
            item->setText(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(rowBackgroundColor);
            item->setToolTip(createToolTip(service));
            tableWidget->setItem(i, 0, item);
        }
        // Größe
        {   QTableWidgetItem* item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            int size = service.size();
            if (size == INVALID) item->setText("---");
            else                 item->setText(QString::number(size));
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(rowBackgroundColor);
            item->setToolTip(toolTipText);
            tableWidget->setItem(i, 1, item);
        }
        // Zugriff
        {   QTableWidgetItem* item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setText(Global::print(service.access()));
            item->setTextAlignment(Qt::AlignCenter);
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setBackground(rowBackgroundColor);
            item->setToolTip(toolTipText);
            tableWidget->setItem(i, 2, item);
        }
        // Beschreibung
        {   QTableWidgetItem* item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            QString nameText = service.name();
            QString statusText;
            {   switch (tag)
                {   case ServiceMap::Unknown:    statusText = tr("Unknown failure");                                                        break;
                    case ServiceMap::Invalid:    statusText = tr("Service is invalid");                                                     break;
                    case ServiceMap::PgmLdrArea: statusText = tr("Service must not be located in program loader area: 000000h .. 0FFFFFh"); break;
                    case ServiceMap::Overlap:    statusText = tr("Overlaying services with inkompatible access types are not allowed");     break;
                    case ServiceMap::Adjacent:   statusText = tr("Adjacent services with different access types are not allowed");          break;
                    case ServiceMap::Mandatory:  statusText = tr("Mandatory services can be overwritten but not merged");                   break;
                    default:                     ;
                }
            }
            if (statusText.isEmpty()) item->setText(nameText);
            else                      item->setText(QString("%1 (%2)").arg(nameText, statusText));
            QFont font = item->font();
            font.setItalic(true);
            item->setFont(font);
            item->setBackground(rowBackgroundColor);
            item->setToolTip(toolTipText);
            tableWidget->setItem(i, 3, item);
        }
    }
    // Mehrfach vorkommende Dienstnamen mit Nummern erweitern
    {   QMap<QString, int> map;
        // (1) In <map> verzeichnen, wie häufig jeder Dienstname vorkommt
        for (int i=0; i<serviceMap().size()-1; i++)
        {   if (serviceMap().tag(i) != ServiceMap::Valid) continue;
            QString name = serviceMap().at(i).name();
            if (map.contains(name)) map[name] = map[name]+1;  // Vorkommen zählen
            else                    map[name] = 1;            // Zu Anfang, Vorkommen: 1
        }
        // (2) Alle Dienstname, die nur einmal vorkommen wieder aus <map> entfernen
        QMap<QString, int>::iterator it = map.begin();
        while (it != map.end())
        {   if (it.value() < 2) it = map.erase(it);
            else                it++;
        }
        // (3) Die zu den Dienstname passenden Tabelleneinträge mit Zahlen versehen
        for (QMap<QString, int>::const_iterator it = map.cbegin(); it != map.cend(); it++)
        {   QString name = it.key();
            int number = 1;
            for (int i=0; i<serviceMap().size(); i++)
            {   if (serviceMap().at(i).name() == name)
                {   QString itemText = QString("%1 (#%2)").arg(name).arg(number);
                    tableWidget->item(i, 3)->setText(itemText);
                    number++;
                }
            }
        }
    }
    if (!serviceMap().isValid())
    {   mainFrame().showStatusBarWarning(tr("Invalid service map"));
    }
    mUi->serviceDetailsView->hide();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onOpenInfoView(void)
{
    int row = mUi->serviceList->currentRow();
    if (row >= 0)
    {   ServiceMap::ItemTag state = serviceMap().tag(row);
        if (state != ServiceMap::Adjacent && state != ServiceMap::Overlap)
        {   mUi->serviceList->setCurrentCell(row, 0);
            mUi->serviceDetailsView->show();
        }
    }
    else
    {   mUi->serviceDetailsView->hide();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onReload(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) mCurrentDevice = Cy4Device();
    else                   mCurrentDevice = *device;
    QString issue;
    if (serviceMap().load(mCurrentDevice, issue))
    {   mainFrame().showStatusBarInfo(tr("Service map reloaded"));
    }
    else
    {   mainFrame().showStatusBarWarning(issue);
    }
    populateList();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onOpenInEditor(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    Cy4ApplicationType type = device->typePlate().applicationType();
    QString filePath = Global::mapFilePathFor(type);
    QString issue;
    if (!Global::openFileInEditor(filePath, &issue))
    {   mainFrame().showStatusBarWarning(issue);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onCustomAccess(void)
{
    mainFrame().showPage(Page::CustomAccess);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onSearchTextChanged(void)
{
    QString text = mUi->searchText->text();
    mUi->clearSearchTextButton->setEnabled(!text.isEmpty());
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onFindNext(void)
{
    QString searchText = mUi->searchText->text().trimmed();
    if (searchText.trimmed().isEmpty()) return;
    int currentRow = max(0, mUi->serviceList->currentIndex().row());
   // mSearchPosition
    int row = currentRow;
    do
    {   row++;
        if (row == mUi->serviceList->rowCount()) row = 0;
        QString lineText;
        for (int column=0; column<mUi->serviceList->columnCount(); column++)
        {   QString cellText = mUi->serviceList->item(row, column)->text();
            if (cellText.indexOf(searchText, 0, Qt::CaseInsensitive) != -1)
            {   QTableWidgetItem *item = mUi->serviceList->item(row, column);
                QModelIndex index = mUi->serviceList->indexFromItem(item);
                mUi->serviceList->setCurrentIndex(index);
                mUi->serviceList->scrollTo(index);
                if (row == currentRow) mainFrame().showStatusBarInfo(tr("Nothing else found"));
                else                   mainFrame().clearStatusBarMessage();
                return;
            }
        }
    } while (row != currentRow);
    mainFrame().showStatusBarInfo(tr("\"%1\" not found").arg(searchText));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onDownload(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    Cy4ApplicationType type = device->typePlate().applicationType();
    QString tempFilePath = QString("%1/tmp_%2").arg(QDir::tempPath(), type.toText());
    QString url = Global::mapFileUrlFor(type);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString issue;
    bool success = downloader().get(url, tempFilePath, issue);
    if (!success)
    {   mainFrame().showStatusBarInfo(QObject::tr("Download failed: %1").arg(issue));
        QGuiApplication::restoreOverrideCursor();
        return;
    }
    if (serviceMap().import(*device, tempFilePath, issue))
    {   mainFrame().showStatusBarInfo(tr("Service map downloaded"));
    }
    else
    {   mainFrame().showStatusBarWarning(issue);
    }
    Global::removeFile(tempFilePath);
    onReload();
    QGuiApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onImport(void)
{
    QString preset = settings().serviceMapsImportExportFilePath;
    if (!Global::pathExists(preset)) preset = settings().localCy4ToolDataDirectoryPath;
    QString filter   = "*.json\n*.*";
    QString title    = tr("Import service map");
    QString filePath = QFileDialog::getOpenFileName(NULL, title, preset, filter);
    if (filePath.isEmpty()) return;
    settings().serviceMapsImportExportFilePath = filePath;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    QString issue;
    if (serviceMap().import(*device, filePath, issue))
    {   mainFrame().showStatusBarInfo(tr("Service map imported"));
    }
    else
    {   mainFrame().showStatusBarWarning(issue);
    }
    mCurrentDevice = serviceMap().device();
    populateList();
    QGuiApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onCopyToClipboard(void)
{
    Cy4TypePlate       typePlate = serviceMap().device().typePlate();
    Cy4ApplicationType type      = typePlate.applicationType();
    Cy4Version         version   = typePlate.applicationVersion();
    QString            label;
    int index = deviceList().indexOf(serviceMap().device().address());
    if (index != INVALID) label = deviceList().label(index);
    QString text = QString("Service map\n"
                           "\n"
                           "application: %1, %2\n"
                           "label:       \"%3\"\n"
                           "\n"
                           " Location | Size | Access | Name\n"
                           "----------+------+--------+----------------------------------"
                           "--------\n")
                           .arg(type.toText().c_str())
                           .arg(version.toText().c_str())
                           .arg(label);
    for (int i=0; i<serviceMap().size(); i++)
    {   const Service& service = serviceMap().at(i);
        Cy4Location location = service.location();
        if (location == 0xFFFFF0)
        {   text.append("----------+------+--------+-----------------------------------"
                        "------\n");
        }
        QString locationText = QString("%1h").arg(location.toText().c_str());
        QString sizeText     = QString("%1").arg(service.size(), 4);
        QString accessText;
        switch (service.access())
        {   case Service::ReadOnly:  accessText = "R "; break;
            case Service::WriteOnly: accessText = "W "; break;
            case Service::ReadWrite: accessText = "RW"; break;
            default:                 accessText = "? ";
        }
        text.append(QString("  %1 | %2 |   %3   | %4\n").arg(locationText,
                                                             sizeText,
                                                             accessText,
                                                             service.name()));
    }
    text.append("----------+------+--------+-----------------------------------------\n");
    QApplication::clipboard()->setText(text);
    mainFrame().showStatusBarInfo(tr("Service map copied to clipboard"));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onContextMenu(QPoint)
{
    mContextMenu.exec(QCursor::pos());
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_F3 )
    {   onFindNext();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::retranslateUi(void)
{
    mUi->retranslateUi(this);
    mUi->serviceDetailsView->hide();
    QString dummy;
    serviceMap().load(mCurrentDevice, dummy);
    populateList();
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceListPage::ServiceListPage(QWidget* parent) : AbstractPage(parent)
{
    // GUI
    {   mUi = new Ui::ServiceListPage;
        mUi->setupUi(this);
    }
    // (...)-Menü
    {   mMoreMenu.addAction(mUi->reloadMenuItem);
        mMoreMenu.addAction(mUi->downloadMeniItem);
        mMoreMenu.addAction(mUi->importMenuItem);
        mMoreMenu.addAction(mUi->openInEditorMenuItem);
        mMoreMenu.addAction(mUi->copyToClipboardMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->customAccessMenuItem);
        mUi->moreButton->setMenu(&mMoreMenu);
    }
    // Kontextmenü
    {   mContextMenu.addAction(mUi->customAccessMenuItem);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->reloadButton           , &QPushButton::clicked               , this                   , &ServiceListPage::onReload);
        connect(mUi->openInEditorButton     , &QPushButton::clicked               , this                   , &ServiceListPage::onOpenInEditor);
        connect(mUi->clearSearchTextButton  , &QPushButton::clicked               , mUi->searchText        , &QLineEdit::clear);
        connect(mUi->searchText             , &QLineEdit::textChanged             , this                   , &ServiceListPage::onSearchTextChanged);
        connect(mUi->searchText             , &QLineEdit::editingFinished         , this                   , &ServiceListPage::onFindNext);
        connect(mUi->findNextButton         , &QPushButton::clicked               , this                   , &ServiceListPage::onFindNext);
        connect(mUi->reloadMenuItem         , &QAction::triggered                 , this                   , &ServiceListPage::onReload);
        connect(mUi->downloadMeniItem       , &QAction::triggered                 , this                   , &ServiceListPage::onDownload);
        connect(mUi->importMenuItem         , &QAction::triggered                 , this                   , &ServiceListPage::onImport);
        connect(mUi->openInEditorMenuItem   , &QAction::triggered                 , this                   , &ServiceListPage::onOpenInEditor);
        connect(mUi->copyToClipboardMenuItem, &QAction::triggered                 , this                   , &ServiceListPage::onCopyToClipboard);
        connect(mUi->customAccessMenuItem   , &QAction::triggered                 , this                   , &ServiceListPage::onCustomAccess);
        connect(mUi->serviceList            , &QTableWidget::clicked              , this                   , &ServiceListPage::onOpenInfoView);
        connect(mUi->serviceList            , &QTableWidget::doubleClicked        , mUi->serviceDetailsView, &ServiceDetailsView::hide);
        connect(mUi->serviceList            , &QWidget::customContextMenuRequested, this                   , &ServiceListPage::onContextMenu);
    }
    // Sonstige Initialisierungen
    adjustTableColumnWidth(mUi->serviceList);
    onSearchTextChanged();
    mUi->serviceList->setContextMenuPolicy(Qt::CustomContextMenu);
    mUi->serviceDetailsView->hide();
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceListPage::~ServiceListPage()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
Page
ServiceListPage::page(void) const
{
    return Page::ServiceList;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Service*
ServiceListPage::currentService(void) const
{
    int row = mUi->serviceList->currentRow();
    if (row < 0) return nullptr;
    return &serviceMap().at(row);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onShow(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    // Kopfzeile bestücken und Dienstekarte laden
    if (mCurrentDevice != *device)
    {   const Cy4TypePlate& typePlate = device->typePlate();
        mUi->titleApplicationTypeText->setText(typePlate.applicationType().toText().c_str());
        mUi->titleApplicationVersionText->setText(typePlate.applicationVersion().toText().c_str());
        int index = deviceList().indexOf(device->address());
        if (index == INVALID)
        {   mUi->titleLabelText->clear();
        }
        else
        {   mUi->titleLabelText->setText(deviceList().label(index));
            deviceList().clearNewFlag(index);
        }
        onReload();
        if (mCurrentDevice.isValid())
        {   QString text = mCurrentDevice.typePlate().applicationType().toText().c_str();
            mainFrame().showStatusBarInfo(tr("Service map loaded for %1").arg(text));
        }
    }
    // Dienst vorauswählen
    int row = mUi->serviceList->currentRow(); // Aktuell ausgewählten Zeile merken
    if (mainFrame().previousPage()->page() == Page::Dashboard)
    {   // Wenn diese Seite von der Bedienfeld-Seite aus geöffnet wurde, die zum
        // aktuellen Dienst-Steuerelement gehörenden Zeile auswählen
        DashboardPage* dashboardPage = qobject_cast<DashboardPage*>(mainFrame().previousPage());
        const AbstractServiceWidget* widget = dashboardPage->currentServiceWidget();
        if (widget != nullptr)
        {   const ServiceProxy& serviceProxy = widget->service();
            row = serviceMap().indexOf(serviceProxy.location(),
                                       serviceProxy.size(),
                                       serviceProxy.access());
        }
    }
    if (row >= 0)
    {   mUi->serviceList->setCurrentCell(row, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceListPage::onHide(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
