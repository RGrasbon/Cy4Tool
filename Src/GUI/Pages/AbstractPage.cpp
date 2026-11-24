//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Abstrakte Basisklasse Seite                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QEvent>
#include "AbstractPage.h"

//////////////////////////////////////////////////////////////////////////////////////////
void
AbstractPage::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
AbstractPage::AbstractPage(QWidget* parent) : QWidget(parent)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
