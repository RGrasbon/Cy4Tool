//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Tabelle                                                       //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QDir>
#include <QScrollBar>
#include "Conf.h"
#include "TableServiceWidget.h"
#include "NumberValidator.h"
#include "MainFrame.h"
#include "Global.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
QWidget*
TableServiceItemDelegate::createEditor(QWidget*                    parent,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex&          index) const
{
    assert(parent != nullptr);
    QWidget* tableWidget = parent->parentWidget();
    TableServiceWidget* tableServiceWidget = nullptr;
    if (tableWidget != nullptr) tableServiceWidget = qobject_cast<TableServiceWidget*>(tableWidget->parentWidget());
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit != nullptr && tableServiceWidget != nullptr)
    {   double min = tableServiceWidget->mRange.min;
        double max = tableServiceWidget->mRange.max;
        NumberValidator* validator = new NumberValidator(min,
                                                         max,
                                                         int(tableServiceWidget->mPrecision),
                                                         lineEdit);
        validator->setNotation(QDoubleValidator::StandardNotation);
        lineEdit->setValidator(validator);
    }
    return editor;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TableServiceItemDelegate::paint(QPainter*             painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex&          index) const
{
    QStyleOptionViewItem tmpOption = option;
    tmpOption.state &= ~QStyle::State_MouseOver;
    QStyledItemDelegate::paint(painter, tmpOption, index);
};

//////////////////////////////////////////////////////////////////////////////////////////
int
TableServiceWidget::toWidth(const QJsonObject& jsonObject)
{
    if (!jsonObject.contains(KEY::WIDTH)) return 1;
    int width = jsonObject.value(KEY::WIDTH).toInt();
    if (width < 1 || width > 4)
    {   mIssue = tr("Value for property \"%1\" must be 1, 2, 3 or 4").arg(KEY::WIDTH);
        mIsOperational = false;
        return 1;
    }
    return width;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
TableServiceWidget::toColumnCount(const QJsonObject& jsonObject)
{
    if (!jsonObject.contains(KEY::COLUMNS))
    {   mIssue = tr("Property \"%1\" missing").arg(KEY::COLUMNS);
        mIsOperational = false;
        return 1;
    }
    int columns = jsonObject.value(KEY::COLUMNS).toInt();
    if (columns < 1 || columns > 1000)
    {   mIssue = tr("Value of property \"%1\" must be in the range of 1 .. 1000")
                    .arg(KEY::COLUMNS);
        mIsOperational = false;
        return 1;
    }
    return columns;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TableServiceWidget::setItemValue(QTableWidgetItem* item, double value)
{
    QString text;
    if (mSpecialValueMap.contains(value))
    {   text = mSpecialValueMap[value];
    }
    else
    {   if (std::isnan(value) || value < mRange.min || value > mRange.max)
        {   text = "---";
        }
        else
        {   text = QString::number(value, 'f', int(mPrecision));
        }
    }
    setItemText(item, text);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TableServiceWidget::setItemText(QTableWidgetItem* item, const QString& text)
{
    bool blocked = mTableWidget->signalsBlocked();
    mTableWidget->blockSignals(true);
    item->setText(text);
    mTableWidget->blockSignals(blocked);
}

//////////////////////////////////////////////////////////////////////////////////////////
double
TableServiceWidget::fromItemText(QTableWidgetItem* item)
{
    QString text = item->text();
    QMap<double, QString>::Iterator it = mSpecialValueMap.begin();
    while (it != mSpecialValueMap.end() && text != it.value()) it++;
    bool ok = true;
    double value;
    if (it == mSpecialValueMap.end()) value = item->text().toDouble(&ok);
    else                              value = it.key();
    if (!ok) value = std::numeric_limits<double>::quiet_NaN();
    return value;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TableServiceWidget::onItemChanged(QTableWidgetItem* item)
{
    assert(item != nullptr);
    if (item->text().isEmpty()) return;
    // Zahlenwert holen, entweder aus <item> oder <mSpecialValueMap>
    double value = fromItemText(item);
    if (!std::isnan(value))
    {   // <value> in ByteArray umwandeln und in Dienstdaten integrieren
        value = bound(mRange.min, value, mRange.max);
        ByteArray data = serialize(value, mFormat, mWidth);
        int row    = item->row();
        int column = item->column();
        if (row == -1 || column == -1) return;
        int index = (row*mTableWidget->columnCount() + column)*mWidth;
        mServiceProxy.write(index, data);
        emit dataChanged();
    }
    setItemValue(item, value);
    applyChanges();
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
TableServiceWidget::eventFilter(QObject* source, QEvent* event)
{
    if (source == mTableWidget->viewport())
    {   if (event->type() == QEvent::Wheel)
        {   // Vermeiden, dass bei überrollen mit der Maus die Tabelle verschoben wird.
            event->ignore();
            return true;
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {   if (mServiceProxy.access() == Service::ReadWrite)
            {   QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                QTableWidgetItem* item = mTableWidget->itemAt(mouseEvent->pos());
                if (item == nullptr || !(item->flags() & Qt::ItemIsEditable))
                {   mainFrame().showStatusBarInfo(tr("Item not accassable"));
                }
            }
            else
            {   notifyStatus();
            }
        }
    }
    return QObject::eventFilter(source, event);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TableServiceWidget::applyChanges(void)
{
    // Das eigentliche Schreiben wurde in <onItemChanged> durchgeführt
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
TableServiceItemDelegate::TableServiceItemDelegate(QWidget* parent)
                        : QStyledItemDelegate(parent)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
TableServiceWidget::TableServiceWidget(const ServiceProxy& serviceProxy,
                                       const QJsonObject&  jsonObject,
                                       QWidget*            parent)
                  : AbstractServiceWidget(serviceProxy, parent)
{
    int columnCount;
    // Dienst
    {   // Hinweismeldung: "enabled-if" wird nicht mehr unterstützt
        if (serviceProxy.access() == Service::WriteOnly)
        {   mIssue = tr("Write only access not supported for widget \"%1\"").arg(KEY::TABLE);
            mIsOperational = false;
        }
        else if (jsonObject.contains(DEPRECATED::SEPARATOR))
        {   mIssue = tr("Property \"%1\" is no longer supported for \"%2\"")
                        .arg(DEPRECATED::SEPARATOR, KEY::TABLE);
        }
        mWidth           = toWidth(jsonObject);
        mFormat          = toFormat(jsonObject);
        mValueDomain     = toValueDomain(jsonObject);
        mPrecision       = toPrecision(jsonObject, mFormat);
        mRange           = toRange(jsonObject, mFormat, mValueDomain, mWidth);
        mSpecialValueMap = toSpecialValues(jsonObject);
        columnCount      = toColumnCount(jsonObject);
        if (mServiceProxy.size() % mWidth != 0)
        {   mIssue = tr("Service size must be a multiple of %1 (property \"%2\")")
                        .arg(mWidth).arg(KEY::WIDTH);
            mIsOperational = false;
        }
        if (mFormat == NumberFormat::Float && mWidth != 4)
        {   mIssue = tr("Number format \"%1\" requires \"%2\" of 4")
                        .arg(VALUE::FLOAT, KEY::WIDTH);
            mIsOperational = false;
        }
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mTableWidget = new QTableWidget(this);
        mTableWidget->setContextMenuPolicy(Qt::NoContextMenu);
        mTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        mTableWidget->setFocusPolicy(Qt::StrongFocus);
        mTableWidget->horizontalHeader()->hide();
        mTableWidget->verticalHeader()->hide();
        mTableWidget->setColumnCount(columnCount);
        connect(mTableWidget, &QTableWidget::itemChanged, this, &TableServiceWidget::onItemChanged);
        int rowCount = (mServiceProxy.size() + columnCount - 1) / columnCount;
        mTableWidget->setRowCount(rowCount);
        mTableWidget->viewport()->installEventFilter(this);
        TableServiceItemDelegate* delegate = new TableServiceItemDelegate(mTableWidget);
        mTableWidget->setItemDelegate(delegate);
        verticalLayout->addWidget(mTableWidget);
        setLayout(verticalLayout);
        // Zellen anlegen
        for (int row = 0; row < mTableWidget->rowCount(); row++)
        {   for (int column = 0; column < mTableWidget->columnCount(); column++)
            {   QTableWidgetItem *item = new QTableWidgetItem();
                item->setTextAlignment(Qt::AlignCenter);
                int index = (row*mTableWidget->columnCount() + column)*mWidth;
                if (mServiceProxy.access() == Service::ReadOnly || index >= mServiceProxy.size())
                {   item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    item->setBackground(QColor(245, 245, 245));
                }
                mTableWidget->setItem(row, column, item);
            }
        }
        if (mServiceProxy.access() == Service::ReadOnly)
        {   mTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
            mTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
            mTableWidget->setFocusPolicy(Qt::NoFocus);
            mTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
            mTableWidget->viewport()->setStyleSheet(CFG::BackgroundForReadOnly);
        }
        // Zellenbreite bestimmen
        QLineEdit lineEdit;
        QString measurementTextMin = Global::createMeasurementTextFor(mRange.min, int(mPrecision));
        QString measurementTextMax = Global::createMeasurementTextFor(mRange.max, int(mPrecision));
        int widthMin = Global::calculateWidthFor(measurementTextMin, &lineEdit);
        int widthMax = Global::calculateWidthFor(measurementTextMax, &lineEdit);
        int columnWidth = qMax(widthMin, widthMax); // Platzbedarf
        for (const QString& word : std::as_const(mSpecialValueMap))
        {   int wordWidth = Global::calculateWidthFor(word, &lineEdit);
            columnWidth = qMax(wordWidth, columnWidth);
        }
        columnWidth = qMax(Global::calculateWidthFor("---", &lineEdit), columnWidth);
        for (int column = 0; column < mTableWidget->columnCount(); column++)
        {   mTableWidget->horizontalHeader()->resizeSection(column, columnWidth);
        }
        mTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        // Tabellenausmaße festlegen
        int scrollBarSize   = mTableWidget->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        int rowHeight = mTableWidget->rowHeight(0) + 2;
        if (rowCount < 10)
        {   int tableHeight = rowCount * rowHeight + scrollBarSize;
            mTableWidget->setMaximumHeight(tableHeight);
        }
        else
        {   int tableHeight = 10 * rowHeight + scrollBarSize;
            mTableWidget->setMinimumHeight(tableHeight);
        }
        int tableWidth = columnWidth * mTableWidget->columnCount() +  // Zellenbreite
                         mTableWidget->frameWidth() * 10 +            // + Rahmen
                         scrollBarSize;                               // + Rollbalken
        mTableWidget->setMinimumWidth(150);
        mTableWidget->setMaximumWidth(tableWidth);
        mTableWidget->setEnabled(mIsOperational);
    }
    setMaximumSize(mTableWidget->maximumSize());
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
TableServiceWidget::toClipboardText(void)
{
    QStringList csvLineList;
    for (int row = 0; row < mTableWidget->rowCount(); row++)
    {   QStringList itemTextList;
        for (int column = 0; column < mTableWidget->columnCount(); column++)
        {   QTableWidgetItem* item = mTableWidget->item(row, column);
            itemTextList.append(item->text());
        }
        QString csvLine = itemTextList.join(";");
        csvLineList.append(csvLine);
    }
    #ifdef _WIN32
    constexpr char NEWLINE[] = "\r\n";
    #else
    constexpr char NEWLINE[] = "\n";
    #endif
    return csvLineList.join(NEWLINE);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TableServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        ByteArray tmpData(mWidth);
        for (int row = 0; row < mTableWidget->rowCount(); row++)
        {   for (int column = 0; column < mTableWidget->columnCount(); column++)
            {   QTableWidgetItem* item = mTableWidget->item(row, column);
                int index = (row*mTableWidget->columnCount() + column)*mWidth;
                if (index < int(data.size())-mWidth+1)
                {   for (int i=0; i<mWidth; i++) tmpData[i] = data[index+i];
                    double value = deserialize(tmpData, mValueDomain, mFormat);
                    setItemValue(item, value);
                }
                else
                {   setItemText(item, "");
                }
            }
        }
    }
    else
    {   for (int row = 0; row < mTableWidget->rowCount(); row++)
        {   for (int column = 0; column < mTableWidget->columnCount(); column++)
            {   QTableWidgetItem* item = mTableWidget->item(row, column);
                item->setText("");
            }
        }
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

