//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Dienst-Detailansicht                                                                //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QClipboard>
#include "ui_ServiceDetailsView.h"
#include "Global.h"
#include "MainFrame.h"
#include "ServiceDetailsView.h"

//////////////////////////////////////////////////////////////////////////////////////////
static QString
createSignatureText(const Service* service)
{    
    QString text = QString("%1h:%2:%3").arg(service->location().toText())
                                       .arg(service->size())
                                       .arg(Global::print(service->access()));
    if (service->location() < 0x100000)
    {   text += QString(" (%1)").arg(QObject::tr("Progam loader area"));
    }
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
createBriefText(const Service* service)
{
    QString text;
    const QStringList list = service->brief().split('\n', Qt::SkipEmptyParts);
    if (list.isEmpty())
    {   text = QObject::tr("n.a.");
    }
    else if (list.count() == 1)
    {   text = list.first().toHtmlEscaped();
        text.replace("&lt;br&gt;", "<br>", Qt::CaseInsensitive);
    }
    else
    {   QStringList itemTextList;
        for (const QString& item : list)
        {   if (item.trimmed().isEmpty()) continue;
            QString itemText = item.toHtmlEscaped();
            itemText.replace("&lt;br&gt;", "<br>", Qt::CaseInsensitive);
            itemTextList.append(QString("&bull; %1").arg(itemText));
        }
        text = itemTextList.join("<br>");
    }
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceDetailsView::onCopyToClipboard(void)
{    
    const Service* service = Global::currentService();
    if (service == nullptr) return;
    QString briefTitle  = tr("Brief");
    QString briefIndent = QString(briefTitle.length(), ' ');
    QString briefText;
    {   QStringList list = service->brief().split("\n");
        if (list.isEmpty())
        {   briefText = QObject::tr("n.a.");
        }
        if (list.count() == 1)
        {   briefText = list[0];
        }
        else
        {   briefText.append(QString("· %2").arg(list[0]));
            for (int i=1; i<list.length(); i++)
            {   briefText.append("\n");
                briefText.append(QString("%1  · %2").arg(briefIndent, list[i]));
            }
        }
        if (list.count() < 2) briefText.replace("<br>", "\n  "+briefIndent);
        else                  briefText.replace("<br>", "\n    "+briefIndent); // + "· "
    }
    QString text = QString("%1: %2\n"
                           "%3: %4\n"
                           "%5: %6\n")
                           .arg(tr("Service"), service->name(),
                                tr("Signature"), createSignatureText(service),
                                briefTitle, briefText);
    QApplication::clipboard()->setText(text);
    mainFrame().showStatusBarInfo(tr("Service details copied to clipboard."));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceDetailsView::onCustomAccess(void)
{
    mainFrame().showPage(Page::CustomAccess);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceDetailsView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) mUi->retranslateUi(this);
    QWidget::changeEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceDetailsView::ServiceDetailsView(QWidget* parent) : QWidget(parent)
{
    mUi = new Ui::ServiceDetailsView;
    mUi->setupUi(this);
    connect(mUi->customAccessButton   , &QPushButton::clicked, this, &ServiceDetailsView::onCustomAccess);
    connect(mUi->copyToClipboardButton, &QPushButton::clicked, this, &ServiceDetailsView::onCopyToClipboard);
    connect(mUi->closeButton          , &QPushButton::clicked, this, &ServiceDetailsView::hide);
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceDetailsView::~ServiceDetailsView()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceDetailsView::show(void)
{    
    const Service* service = Global::currentService();
    if (service == nullptr) return;
    QString issueText;
    {   QString issue = service->issue();
        if (!issue.isEmpty())
        {   issueText = QString("<b><u>%1:</u> %2").arg(tr("Invalid service"), issue);
            mUi->issueText->setText(issueText);
        }
        mUi->issueText->setVisible(!issue.isEmpty());
    }
    mUi->nameText->setText(service->name());
    mUi->signatureText->setText(createSignatureText(service));
    mUi->briefText->setText(createBriefText(service));
    if (!isVisible()) QWidget::show();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceDetailsView::hide(void)
{
    QWidget::hide();
}

//////////////////////////////////////////////////////////////////////////////////////////
