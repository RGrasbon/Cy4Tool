//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Basisklasse                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include "Global.h"
#include "MainFrame.h"
#include "ServiceMap.h"
#include "AbstractServiceWidget.h"
#include "Conf.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
static bool
isValid(const ServiceProxy& serviceProxy)
{
    return serviceProxy.access() != Service::Unknown &&
           serviceProxy.location().isValid()         &&
           serviceProxy.size() > 0                   &&
           serviceProxy.location() + serviceProxy.size() <= 0xFFFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
createToolTip(const AbstractServiceWidget* serviceWidget, bool accessFailed)
{
    const ServiceProxy& serviceProxy = serviceWidget->service();
    QString issue = serviceWidget->issue();
    if (issue.isEmpty() && accessFailed) issue = QObject::tr("Access failed");
    QString text;
    if (!issue.isEmpty())
    {   text += QString("<b><u>%1:</u></b><p>"
                        "<b>%2</b> %3<p><hr>").arg(QObject::tr("Service widget issues"))
                                              .arg(QChar(0x2192)).arg(issue);
    }
    text += QString("<table>"
                    "<tr><td><b>%1: </b></td><td><i>%2</i></td></tr>"
                    "<tr><td><b>%3: </b></td><td><i>%4</i></td></tr>"
                    "<tr><td><b>%5: </b></td><td><tt>%6h : %7 : %8</tt></td></tr>"
                    "</table>")
                    .arg(QObject::tr("Service"),
                         serviceProxy.name().isEmpty() ? "?" : serviceProxy.name(),
                         QObject::tr("Brief"),
                         serviceProxy.brief(),
                         QObject::tr("Signature"),
                         serviceProxy.location().toText())
                     .arg(serviceProxy.size())
                     .arg(Global::print(serviceProxy.access()));
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Vorzeichenbehaftung aus <jsonValue> holen
//////////////////////////////////////////////////////////////////////////////////////////
ValueDomain
AbstractServiceWidget::toValueDomain(const QJsonObject& jsonObject)
{
    if (!jsonObject.contains(KEY::DOMAIN)) return ValueDomain::Signed;
    QJsonValue jsonValue = jsonObject.value(KEY::DOMAIN);
    bool isSigned   = jsonValue.toString() == VALUE::SIGNED;
    bool isUnsigned = jsonValue.toString() == VALUE::UNSIGNED;
    if (!isSigned && !isUnsigned)
    {   mIssue = tr("Value for \"%1\" must be \"%2\" or \"%3\"")
                    .arg(jsonValue.toString())
                    .arg(VALUE::SIGNED, VALUE::UNSIGNED);
        return ValueDomain::Signed;
    }
    return isSigned ? ValueDomain::Signed : ValueDomain::Unsigned;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Holt das Zahlenformat aus <jsonObject>
//////////////////////////////////////////////////////////////////////////////////////////
NumberFormat
AbstractServiceWidget::toFormat(const QJsonObject&  jsonObject)
{
    NumberFormat format = NumberFormat::Integer;  // Default
    if (jsonObject.contains(KEY::FORMAT))
    {   QString text = jsonObject.value(KEY::FORMAT).toString();
        if      (text == VALUE::FIXPOINT1) format = NumberFormat::Fixpoint1;
        else if (text == VALUE::FIXPOINT2) format = NumberFormat::Fixpoint2;
        else if (text == VALUE::FIXPOINT3) format = NumberFormat::Fixpoint3;
        else if (text == VALUE::FIXPOINT4) format = NumberFormat::Fixpoint4;
        else if (text == VALUE::FIXPOINT5) format = NumberFormat::Fixpoint5;
        else if (text == VALUE::FIXPOINT6) format = NumberFormat::Fixpoint6;
        else if (text == VALUE::FLOAT)     format = NumberFormat::Float;
        else if (text != VALUE::INTEGER)
        {   mIssue = QObject::tr("Unknown value for \"%1\": \"%2\"")
                                 .arg(KEY::FORMAT, text);
        }
    }
    return format;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Holt die Anzahl an Nachkommastellen aus <jsonObject>
//////////////////////////////////////////////////////////////////////////////////////////
Precision
AbstractServiceWidget::toPrecision(const QJsonObject& jsonObject, NumberFormat format)
{
    int precision;
    if (jsonObject.contains(KEY::PRECISION))
    {   if (!jsonObject.value(KEY::PRECISION).isDouble())
        {   mIssue = QObject::tr("\"%1\" must be a number").arg(KEY::PRECISION);
        }
        precision = jsonObject.value("precision").toInt(0);
        if (precision < 0 || precision > 7)
        {   mIssue = QObject::tr("Value for \"%1\" must be in the range 0 .. 6")
                                .arg(KEY::PRECISION);
            precision = qBound(0, precision, 7);
        }
    }
    else
    {   if      (format == NumberFormat::Integer) precision = 0;
        else if (format == NumberFormat::Float)   precision = 6;
        else                                      precision = int(format); // Fixpoint
    }
    if (format == NumberFormat::Integer && precision != 0)
    {   mIssue = QObject::tr("For number-format \"%1\", \"%2\" must be 0")
                            .arg(VALUE::INTEGER, KEY::PRECISION);
        precision = 0;
    }
    return Precision(precision);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Holt den Wertebereich unter Berücksichtigung des Zahlenformats aus <jsonObject>
//////////////////////////////////////////////////////////////////////////////////////////
AbstractServiceWidget::NumberRange
AbstractServiceWidget::toRange(const QJsonObject& jsonObject,
                               NumberFormat       format,
                               ValueDomain        domain,
                               int                size)
{
    static const double   limit[][7][4] =
    {   // [0] Untergrenze vorzeichenbehaftet
        { {     -128.0,   -32768.0, -8388608.0, -2147483648.0 },   // Integer
          {     -12.80,   -3276.80, -838860.80, -214748364.80 },   // Fixpoint1
          {     -1.280,   -327.680, -83886.080, -21474836.480 },   // Fixpoint2
          {    -0.1280,   -32.7680, -8388.6080, -2147483.6480 },   // Fixpoint3
          {   -0.01280,   -3.27680, -838.86080, -214748.36480 },   // Fixpoint4
          {  -0.001280,  -0.327680, -83.886080, -21474.836480 },   // Fixpoint5
          { -0.0001280, -0.0327680, -8.3886080, -2147.4836480 }    // Fixpoint6
        },
        // [1] Obergrenze vorzeichenbehaftet
        { {     +127.0,   +32767.0, +8388607.0, +2147483647.0 },   // Integer
          {     +12.70,   +3276.70, +838860.70, +214748364.70 },   // Fixpoint1
          {     +1.270,   +327.670, +83886.070, +21474836.470 },   // Fixpoint2
          {    +0.1270,   +32.7670, +8388.6070, +2147483.6470 },   // Fixpoint3
          {   +0.01270,   +3.27670, +838.86070, +214748.36470 },   // Fixpoint4
          {  +0.001270,  +0.327670, +83.886070, +21474.836470 },   // Fixpoint5
          { +0.0001270, +0.0327670, +8.3886070, +2147.4836470 }    // Fixpoint6
        },
        // [2] Obergrenze vorzeichenlos
        { {      255.0,    65535.0, 16777215.0,  4294967295.0 },   // Integer
          {      25.50,    6553.50, 1677721.50,  429496729.50 },   // Fixpoint1
          {      2.550,    655.350, 167772.150,  42949672.950 },   // Fixpoint2
          {     0.2550,    65.5350, 16777.2150,  4294967.2950 },   // Fixpoint3
          {    0.02550,    6.55350, 1677.72150,  429496.72950 },   // Fixpoint4
          {   0.002550,   0.655350, 167.772150,  42949.672950 },   // Fixpoint5
          {  0.0002550,  0.0655350, 16.7772150,  4294.9672950 }    // Fixpoint6
        }
    };
    NumberRange range;
    if (jsonObject.contains(KEY::MIN) && !jsonObject.value(KEY::MIN).isDouble())
    {   mIssue = QObject::tr("\"%1\" must be a number").arg(KEY::MIN);
    }
    if (jsonObject.contains(KEY::MAX) && !jsonObject.value(KEY::MAX).isDouble())
    {   mIssue = QObject::tr("\"%1\" must be a number").arg(KEY::MAX);
    }
    range.min = jsonObject.value(KEY::MIN).toDouble(std::numeric_limits<double>::min());
    range.max = jsonObject.value(KEY::MAX).toDouble(std::numeric_limits<double>::max());
    int index = qBound(1, size, 4)-1;
    if (format != NumberFormat::Float)
    {   double lowerLimit = 0;
        double upperLimit = 0;
        switch (domain)
        {   case ValueDomain::Signed:
                lowerLimit = limit[0][int(format)][index];
                upperLimit = limit[1][int(format)][index];
                break;
            case ValueDomain::Unsigned:
                lowerLimit = 0.0;
                upperLimit = limit[2][int(format)][index];
                break;
            default:
                assert(false);
        }
        if (!jsonObject.contains(KEY::MIN))
        {   range.min = lowerLimit;
        }
        else if (range.min < lowerLimit)
        {   mIssue = QObject::tr("Specified minimum of %1 is below the value type limit (%2)")
                                 .arg(range.min).arg(lowerLimit);
        }
        if (!jsonObject.contains(KEY::MAX))
        {   range.max = upperLimit;
        }
        else if (range.max > upperLimit)
        {   mIssue = QObject::tr("Specified maximum of %1 is above the value type limit (%2)")
                                 .arg(range.max).arg(upperLimit);
        }
        range.min = qMax(range.min, lowerLimit);
        range.max = qMin(range.max, upperLimit);
    }
    if (range.min > range.max)
    {   range = NumberRange();
        mIssue = QObject::tr("\"%1\" must be less than \"%2\"").arg(KEY::MIN, KEY::MAX);
    }
    return range;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Holt aus <jsonObject> eine Liste mit Werten, denen eine spezielle Bedeutung zugeodnet ist
//////////////////////////////////////////////////////////////////////////////////////////
QMap<double, QString>
AbstractServiceWidget::toSpecialValues(const QJsonObject& jsonObject)
{
    QMap<double, QString> map;
    if (jsonObject.contains(KEY::SPECIALS))
    {   QJsonObject jsonMapObject = jsonObject.value(KEY::SPECIALS).toObject();
        QStringList keyList = jsonMapObject.keys();
        for(const QString& key : std::as_const(keyList))
        {   bool ok;
            double value = key.toDouble(&ok);
            if (ok)
            {   QString name = Global::jsonLanguageEntry(jsonMapObject[key]);
                map[value] = name;
            }
            else
            {   mIssue = QObject::tr("\"%1\": Entry \"%2\" must be a number")
                                     .arg(KEY::SPECIALS, key);
            }
        }
    }
    return map;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Zahlenwert aus <jsonValue> holen
//////////////////////////////////////////////////////////////////////////////////////////
SInt32
AbstractServiceWidget::toNumber(const QJsonValue& jsonValue, ValueDomain valueDomain, SInt32 deflt)
{
    if (mServiceProxy.size() > 4)
    {   mIssue = tr("Service size must be 1, 2, 3 or 4");
        return deflt;
    }
    if (jsonValue.isUndefined()) return deflt;
    SInt64 value;
    value = jsonValue.toInteger(INT64_MIN);
    if (value == INT64_MIN)
    {   mIssue = tr("Invalid number: %1").arg(jsonValue.toString());
        return deflt;
    }
    static const SInt64 limits[][2][2]
    {   //              Signed           ,        Unsigned
        //   ----------------------------+-------------------------
        { {       -0x80LL,       0x7FLL }, { 0ULL,       0xFFULL }},  // [0]  8 Bit
        { {     -0x8000LL,     0x7FFFLL }, { 0ULL,     0xFFFFULL }},  // [1] 16 Bit
        { {   -0x800000LL,  0x7FFFFFFLL }, { 0ULL,   0xFFFFFFULL }},  // [2] 24 Bit
        { { -0x80000000LL, 0x7FFFFFFFLL }, { 0ULL, 0xFFFFFFFFULL }}   // [3] 32 Bit
    };
    int row    = mServiceProxy.size()-1;
    int column = int(valueDomain == ValueDomain::Unsigned);
    SInt64 min = limits[row][int(column)][0];
    SInt64 max = limits[row][int(column)][1];
    {   if (value < min || value > max)
        {   mIssue = tr("Value %d out of range: %1 .. %2").arg(value).arg(min).arg(max);
            return deflt;
        }
    }
    return SInt32(value);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt ein ByteArray aus <value> unter Bedrücksichtigung des Formates und der Dienstbreite
//////////////////////////////////////////////////////////////////////////////////////////
ByteArray
AbstractServiceWidget::serialize(double value, NumberFormat format) const
{
    return serialize(value, format, mServiceProxy.size());
}

//////////////////////////////////////////////////////////////////////////////////////////
ByteArray
AbstractServiceWidget::serialize(double value, NumberFormat format, int size) const
{
    assert(size >= 1 && size <= 4);
    UInt32 buffer = 0;
    if (format == NumberFormat::Float)
    {   if (size != 4 || sizeof(float) != 4) return ByteArray();
        float floatValue = static_cast<float>(value);
        std::memcpy(&buffer, &floatValue, 4);
    }
    else
    {   int index = int(format);
        constexpr SInt32 multiplier[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
        if (index >= CARDINALITY(multiplier)) return ByteArray();
        buffer = UInt32(qRound(value * multiplier[index]));
    }
    ByteArray data(size);
    for (int i=0; i<size; i++)
    {   data[i] = UInt08(buffer >> (size-i-1)*8);
    }
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Extrahiert einen Zahlenwert aus <data> unter Bedrücksichtigung der Vorzeichenbehaftung
// in <valueDomain> und des Formates in <format>
//////////////////////////////////////////////////////////////////////////////////////////
double
AbstractServiceWidget::deserialize(const ByteArray& data, ValueDomain valueDomain, NumberFormat format) const
{
    USize size = data.size();
    assert(size >= 1 && size <= 4);
    UInt32 buffer = 0;
    for (USize i=0; i<size; i++)
    {   buffer <<= 8;
        buffer |= UInt32(data[i]);
    }
    double value = NAN;
    if (format == NumberFormat::Float)
    {   if (size == 4)
        {   float tmpFloat;
            std::memcpy(&tmpFloat, &buffer, sizeof(float));
            value = double(tmpFloat);
        }
    }
    else
    {   switch (valueDomain)
        {   case ValueDomain::Signed:
                switch (size)
                {   case 1:  value = (buffer & 0x000080) ? SInt32(buffer | 0xFFFFFF00) : SInt32(buffer); break;
                    case 2:  value = (buffer & 0x008000) ? SInt32(buffer | 0xFFFF0000) : SInt32(buffer); break;
                    case 3:  value = (buffer & 0x800000) ? SInt32(buffer | 0xFF000000) : SInt32(buffer); break;
                    case 4:  value = SInt32(buffer);                                                     break;
                    default: ;
                }
                break;
            case ValueDomain::Unsigned:
                if (size >= 1 && size <= 4) value = buffer;
                break;
            default:
                assert(false);
        }
        if (value != NAN)
        {   int index = int(format);
            constexpr SInt32 divisor[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
            assert(index < CARDINALITY(divisor));
            value /= divisor[index];
        }
    }
    return value;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AbstractServiceWidget::notifyStatus(void) const
{
    if (!mIsOperational)
    {   mainFrame().showStatusBarWarning(tr("Widget issue: %1").arg(mIssue));
    }
    else if(mServiceProxy.access() == Service::ReadOnly)
    {   mainFrame().showStatusBarInfo(tr("Widget is read only!"));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AbstractServiceWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {   notifyStatus();
    }
    QWidget::mousePressEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
AbstractServiceWidget::AbstractServiceWidget(const ServiceProxy& serviceProxy, QWidget* parent)
                     : QWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    mServiceProxy = serviceProxy;
    mIsOperational = true;
    int index = serviceMap().indexOf(mServiceProxy.location(), mServiceProxy.size(), mServiceProxy.access());
    if (index == INVALID)
    {   mIssue = tr("Service merge failure in service map");
        mIsOperational = false;
        return;
    }
    else if (!mServiceProxy.location().isValid())
    {   mIssue = tr("Invalid \"%1\"").arg(KEY::LOCATION);
        mIsOperational = false;
    }
    else if (mServiceProxy.size() < 1 || mServiceProxy.size() > CFG::MaximumServiceSize)
    {   mIssue = tr("Invalid \"%1\": %2").arg(KEY::SIZE).arg(mServiceProxy.size());
        mIsOperational = false;
    }
    else if (mServiceProxy.access() == Service::Unknown)
    {   mIssue = tr("Unknown \"%1\"").arg(KEY::ACCESS);
        mIsOperational = false;
    }
    else if (mServiceProxy.location() + mServiceProxy.size() > 0xFFFFFF)
    {   mIssue = tr("Service exceeds location FFFFFFh");
        mIsOperational = false;
    }
    else if (isValid(mServiceProxy) && mServiceProxy.name().isEmpty())
    {   mIssue = tr("Entry \"%1\" missing").arg(KEY::NAME);
        mIsOperational = true;
    }
    else if (serviceMap().tag(index) != ServiceMap::Valid)
    {   mIssue = tr("Incorrect service map");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AbstractServiceWidget::styleWidget(void)
{
    QString styleSheet;
    bool isValid = mIsOperational && mIssue.isEmpty();
    QColor color;
    if (!isValid)
    {   color = QColor(255, 150,  25);
    }
    else if (mServiceProxy.accessFailed())
    {   mServiceProxy.accessFailed();
        color = QColor(255, 255, 128);
    }
    else
    {   QWidget dummyWidget;
        color = dummyWidget.palette().color(QPalette::Window);
    }
    QString toolTip = createToolTip(this, mServiceProxy.accessFailed());
    setToolTip(toolTip);
    QPalette tmpPalette = palette();
    tmpPalette.setColor(QPalette::Window, color);
    setPalette(tmpPalette);
    setAutoFillBackground(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
AbstractServiceWidget::issue(void) const
{
    return mIssue;
}

//////////////////////////////////////////////////////////////////////////////////////////
const ServiceProxy&
AbstractServiceWidget::service(void) const
{
    return mServiceProxy;
}

//////////////////////////////////////////////////////////////////////////////////////////

