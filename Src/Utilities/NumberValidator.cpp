//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Zahlenvalidator                                                       				//
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include "NumberValidator.h"

//////////////////////////////////////////////////////////////////////////////////////////
NumberValidator::NumberValidator(double min, double max, int decimals, QObject* parent)
               : QDoubleValidator(min, max, decimals, parent)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
QValidator::State
NumberValidator::validate(QString& input, int&) const
{
    if (input.isEmpty()) return Intermediate;
    int d = decimals();
    if (d < 0 || d > 6) return Invalid;
    QString tmpInput = input;
    tmpInput.replace(',', '.'); // Auch ',' als Dezimaltrennzeichen erlauben
    if (tmpInput == "-" || tmpInput == "+" || tmpInput == ".") return Intermediate;
    if (tmpInput.length() == 2)
    {   if ((tmpInput[0] == '+' || tmpInput[0] == '-') && tmpInput[1] == '.')
        {   // "+." oder "-."
            return Intermediate;
        }
    }
    bool ok;
    double value = tmpInput.toDouble(&ok);
    if (ok)
    {   int index = tmpInput.indexOf(".");
        int fractionalDigits = index == -1 ? 0 : tmpInput.length()-index-1;
        if (fractionalDigits > decimals()) return Invalid;
    }
    else
    {   return Invalid;
    }
    if (value > top()) return Invalid;
    if (value < bottom())
    {   if (bottom() < 0) return Invalid;
        // Ermöglichen, durch weitere Eingaben in den gültigen Bereich zu gelangen
        return Intermediate;
    }
    return Acceptable;
}

//////////////////////////////////////////////////////////////////////////////////////////
