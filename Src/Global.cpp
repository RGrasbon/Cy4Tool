//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Globale Datenstrukturen                                                             //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDesktopServices>
#include <QEventLoop>
#include <QStyleOption>
#include "TypeDefs.h"
#include "Global.h"
#include "MainFrame.h"
#include "Settings.h"
#include "DeviceListPage.h"
#include "ServiceListPage.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  evaluate(): Ermittelt den ganzzahligen Wert des angegebenen Terms. evaluate() ist   //
//  ----------  die rekursive Hilfsfunktion von evaluate(const QString&).               //
//                                                                                      //
//  Eingabe: · a: Zeiger auf das erste Zeichen der auszuwertenden Zeichenkette.         //
//           · b: Zeiger auf das erste Zeichen hinter der auszuwertenden Zeichenkette.  //
//  Ausgabe: Der ermittelte Wert oder INVALID, wenn der Term nicht ausgewertet werden   //
//           konnte.                                                                    //
//////////////////////////////////////////////////////////////////////////////////////////
static SInt32
evaluate(const char* a, const char* b)
{
    if (a >= b) return INVALID;     // Ungültiger oder leerer Term
    // Trim(), Term von umgebende Leerzeichen freistellen
    while (*a== ' ' || *a== '\t')
    {   a++;
        if( a==b ) return INVALID;  // Leer
    }
    while( *(b-1) == ' ' || *(b-1) == '\t' ) b--;
    // Letzten Term identifizieren
    const char* op = nullptr;
    const char* c  = b;
    while (a < c)
    {   int x = 0;
        while (a < c)
        {   c--;
            if (*c == ')') x--;
            if (*c == '(') x++;
            if (x == 0)
            {   if (*c == '+' || *c == '-' || *c == '*' || *c == '/')
                {   if( op == NULL ) op = c;
                    break;
                }
            }
        }
        if (*c == '+' || *c == '-')
        {   op = c;
            break;
        }
    }
    // Term ist Ausdruck ... also auswerten
    if (op != NULL)
    {   c = op;
        if (*op == '+' || *op == '-')
        {   do
            {   if( c == a ) break;
                c--;
            } while (*c == ' ' || *c == '\t');
            int x = 0;
            if (*c == '*' || *c == '/') op = c;
            if (c != op)
            {   if (*c == '+' || *c == '-') op = c;
                x = evaluate(a, op);
                if( x == INVALID ) return INVALID;
            }
            int y = evaluate(op+1, b);
            if( y == INVALID ) return INVALID;
            if( *op == '+' )
            {   SInt64 result = x + y;
                if (result >  SInt64(0x7FFFFFFF)) return INVALID; // Überlauf
                if (result < -SInt64(0x80000000)) return INVALID; // Unterlauf
                return SInt32(result);
            }
            if( *op == '-' )
            {   SInt64 result = x - y;
                if (result >  SInt64(0x7FFFFFFF)) return INVALID; // Überlauf
                if (result < -SInt64(0x80000000)) return INVALID; // Unterlauf
                return SInt32(result);
            }
        }
        int x = evaluate(a, op);
        if (x == INVALID) return INVALID;
        int y = evaluate(op+1, b);
        if (y == INVALID) return INVALID;
        switch (*op)
        {   case '*':
            {   SInt64 result = x * y;
                if (result >  SInt64(0x7FFFFFFF)) return INVALID; // Überlauf
                if (result < -SInt64(0x80000000)) return INVALID; // Unterlauf
                return SInt32(result);
            }
            case '/':
                if (y == 0) return INVALID;    // Division durch Null
                return x / y;
            default:
                return INVALID;
        }
    }
    // Verarbeiten: Klammerung "(123)"
    if( *a == '(' ) return evaluate(a+1, b-1);
    // Verarbeiten: Zahl "123"
    int base;
    if( b-a > 2 && *a == '0' &&  (*(a+1) == 'x' || *(a+1) == 'X') )
    {   // Hexadezimal-Präfix: "0x"
        base = 16;
        a+=2;   // Hex-Präfix überspringen
    }
    else if (*(b-1) == 'h' || *(b-1) == 'H')
    {   // Hexadezimal-Posfix: "h"
        base = 16;
        b--;    // Hex-Posfix abschneiden
    }
    else
    {   // Dezimal
        base = 10;
    }
    SInt64 value = 0;
    while (a < b)
    {   char c = *a;
        int n;
        if      (              c >= '0' && c <= '9') n =  0 + c - '0';
        else if (base == 16 && c >= 'a' && c <= 'f') n = 10 + c - 'a';
        else if (base == 16 && c >= 'A' && c <= 'F') n = 10 + c - 'A';
        else                                         return INVALID;
        value = value * base + n;
        if (value >  SInt64(0x7FFFFFFF)) return INVALID; // Überlauf
        if (value < -SInt64(0x80000000)) return INVALID; // Unterlauf
        a++;
    }
    return SInt32(value);
}

//////////////////////////////////////////////////////////////////////////////////////////
static int
lineNumber(int position, const QByteArray& text)
{
    if (position < 0 || position >= text.length()) return INVALID;
    int line = 1;
    int i = 0;
    while (i < text.length() && position > 0)
    {   if( text[i] == '\r' )
        {   i++;
            if (i < text.length() && text[i] == '\n') i++;
            line++;
        }
        else if (text[i] == '\n')
        {   line++;
        }
        i++;
        position--;
    }
    return line;
}

//////////////////////////////////////////////////////////////////////////////////////////
namespace Global
{

//////////////////////////////////////////////////////////////////////////////////////////
void
connectToGateway(void)
{
    mainFrame().connectToGateway();
}

//////////////////////////////////////////////////////////////////////////////////////////
QStringList
translations(void)
{
    // Liste mit BCP47-Codes auf Basis der verfügbaren Übersetzungsdateien zusammenstellen
    QStringList list("en-US"); // Grundsprache Englisch (US) ist obligatorisch
    {   QDir dir(":Trans/");
        QStringList fileList = dir.entryList(QStringList("*.qm"));
        for (int i=0; i<fileList.count(); i++)
        {   QString code = fileList[i];
            int start  = sizeof("trans_")-1;
            int length = code.length()-(start+int(sizeof(".qm"))-1);
            code = code.mid(start, length);
            list.append(code);
        }
    }
    return list;
}

//////////////////////////////////////////////////////////////////////////////////////////
QLocale
toLocal(const QString& code, const QLocale& deflt)
{
    QStringList codeList = translations();
    for (int i=0; i<codeList.count(); i++)
    {   if (code == codeList[i]) return QLocale(code);
    }
    return deflt;
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
toCode(const QLocale& locale)
{
    QString name = locale.name();
    return name.replace("_", "-");         // "de_DE" -> "de-DE"
}

//////////////////////////////////////////////////////////////////////////////////////////
void
updateLanguage(void)
{
    mainFrame().updateLanguage();
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Konvention bei Bezeichnung von Pfad-Segmenten:                                      //
//                                                                                      //
//    "/DirectoryA/DirectoryB/File.c"                                                   //
//     :           :        : :    :                                                    //
//     |---------------------------| → 'filePath':      "/DirectoryA/DirectoryB/File.c" //
//     :           :        : :    :                                                    //
//     |--------------------| :    : → 'directoryPath': "/DirectoryA/DirectoryB"        //
//                 :        : :    :                                                    //
//                 :        : |----| → 'fileName':      "File.c"                        //
//                 :        :                                                           //
//                 |--------|        → 'directoryName': "DirectoryB"                    //
//                                                                                      //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////
bool
load(const QString& filePath, QByteArray& data)
{
    QString issue;
    return load(filePath, data, issue);
}

bool
load(const QString& filePath, QByteArray& data, QString& issue)
{
    data.clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {   if (file.exists()) issue = QObject::tr("File open failure: \"%1\"").arg(filePath);
        else               issue = QObject::tr("No such file: \"%1\"").arg(filePath);
        return false;
    }
    if (file.size() > 0xFFFFFF)
    {   issue = QObject::tr("File size exceeds 16 Mib: \"%1\"")
                            .arg(QFileInfo(filePath).fileName());
        return false;
    }
    data = file.readAll();
    issue.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
load(const QString& filePath, QString& text)
{
    QString issue;
    return load(filePath, text, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
load(const QString& filePath, QString& text, QString& issue)
{
    QByteArray data;
    if (!load(filePath, data, issue)) return false;
    text = data;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
load(const QString& filePath, QJsonObject& jsonObject)
{
    QString issue;
    return load(filePath, jsonObject, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
load(const QString& filePath, QJsonObject& jsonObject, QString& issue)
{
    jsonObject = QJsonObject();
    QByteArray data;
    if (!load(filePath, data, issue)) return false;
    if (!data.isEmpty())
    {   QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError)
        {   int lineNr = lineNumber(error.offset, data);
            if (lineNr == INVALID)
            {   issue = QObject::tr("JSON parsing failure: %1")
                                    .arg(error.errorString());
            }
            else
            {   issue = QObject::tr("JSON parsing failure near line %1: %2")
                                    .arg(lineNr)
                                    .arg(error.errorString());
            }
            return false;
        }
        jsonObject = document.object();
    }
    issue.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
save(const QString& filePath, const QByteArray& data)
{
    QString issue;
    return save(filePath, data, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
save(const QString& filePath, const QByteArray& data, QString& issue)
{
    QFile file(filePath);
    if (filePath.isEmpty())
    {   issue = QObject::tr("File write failure: Empty file name given");
        return false;
    }
    if (!file.open(QIODevice::WriteOnly))
    {   issue = QObject::tr("File open failure: \"%1\"").arg(filePath);
        return false;
    }
    if (file.write(data) != data.length())
    {   issue = QObject::tr("File write failure: \"%1\"").arg(filePath);
        return false;
    }
    issue.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
save(const QString& filePath, const QString& text)
{
    QString issue;
    return save(filePath, text, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
save(const QString& filePath, const QString& text, QString& issue)
{
    QByteArray data = text.toUtf8();
    return save(filePath, data, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
save(const QString& filePath, const QJsonObject& jsonObject)
{
    QString issue;
    return save(filePath, jsonObject, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
save(const QString& filePath, const QJsonObject& jsonObject, QString& issue)
{
    QJsonDocument document(jsonObject);
    QByteArray data = document.toJson();
    return save(filePath, data, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
removeFile(const QString& filePath)
{
    return QFile::remove(filePath);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
copyFile(const QString& sourceFilePath, const QString& destinationFilePath)
{
    if (!pathExists(sourceFilePath)) return false;
    if (pathExists(destinationFilePath)) removeFile(destinationFilePath);
    return QFile::copy(sourceFilePath, destinationFilePath);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
pathExists(const QString& path)
{
    return QFile(path).exists();
}

//////////////////////////////////////////////////////////////////////////////////////////
QStringList
directoryContent(const QString& directoryPath, const QString& filter)
{
    QStringList filterList;
    if (!filter.isEmpty()) filterList.append(filter);
    QDir directory(directoryPath);
    return directory.entryList(filterList, QDir::Files, QDir::Name);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  evaluate(): Ermittelt den ganzzahligen Wert des angegebenen Ausdrucks. Als Eingabe  //
//  ----------  akzeptiert die Funktion ganze Dezimal- oder Hexadezimalzahlen, zweitere //
//  müssen durch ein 'h'-Postfix oder durch den C-Präfix "0x" gekennzeichnet sein. Als  //
//  Operationen stehen die vier Grundrechenarten ('+', '-' '*' und '/') zur Verfügung.  //
//                                                                                      //
//      Beispiele:                                                                      //
//                                                                                      //
//      int result;                                                                     //
//                                                                                      //
//      result = evaluate("");                             // INVALID ("")              //
//      result = evaluate("60-40-20");                     // 0                         //
//      result = evaluate("  + - 5");                      // INVALID ("+-...")         //
//      result = evaluate("5 + - 5");                      // 0                         //
//      result = evaluate("((1+2))");                      // 3                         //
//      result = evaluate("(( (  1 ) + ( 2)  ) )");        // 3                         //
//      result = evaluate("\t((\t1\t+\t2\t)\t)\t");        // 3                         //
//      result = evaluate("1+ (2  *2 )");                  // 5                         //
//      result = evaluate("(2/2 )+14");                    // 15                        //
//      result = evaluate("3*3+2*2");                      // 13                        //
//      result = evaluate("3*(3+2)*2");                    // 30                        //
//      result = evaluate("() + (2*2)");                   // INVALID ("()")            //
//      result = evaluate("+(2*2)+");                      // INVALID ("...+")          //
//      result = evaluate("60*-4020");                     // -241200                   //
//      result = evaluate("-2+2");                         // 0                         //
//      result = evaluate("-(2+3)");                       // -5                        //
//      result = evaluate("--(2*3)");                      // INVALID ("--...")         //
//      result = evaluate("((1)+4*100)*(3)/(5-+(4))");     // 1203                      //
//      result = evaluate("2/-1");                         // -2                        //
//      result = evaluate("60h*-4020H");                   // -1575936                  //
//      result = evaluate("0x60*-0X4020");                 // -1575936                  //
//      result = evaluate("-2h+2h");                       // 0                         //
//      result = evaluate("-0x2+0X2");                     // 0                         //
//      result = evaluate("-(2h+3h)");                     // -5                        //
//      result = evaluate("-(x2+0x3)");                    // -5                        //
//      result = evaluate("((1h)+4H*64h)/(5h-+(4h))");     // 401                       //
//      result = evaluate("((0x1)+0x4*0X64)/(0x5-+(0x4))"; // 401                       //
//      result = evaluate("12 / 2*(1+1+1)");               // 18                        //
//                                                                                      //
//  Eingabe: · expression: Der auszuwertende Ausdruck.                                  //
//  Ausgabe: Der ermittelte Wert oder INVALID, wenn der Ausdruck nicht ausgewertet      //
//           werden konnte.                                                             //
//////////////////////////////////////////////////////////////////////////////////////////
int
evaluate(const QString& expression)
{
    QByteArray string = expression.toLatin1().data();
    const char* a = string.constData();
    const char* b = string.constData() + string.length();
    return ::evaluate(a, b);
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
jsonLanguageEntry(const QJsonValue& jsonValue, bool languageFallback)
{
    if (jsonValue.isString()) return QJsonValue(jsonValue).toString();
    QJsonObject jsonObject = jsonValue.toObject();
    if( jsonObject.isEmpty() ) return QString();
    QString cc = toCode(settings().locale).first(2); // "de_DE" -> "de"
    if (jsonObject.contains(cc)) return jsonObject.value(cc).toString();
    if (languageFallback || settings().allowLanguageFallback)
    {   // Alternative Übersetzung anzeigen, wenn Eintrag in der Sprache nicht verfügbar
        QJsonObject::const_iterator it = jsonObject.constBegin();
        if (it != jsonObject.constEnd()) return it.value().toString();
    }
    return QString("[%1] %2").arg(cc, QObject::tr("n.a."));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
doMainLoop(void)
{
    QEventLoop  eventLoop;
    QTimer      timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timer.start(10);
    eventLoop.exec();
    timer.stop();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
mapFileNameFor(const Cy4ApplicationType& type)
{
    if (!type.isValid()) return QString();
    return QString("%1.json").arg(type.toText());
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
mapFilePathFor(const Cy4ApplicationType& type)
{
    if (!type.isValid()) return QString();
    QString directoryPath = settings().localServiceMapsDirectoryPath;
    QString fileName   = mapFileNameFor(type);
    return QString("%1/%2").arg(directoryPath, fileName);
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
mapFileUrlFor(const Cy4ApplicationType& type)
{
    if (!type.isValid()) return QString();
    return QString("%1/%2/%3/%4").arg(settings().onlineResourcesUrl,
                                      CFG::ToolDataDirectoryName,
                                      CFG::ServiceMapsDirectoryName,
                                      Global::mapFileNameFor(type));
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
openFileInEditor(const QString& filePath, QString* issue)
{
    QUrl url(QString("file:///%1").arg(filePath));
    if( !url.isValid() )
    {   if (issue != nullptr) *issue = QObject::tr("Invalid URL %1").arg(url.toString());
        return false;
    }
    if( !QDesktopServices::openUrl(url) )
    {   QString suffix = QFileInfo(filePath).suffix();
        if (issue != nullptr) *issue = QObject::tr("No application found for file type "
                                                   "\"*.%1\".").arg(suffix);
        return false;
    }
    if (issue != nullptr) issue->clear();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
print(Service::Access access)
{
    QString text;
    switch (access)
    {   case Service::ReadOnly:  text = "R";  break;
        case Service::WriteOnly: text = "W";  break;
        case Service::ReadWrite: text = "RW"; break;
        default:                 text = "?";
    }
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4Device*
currentDevice(void)
{
    DeviceListPage* deviceListPage = qobject_cast<DeviceListPage*>(mainFrame().page(Page::DeviceList));
    return deviceListPage->currentDevice();
}

//////////////////////////////////////////////////////////////////////////////////////////
const Service*
currentService(void)
{
    ServiceListPage* serviceListPage = qobject_cast<ServiceListPage*>(mainFrame().page(Page::ServiceList));
    return serviceListPage->currentService();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Platzbedarf von <text> abschätzen [Pixels]
//////////////////////////////////////////////////////////////////////////////////////////
int
calculateWidthFor(const QString& text, const QLineEdit* lineEdit)
{
    QFontMetrics metric(lineEdit->font());
    int textWidth = metric.horizontalAdvance(text);
    QStyleOptionFrame option;
    option.lineWidth = lineEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int width = CFG::QLineEditHorizontalPading*2 + textWidth + 2*option.lineWidth;
    return qBound(16, width, 500);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt eine Zeichenkette, die der Zahl <value> mit <precision> Nachkommastellen
// entspricht
QString
createMeasurementTextFor(double value, int precision)
{
    if (value == -qInf() || value == +qInf()) value = 88888.8888; // Ersatzwert
    QString text = QString::number(value, 'f', precision);
    for (int i=0; i<text.length(); i++) if( text[i].isDigit() ) text[i] = '8';
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
} // namespace Global
