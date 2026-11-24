//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Zahlenvalidator                                                       				//
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QValidator>

//////////////////////////////////////////////////////////////////////////////////////////
class NumberValidator : public QDoubleValidator
{
    Q_OBJECT

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                            NumberValidator(double, double, int, QObject* = nullptr);
        QValidator::State   validate(QString&, int&) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
