//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Platzhalter (für unbekannte Service-Steuerelemente)           //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include "DummyServiceWidget.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
void
DummyServiceWidget::applyChanges(void)
{
    // Nichts zu tun
}

//////////////////////////////////////////////////////////////////////////////////////////
DummyServiceWidget::DummyServiceWidget(const ServiceProxy& serviceProxy,
                                       const QJsonObject&  jsonObject,
                                       QWidget*            parent)
                  : AbstractServiceWidget(serviceProxy, parent)
{
    mServiceProxy = serviceProxy;
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(1);
    {   QLabel* label = new QLabel();
        QPixmap pixmap(QString::fromUtf8(":/Symbols/Unknown.png"));
        pixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(pixmap);
        label->setAlignment(Qt::AlignCenter);
        verticalLayout->addWidget(label);
    }
    {   QLabel* label = new QLabel();
        QFont font = label->font();
        font.setFamily(QString::fromUtf8("Courier New"));
        label->setFont(font);
        label->setStyleSheet(QString::fromUtf8("color: rgba(25, 25, 25, 90);"));
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label->setText(jsonObject.value(KEY::TYPE).toString());
        verticalLayout->addWidget(label);
    }
    {   QLabel* label = new QLabel();
        QFont font = label->font();
        font.setItalic(true);
        label->setFont(font);
        label->setStyleSheet(QString::fromUtf8("color: rgba(25, 25, 25, 90);"));
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label->setText(QObject::tr("Unknown type"));
        verticalLayout->addWidget(label);
    }
    setLayout(verticalLayout);
}


//////////////////////////////////////////////////////////////////////////////////////////
QString
DummyServiceWidget::toClipboardText(void)
{
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DummyServiceWidget::populateWidget(void)
{
    // Nichts zu tun
}

//////////////////////////////////////////////////////////////////////////////////////////

