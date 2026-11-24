//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Platzhalter (für unbekannte Service-Steuerelemente)           //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLabel>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class DummyServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
              DummyServiceWidget(const ServiceProxy&,
                                 const QJsonObject&,
                                 QWidget* = nullptr);
    virtual  ~DummyServiceWidget() = default;    
    QString   toClipboardText(void) override;
	void      populateWidget(void) override;
};

//////////////////////////////////////////////////////////////////////////////////////////


