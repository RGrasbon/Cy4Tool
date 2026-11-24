//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Aufzählung                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QAbstractitemView>
#include <QStandardItem>
#include <QCollator>
#include <QMessageBox>
#include "Conf.h"
#include "Global.h"
#include "EnumerationServiceWidget.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
bool
EnumerationServiceWidget::eventFilter(QObject* source, QEvent* event)
{
    if (source == mComboBox)
    {   if (event->type() == QEvent::Wheel)
        {   if (mComboBox->view() != nullptr && !mComboBox->view()->isVisible())
            {   // Verhindern, dass allein durch überrollen mit der Maus bereits eine
                // Auswahl getroffen wird.
                if (mComboBox->parentWidget())
                {   QCoreApplication::sendEvent(mComboBox->parentWidget(), event);
                }
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {   notifyStatus();
        }
    }
    return QObject::eventFilter(source, event);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
EnumerationServiceWidget::applyChanges(void)
{
    if (!mConfirm.isEmpty())
    {   if (QMessageBox::question(this, APPL_NAME, mConfirm) != QMessageBox::Yes) return;
    }
    ByteArray widgetData = serialize(mComboBox->currentData().toDouble());
    mServiceProxy.write(0, widgetData);
    if (mServiceProxy.access() == Service::WriteOnly) mComboBox->setCurrentIndex(-1);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
EnumerationServiceWidget::EnumerationServiceWidget(const ServiceProxy& serviceProxy,
                                                   const QJsonObject&  jsonObject,
                                                   QWidget*            parent)
                        : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    QList<QStandardItem*> itemList;
    {   if (mServiceProxy.size() > 4)
        {   mIssue = tr("Service size must be 1, 2, 3 or 4");
            mIsOperational = false;
        }
        mValueDomain = toValueDomain(jsonObject);
        if (jsonObject.contains(KEY::ASSIGNMENTS))
        {   QJsonObject jsonAssignments = jsonObject.value(KEY::ASSIGNMENTS).toObject();
            QStringList keyList = jsonAssignments.keys();
            {   // "Natürliche Sortierung": "1", "2", "10", ...
                QCollator collator;
                collator.setNumericMode(true);
                std::sort(keyList.begin(), keyList.end(),
                            [&collator](const QString& a, const QString& b)
                            {   return collator.compare(a, b) < 0;
                            }
                         );
            }
            for(const QString& key : std::as_const(keyList))
            {   bool ok;
                SInt32 value;
                switch (mValueDomain)
                {   case ValueDomain::Signed:   value = key.toInt(&ok);          break;
                    case ValueDomain::Unsigned: value = SInt32(key.toUInt(&ok)); break;
                    default:                    ok = false;
                }
                QStandardItem* item;
                if (ok)
                {   item = new QStandardItem(Global::jsonLanguageEntry(jsonAssignments[key]));
                    item->setData(value, Qt::UserRole);
                }
                else
                {   item = new QStandardItem("???");
                    item->setData(INT_MIN, Qt::UserRole);
                }
                if (serviceProxy.access() == Service::ReadOnly)
                {   item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                }
                itemList.append(item);
            }
        }
        else
        {   mIssue = tr("Missing entry \"%1\"").arg(KEY::ASSIGNMENTS);
            mIsOperational = false;
        }
        mConfirm = Global::jsonLanguageEntry(jsonObject.value(KEY::CONFIRM));
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mComboBox = new QComboBox(this);
        mComboBox->setContextMenuPolicy(Qt::NoContextMenu);
        mComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mComboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(mComboBox, &QComboBox::activated, this, &EnumerationServiceWidget::applyChanges);
        verticalLayout->addWidget(mComboBox, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        mComboBox->setCurrentIndex(-1);
        mComboBox->installEventFilter(this);
        for(QStandardItem* item : itemList)
        {   qobject_cast<QStandardItemModel*>(mComboBox->model())->appendRow(item);
        }
        if (mServiceProxy.access() == Service::ReadOnly)
        {   mComboBox->setFocusPolicy(Qt::NoFocus);
            mComboBox->setStyleSheet(CFG::BackgroundForReadOnly);
        }
        mComboBox->setEnabled(mIsOperational);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
EnumerationServiceWidget::toClipboardText(void)
{
    bool ok;
    QString text = mComboBox->currentText();
    SInt32 value = mComboBox->currentData().toInt(&ok);
    if (ok)
    {   switch (mValueDomain)
        {   case ValueDomain::Signed:
                return QString("%1 (%2)").arg(text).arg(value);
            case ValueDomain::Unsigned:
                return QString("%1 (%2)").arg(text).arg(static_cast<UInt32>(value));
            default:
                ;
        }
    }
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
EnumerationServiceWidget::populateWidget(void)
{
    int newIndex = -1;
    if (mServiceProxy.isUpToDate())
    {   SInt32 data = deserialize(mServiceProxy.read(), mValueDomain);
        int index = mComboBox->findData(data);
        if (index >= 0 && index < mComboBox->count()) newIndex = index;
    }
    mComboBox->setCurrentIndex(newIndex);
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

