//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4NET Schnittstelle                                                                //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//   Cy4 ASCII PDU Synopsis:                                                            //
//   -----------------------                                                            //
//                                      XX XX XX XXXXXX XX XX XX ... XX XX              //
//                                      |  |  |    |    |  |  |      |  |               //
//                                      |  |  |    |    |  |  |      |  |               //
//           DTF:   "C0"   ..   "C5" ---+  |  |    |    |  |  |      |  |               //
//           NET:   "00"   ..   "FF" ------+  |    |    |  |  |      |  |               //
//           DEV:   "00"   ..   "FF" ---------+    |    |  |  |      |  |               //
//           LOC: "000000" .. "FFFFFE" ------------+    |  |  |      |  |               //
//           CNT:   "01"   ..   "40" -------------------+  |  |      |  |               //
//           DAT 1: "00"   ..   "FF" ----------------------+  |      |  |               //
//           DAT 2: "00"   ..   "FF" -------------------------+      |  |               //
//            :      ::    ..    ::                                  |  |               //
//           DAT n: "00"   ..   "FF" --------------------------------+  |               //
//           CRC:   "00"   ..   "FF" -----------------------------------+               //
//                                                                                      //
//                                                                                      //
//   · DTF: Datagramm Typflags (Datagram Type Flags): Die oberen 5 Bits enthalten die   //
//          Datagramm-Identifikationsbits DIB (Datagram Identification Bits). Cy4NET-   //
//          ASCII-PDUs identifizieren sich hier stets mit der Bitfolge: 11000. Übrig    //
//          bleiben Bit 2, 1 und 0, die die Charakteristik des Datagramms bestimmen:    //
//                                                                                      //
//                            7 6 5 4 3 2 1 0                                           //
//                           +-+-+-+-+-+-+-+-+                                          //
//                           |1|1|0|0|0|F|D|P|                                          //
//                           +-+-+-+-+-+-+-+-+                                          //
//                            \_______/ | | |                                           //
//                               DIB    | | +---- P: Nutzlast-Flag: 1 = Führt Nutzlast  //
//                                      | |          mit, 0 = keine Nutzlast.           //
//                                      | +------ D: Datagramm-Richtungs-Flag (1 = An-  //
//                                      |            frage, 0 = Antwort.                //
//                                      +-------- F: Fehler-Flag: 1 = Zugriffsfehler,   //
//                                                   0 = Antwort ist korrekt.           //
//                                                                                      //
//              Mit den Bits 'F', 'D' und 'P' lassen sich somit acht Wertausprägungen   //
//              für das DTF kombinieren. Da es allerdings keinen Sinn ergibt, eine als  //
//              fehlerhaft gekennzeichnete Anfrage zu senden, kommen die Typaus-        //
//              prägungenwerden C6h und C7h nicht zum Einsatz. Folgende ASCII-Data-     //
//              gramm-Typen gibt es:                                                    //
//                                                                                      //
//                          DIB  FDP | DTF-Wert | Typ                                   //
//                        -----------+----------+---------------                        //
//                         11000 010 |   C2h    | Leseanforderung                       //
//                         11000 001 |   C1h    | Leseantwort                           //
//                         11000 101 |   C5h    | Lesefehler                            //
//                         11000 011 |   C3h    | Schreibaufforderung                   //
//                         11000 000 |   C0h    | Schreibbestätigung                    //
//                         11000 100 |   C4h    | Schreibfehler                         //
//                                                                                      //
//   · NET: Netzadresse des Empfängerknotens.                                           //
//                                                                                      //
//   · DEV: Geräteadresse des Empfängerknotens.                                         //
//                                                                                      //
//   · LOC: Dienstadresse. LOC ist eine vorzeichenlose 24-Bit-Zahl im Bereich von 0 bis //
//          FFFFFEh, FFFFFFh ist nicht erlaubt. Rundrufe sind festgelegt auf den        //
//          Bereich von E00000h bis EFFFFFh. Der Adressbereich unter 100000h ist für    //
//          Zugriffe des Programmladermodules reserviert. <LOC> hat die Byte-Reihen-    //
//          folge Big Endian (High/Mid/Low)                                             //
//                                                                                      //
//   · CNT:  Je nach Datagrammtyp, die Anzahl der                                       //
//                 · mitgeführten  (bei Schreibaufforderungen)                          //
//                 · geschriebenen (bei Schreibbestätigungen bzw. Schreibfehlern)       //
//                 · angeforderten (bei Leseanforderungen)                              //
//                 · gelesenen     (bei Leseantworten bzw. Lesefehlern)                 //
//              Datenbytes. Wertebereich: 1 .. 64                                       //
//                                                                                      //
//   · DAT:  Nutzlast. Datagramme vom Typ Schreibaufforderung und Leseantwort führen in //
//           dieser Sektion <CNT> Datenbytes mit sich (siehe 'P' Flag im DTF). Bei      //
//           Schreibbestätigungen, Leseanforderungen und den Fehlern ist die Sektion    //
//           DAT leer.                                                                  //
//                                                                                      //
//   · CRC:  8-Bit Dow-CRC über alle Datagrammbytes, außer CRC.                         //
//                                                                                      //
//                                                                                      //
//                                                                                      //
//  Beispiel 1: "CallTypePlate" von Gerät an Adresse 255.3                              //
//                                                                                      //
//      Leseanforderung: "C2FF03FFFFF00717"                                             //
//      Leseantwort:     "C1FF03FFFFF007A0012008B00023EA"                               //
//                                                                                      //
//                                                                                      //
//  Beispiel 2: "EnterApplication" von Gerät an Adresse 1.6                             //
//                                                                                      //
//      Schreibaufforderung: "C30106FFFFF8015EC6"                                       //
//      Schreibbestätigung:  "C00106FFFFF80175"                                         //
//                                                                                      //
//                                                                                      //
//  Beispiel 3: "CallTypePlate" von einem nicht existierenden Gerät an Adresse 20.3     //
//                                                                                      //
//      Leseanforderung: "C21403FFFFF007C6"                                             //
//      Lesefehler:      "C51403FFFFF00775"                                             //
//                                                                                      //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

#ifdef CY4_MESSAGE_MONITOR
    #include <iostream>
#endif
#include <QTcpSocket>
#include <QSerialPort>
#include <QByteArray>
#include "Cy4Net.h"

using namespace std;

static const int GATEWAY_RESPONSE_TIMEOUT = 1500; // Zeitgrenze für Antwort vom Gateway [ms]
static const int PAUSE_TIME               = 1000; // Länge einer "Posting-Pause"

//////////////////////////////////////////////////////////////////////////////////////////
static void
updateCRC(UInt08 data, UInt08& crc)
{
    crc ^= data;
    for (UInt08 i=0; i<8; i++) crc = (crc >> 1) ^ ((crc & 1) ? 0x8C : 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
printChar(char*& text, char c)
{
    *text = c;
    text++;
    *text = '\0';
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
printHex08(char*& text, UInt08 value)
{
    UInt08 h = value>>4;
    if (h < 10) printChar(text, char('0'    + h));
    else        printChar(text, char('A'-10 + h));
    UInt08 l = value&0xF;
    if (l < 10) printChar(text, char('0'    + l));
    else        printChar(text, char('A'-10 + l));
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
parseHexChar(const char*& text, UInt08& value)
{
    value = 0;
    int c = int(*text);
    int n;
    if      (c >= '0' && c <= '9') n =  0 + c - '0';
    else if (c >= 'a' && c <= 'f') n = 10 + c - 'a';
    else if (c >= 'A' && c <= 'F') n = 10 + c - 'A';
    else                           return false;
    value = UInt08(n);
    text++;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
parseHex08(const char*& text, UInt08& value)
{
    const char* tmpText = text;
    value = 0;
    UInt08 nibble[2];
    if (!parseHexChar(tmpText, nibble[0])) return false;
    if (!parseHexChar(tmpText, nibble[1])) return false;
    value = UInt08((nibble[0] << 4) + nibble[1]);
    text = tmpText;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static string
printRequestPdu(const Cy4Message& message)
{
    if (!message.isValid()) return "";
    bool payload   = message.access() == Cy4Message::Write;
    bool isRequest = true;
    UInt08 net = 0xff;
    UInt08 dev = 0xff;
    Cy4Address address = message.address();
    if (address.isValid())
    {   net = UInt08(address.net());
        dev = UInt08(address.dev());
    }
    int location;
    if (message.location().isValid()) location = message.location().toInteger();
    else                              location = 0xFFFFFF;
    int count = message.count();
    if (count < 0 || count > CY4_PAYLOAD_CAPACITY) count = 0;
    UInt08  header[7];
    header[0] = UInt08(0xC0 + (isRequest<<1) + (payload<<0));
    header[1] = net;
    header[2] = dev;
    header[3] = UInt08(location >> 16);
    header[4] = UInt08(location >>  8);
    header[5] = UInt08(location >>  0);
    header[6] = UInt08(count);
    char buffer[(8+CY4_PAYLOAD_CAPACITY)*2+1];  // PDU-Bytes in ASCII-Hex plus Nullbyte
    char* text = buffer;
    UInt08 crc = 0xFF;
    for (int i=0; i<7; i++)
    {   printHex08(text, header[i]);
        updateCRC(header[i], crc);
    }
    if (payload)
    {   for (int i=0; i<header[6]; i++)
        {   printHex08(text, message[i]);
            updateCRC(message[i], crc);
        }
    }
    printHex08(text, crc);
    return buffer;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Message
parseResponsePdu(const string& textLine)
{
    const char* text = textLine.c_str();
    while (isspace(*text)) text++;
    if (text[0] == '\0') return Cy4Message();
    UInt08 crc = 0xFF;
    UInt08 buffer[7];
    for (int i=0; i<7; i++)
    {   if (!parseHex08(text, buffer[i])) return Cy4Message();
        updateCRC(buffer[i], crc);
    }
    UInt08 dtf = buffer[0];
    if (dtf < 0xC0 || dtf > 0xC5) return Cy4Message(); // Unmöglicher DTF-Wert
    if (bool(dtf & 0x02)) return Cy4Message();         // 'D' = 1 → Antwort ist Anfrage
    if (bool(dtf & 0x04)) return Cy4Message();         // 'F' = 1 → Fehler
    bool hasPayload = bool(dtf & 0x01);                // 'P' = 1 → Nutzlast enthalten
    Cy4Address   address  = Cy4Address(buffer[1], buffer[2]);
    Cy4Location  location = Cy4Location((int(buffer[3])<<16) + (int(buffer[4]<<8) + buffer[5]));
    int          count    = int(buffer[6]);
    if (count < 1 || count > CY4_PAYLOAD_CAPACITY) return Cy4Message(); // Unmögliche Anzahl
    vector<UInt08> payload(USize(count), 0);
    if (hasPayload)
    {   for (int i=0; i<count; i++)
        {   UInt08 data;
            if (!parseHex08(text, data)) return Cy4Message();
            payload[USize(i)] = data;
            updateCRC(data, crc);
        }
    }
    UInt08 data;
    if (!parseHex08(text, data)) return Cy4Message();
    if (crc != data) return Cy4Message();               // CRC-Fehler
    while (isspace(*text)) text++;
    if (*text != '\0') return Cy4Message();
    Cy4Message response;
    if (hasPayload)
    {   // Leseantwort
        response = Cy4Message(address, location, count);
        for (int i=0; i<count; i++) response[i] = payload[USize(i)];
    }
    else
    {   // Schreibbestätigung
        response = Cy4Message(address, location, payload);
    }
    return response;
}

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef CY4_MESSAGE_MONITOR
static string
printMessage(const Cy4Message& message, bool isRequest)
{
    string text = message.address().toText();
    switch (message.access())
    {   case Cy4Message::Read:  text += " Read ";  break;
        case Cy4Message::Write: text += " Write "; break;
        default:                return " ???";
    }
    text += message.location().toText() + "h";
    if (message.size() < 10) text += ":0";
    else                      text += ":";
    text += to_string(message.size());
    if (isRequest)
    {   text += " request";
    }
    else
    {   if (message.isValid())
        {   if (message.access() == Cy4Message::Read) text += " response";
            else                                      text += " acknowledged";
        }
        else
        {   text += " failed";
        }
    }
    if (message.isValid())
    {   if ((isRequest  && message.access() == Cy4Message::Write) ||
            (!isRequest && message.access() == Cy4Message::Read))
        {   // Bei Schreibanfrage oder Leseantworten Nutzlast ausgeben
            char buffer[1+1+(64*3-1)+1 +1];   // " [xx xx  ...  xx]"
            char* position = buffer;
            printChar(position, ' ');
            printChar(position, '[');
            bool shortPayload = false;
            #ifdef CY4_MESSAGE_SHORT_PAYLOAD
            shortPayload = true;
            #endif
            if (shortPayload && message.size() > 6)
            {   // [A0 03 17 .. B0 01 05]
                printHex08(position, message[0]);
                printChar(position, ' ');
                printHex08(position, message[1]);
                printChar(position, ' ');
                printHex08(position, message[2]);
                printChar(position, ' ');
                printChar(position, '.');
                printChar(position, '.');
                printChar(position, ' ');
                printHex08(position, message[message.size()-3]);
                printChar(position, ' ');
                printHex08(position, message[message.size()-2]);
                printChar(position, ' ');
                printHex08(position, message[message.size()-1]);
            }
            else
            {   for (int i=0; i<message.size(); i++)
                {   if (i != 0) printChar(position, ' ');
                    printHex08(position, message[i]);
                }
            }
            printChar(position, ']');
            text += buffer;
        }
    }
    return text;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Message&
asteriskRequestMarkerMessage(void)
{
    static Cy4Message dummy;
    return dummy;
}

//////////////////////////////////////////////////////////////////////////////////////////
static void*
asteriskRequestMarkerContext(void)
{
    static void* dummy = nullptr;
    return dummy;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Message&
postPauseMarkerMessage(void)
{
    static Cy4Message dummy;
    return dummy;
}

//////////////////////////////////////////////////////////////////////////////////////////
static void*
postPauseMarkerContext(void)
{
    static void* dummy = nullptr;
    return dummy;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::exitTransceiveEventLoopHandle(const Cy4Message& response, void* context)
{
    Cy4Net* cy4Net = static_cast<Cy4Net*>(context);
    emit cy4Net->transceiveCompleted(response);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::onTextReceived(void)
{
    if (mIoDevice == nullptr || mPostingQueue.empty()) return;
    Cy4Message responseMessage;
    bool utilizeResponse = false;
    QObject* source = sender();
    if (source == mIoDevice || source == &mResponseTimer)
    {   // Antworttext lesen
        if (mIoDevice->canReadLine())
        {   mResponseTimer.stop();
            // Textzeile vollständig
            QByteArray text = mIoDevice->readAll();
            responseMessage = parseResponsePdu(text.data());
            #ifdef CY4_MESSAGE_MONITOR
            {   cout << "Cy4NET: ";
                if (responseMessage.isValid())
                {   cout << printMessage(responseMessage, false);
                }
                else
                {   // Fehler, ungültige Anfrage ausgeben
                    Cy4Message tmpMessage = mPostingQueue.front().message;
                    tmpMessage.invalidate();
                    cout << printMessage(tmpMessage, false);
                }
                cout  << endl;
            }
            #endif
            utilizeResponse = true;
        }
        if (source == &mResponseTimer && !utilizeResponse)
        {   // Zeitüberschreitung, Antwort vom Gateway ist ausgeblieben
            mIoDevice->readAll();
            #ifdef CY4_MESSAGE_MONITOR
            cout << "Cy4NET: Response from gateway timed out" << endl;
            #endif
            utilizeResponse = true;
            mGatewayAddress = Cy4Address();
        }
    }
    else if (source == &mPauseTimer)
    {   // Übertragungspause abgelaufen, direkt nächste Anfrage senden
        mPostingQueue.pop_front();
        if (!mPostingQueue.empty()) sendNextPosting();
    }
    else
    {   // Unbekannter Sender
        return;
    }
    if (utilizeResponse)
    {   const Posting&  currentPosting = mPostingQueue.front();
        Cy4Message      requestMessage = currentPosting.message;
        ResponseHandler handler        = currentPosting.callback;
        void*           context        = currentPosting.context;
        mPostingQueue.pop_front();
        if (context == reinterpret_cast<void*>(asteriskRequestMarkerContext))
        {   // Asterisk-Anfrage, 'TypePlate'-Anfrage nachträglich zusammensetzen
            requestMessage = Cy4Message(responseMessage.address(),
                                        responseMessage.location(),
                                        responseMessage.count());
            context = this;
        }
        if (!requestMessage.isValid()  || !responseMessage.isValid() ||
            requestMessage.address()   != responseMessage.address()  ||
            requestMessage.access()    != responseMessage.access()   ||
            requestMessage.address()   != responseMessage.address()  ||
            requestMessage.location()  != responseMessage.location() ||
            requestMessage.count()     != responseMessage.count())
        {   // Antwort passt nicht zur Anfrage, ungültige Anfrage als Antwort übergeben
            responseMessage = requestMessage;
            responseMessage.invalidate();
        }
        if (!mPostingQueue.empty()) sendNextPosting();
        if (handler != nullptr) handler(responseMessage, context);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::sendNextPosting(void)
{
    mIoDevice->readAll();
    const Posting& posting = mPostingQueue.front();
    string textLine;
    if (posting.context == reinterpret_cast<void*>(asteriskRequestMarkerContext))
    {   // Asterisk-Anfrage schicken
        textLine = "*";
        #ifdef CY4_MESSAGE_MONITOR
            cout << "Cy4NET: Call gateway" << endl;
        #endif
    }
    else if (posting.context == reinterpret_cast<void*>(postPauseMarkerContext))
    {   // Übertragungspause einleiten
        mPauseTimer.start(PAUSE_TIME);
        #ifdef CY4_MESSAGE_MONITOR
           cout << "Cy4NET: Pause" << endl;
        #endif
    }
    else
    {   // Normale Anfrage absenden
        textLine = printRequestPdu(posting.message);
        #ifdef CY4_MESSAGE_MONITOR
            cout << "Cy4NET: " << printMessage(posting.message, true) << endl;
        #endif
    }
    if (!textLine.empty())
    {   mIoDevice->write(textLine.c_str());
        mIoDevice->write("\n");
        mResponseTimer.start();
    }
    emit activity();
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Net::Cy4Net(QObject* parent) : QObject(parent)
{
   mIoDevice = nullptr;
   mResponseTimer.setSingleShot(true);
   mResponseTimer.setInterval(GATEWAY_RESPONSE_TIMEOUT);
   connect(&mResponseTimer, &QTimer::timeout, this, &Cy4Net::onTextReceived, Qt::QueuedConnection);
   mPauseTimer.setSingleShot(true);
   connect(&mPauseTimer, &QTimer::timeout, this, &Cy4Net::onTextReceived, Qt::QueuedConnection);
   mIsTransceiving = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Net::~Cy4Net()
{
    disconnect();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  open(): Öffnet die Schnittstelle zum Gateway. Eine bereits bestehende Verbindung    //
//  ------  wird gegebenenfalls zuvor geschlossen. Die Schnittstelle kann eine serielle //
//  Verbindung oder eine TCP-Verbindung mit IP-Adresse und Portnummer sein.             //
//  Ob an der geöffneten Schnittstelle ein Gateway antwortet, kann mittels callGate-    //
//  way() erfragt werden. Zuvor muss jedoch sichergestellt sein, dass die Ereignisver-  //
//  arbeitung läuft.                                                                    //
//                                                                                      //
//  Eingabe: · ip:     IP der TCP-Sockets als Zeichenkette, Beispiel "192.168.168.29".  //
//           · port:   Portnummer des TCP-Sockets: 0 .. 65535.                          //
//           · serial: Name der seriellen Schnittstelle, Beispiel: "COM2".              //
//  Ausgabe: 'true', wenn die Schnittstelle geöffnet werden konnte.                     //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::open(const QString& ip, int port)
{
    close();
    mIoDevice = new QTcpSocket();
    QTcpSocket* socket = static_cast<QTcpSocket*>(mIoDevice);
    socket->connectToHost(ip, quint16(port));
    socket->waitForConnected(5000);
    if (static_cast<QTcpSocket*>(mIoDevice)->state() != QAbstractSocket::ConnectedState)
    {   close();
        return false;
    }
    connect(mIoDevice, &QIODevice::readyRead, this, &Cy4Net::onTextReceived, Qt::QueuedConnection);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::open(const QString& serial)
{
    close();
    mIoDevice = new QSerialPort();
    QSerialPort* port = static_cast<QSerialPort*>(mIoDevice);
    port->setPortName(serial);
    port->setBaudRate(QSerialPort::Baud57600);
    port->setFlowControl(QSerialPort::NoFlowControl);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    if (port->open(QIODevice::ReadWrite))
    {   port->setDataTerminalReady(true);
        port->waitForReadyRead(100);
    }
    if (!mIoDevice->isOpen())
    {   close();
        return false;
    }
    mIoDevice->write("\n");
    mIoDevice->waitForBytesWritten(10);
    connect(mIoDevice, &QIODevice::readyRead, this, &Cy4Net::onTextReceived, Qt::QueuedConnection);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  close(): Schließt die Verbindung zur Schnittstelle. Damit ist das Gateway nicht     //
//  -------  mehr erreichbar, d.h. die von gateway() gelieferte Adresse ist ungültig.   //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::close(void)
{
    if (mIoDevice != nullptr)
    {   mGatewayAddress = Cy4Address();
        disconnect(mIoDevice, nullptr, this, nullptr);
        QSerialPort* serialPort = qobject_cast<QSerialPort*>(mIoDevice);
        QTcpSocket*  tcpSocket  = qobject_cast<QTcpSocket*>(mIoDevice);
        if (serialPort != nullptr)
        {   serialPort->close();
        }
        if (tcpSocket != nullptr)
        {   if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
            {   tcpSocket->disconnectFromHost();
                if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
                {   tcpSocket->waitForDisconnected(2000);
                }
                if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
                {   tcpSocket->abort();
                }
            }
        }
        delete mIoDevice;
        mIoDevice = nullptr;
    }
    mResponseTimer.stop();
    mPauseTimer.stop();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  isOpen(): Prüft, ob die Verbindung zur Schnittstelle hergestellt ist.               //
//  --------                                                                            //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: 'true', wenn die Verbindung zur Schnittstelle besteht.                     //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::isOpen(void) const
{
    return mIoDevice != nullptr && mIoDevice->isOpen();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  callGateway(): Ruft das Gateway. Die Anfrage ist nur erfolgreich, wenn neben der    //
//  -------------  geöffneten Schnittstelle auch die Ereignisverarbeitung läuft.        //
//  Die Adresse des Gateways kann anschließend mittels gateway() erfragt werden.        //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: 'true', wenn ein Gateway geantwortet hat.                                  //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::callGateway(void)
{
    Cy4Message response = transceive(asteriskRequestMarkerMessage());
    if (response.isValid()) mGatewayAddress = response.address();
    else                    mGatewayAddress = Cy4Address();
    return mGatewayAddress.isValid();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  gateway(): Liefert die Adresse eines zuvor mittels callGateway() angefragten Gate-  //
//  ---------  ways. Gab es bislang keine Anfrage ans Gateway oder hat an der Schnitt-  //
//  stelle kein Gateway geantwortet, ist die zurückgelieferte Adresse ungültig.         //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: Adresse des Gateways oder ungültige Adresse.                               //
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Address
Cy4Net::gateway(void) const
{
    return mGatewayAddress;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  transceive(): Übergibt die Anfrage in <request> zur Versendung an die Übertragungs- //
//  ------------  einheit, blockiert den aktuellen Thread in einer lokalen Ereignis-    //
//  schleife und kehrt mit der erhaltenen Antwort zurück. Damit ist transceive() die    //
//  synchrone Variante von post(). Die Funktion ist eintrittsinvariant, d.h., bei einer //
//  bereits laufenden Übertragung darf keine weitere Anfrage gestartet werden (s.a.     //
//  isBusy()).                                                                          //
//  Bleibt die Anfrage unbeantwortet, ist die Antwort eine für ungültig erklärte Kopie  //
//  der Anfrage.                                                                        //
//                                                                                      //
//  Eingabe: · request: Anfragenachricht.                                               //
//  Ausgabe: Antwort auf <request> oder ungültige Kopie von <request>.                  //
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Message
Cy4Net::transceive(const Cy4Message& request)
{
    if (mIoDevice == nullptr) return Cy4Message();
    Cy4Message tmpResponse(request);
    tmpResponse.invalidate();   // Für den Fall, dass die Übertragung abgebrochen wird
    if (mIsTransceiving)
    {   // Neueintritt während einer laufenden Übertragung ist nicht nicht erlaubt
        return tmpResponse;
    }
    QEventLoop eventLoop;
    auto connection = connect(this,
                              &Cy4Net::transceiveCompleted,
                              &eventLoop,
                              [&](const Cy4Message& response)
                              {   if (response.isValid())
                                  {    tmpResponse = response;
                                  }
                                  eventLoop.quit();
                              });
    mIsTransceiving = true;
    post(request, exitTransceiveEventLoopHandle, this);
    eventLoop.exec();
    disconnect(connection);
    mIsTransceiving = false;
    assert((&request == &asteriskRequestMarkerMessage()  ||
            &request == &postPauseMarkerMessage())       ||
           (tmpResponse.address()  == request.address()  &&
            tmpResponse.access()   == request.access()   &&
            tmpResponse.address()  == request.address()  &&
            tmpResponse.location() == request.location() &&
            tmpResponse.count()    == request.count()));
    return tmpResponse;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  post(): Übergibt die Anfrage in <request> zur Versendung an die Übertragungsein-    //
//  ------  heit und kehrt danach unmittelbar zum Aufrufer zurück. Ist die Über-        //
//  tragungseinheit noch mit der Abarbeitung einer vorausgegangenen Anfrage beschäf-    //
//  tigt, wird <request> in die Schlange abzuarbeitender Anfragen eingereiht.           //
//  Optional kann in <callback> ein Antwortbehandler angegeben werden, der nach Ab-     //
//  schluss der Anfrage aufgerufen wird:                                                //
//                                                                                      //
//              void  (*ResponseHandler)(const Cy4Message&, void*);                     //
//                                                                                      //
//  <ResponseHandler> erhält als Parameter die Referenz auf die Antwortnachricht sowie  //
//  <context>. Der Parameter <context> dient üblicherweise dazu, in einem objektorien-  //
//  tierten Umfeld die Antwortnachricht an eine konkrete Bezugsobjekt-Instanz als       //
//  Rückruf weiterzuleiten.                                                             //
//                                                                                      //
//  Eingabe: · request:  Die abzusetzende Anfrage.                                      //
//           · callback: Der nach Abschluss der Anfrage aufzurufende Antwortbehandler   //
//                       (optional, voreingestellt ist 'nullptr').                      //
//           · context:  Der dem Antwortbehandler mitgegebene Parameter (optional,      //
//                       voreingestellt ist 'nullptr').                                 //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::post(const Cy4Message& request, ResponseHandler callback, void* context)
{
    if (mIoDevice == nullptr) return;
    Posting posting;
    if (&request == &asteriskRequestMarkerMessage())
    {   // Asterisk-Anfrage in die Sendeschlange stellen
        posting.message  = Cy4Message();
        posting.callback = callback;
        posting.context   = reinterpret_cast<void*>(asteriskRequestMarkerContext);
    }
    else if (&request == &postPauseMarkerMessage())
    {   // Übertragungspause in die Sendeschlange stellen
        posting.message  = Cy4Message();
        posting.callback = callback;
        posting.context   = reinterpret_cast<void*>(postPauseMarkerContext);
    }
    else
    {   // Normale Anfrage in die Sendeschlange stellen
        posting.message  = request;
        posting.callback = callback;
        posting.context   = context;
    }
    // Anfrage hinten anstellen und wenn es die einzige ist, unmittelbar absenden
    mPostingQueue.push_back(posting);
    if (mPostingQueue.size() == 1) sendNextPosting();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  fetchTypePlate(): Erfragt das Typenschild.                                          //
//  ----------------  Die Funktion existiert in einer synchronen und einer asynchronen  //
//  Variante.                                                                           //
//                                                                                      //
//  Eingabe: · address:  Adresse des anzufragenden Gerätes.                             //
//           · callback: Der nach Abschluss der Anfrage aufzurufende Antwortbehandler   //
//                       oder 'nullptr'.                                                //
//           · context:  Der dem Antwortbehandler mitgegebene Parameter (optional,      //
//                       voreingestellt ist 'nullptr').                                 //
//  Ausgabe: [Synchrone Variante]: -                                                    //
//           [Asynchrone Variante]: Das Typenschild. War die Anfrage nicht erfolgreich, //
//           ist das zurückgelieferte Typenschild ungültig.                             //
//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypePlate
Cy4Net::fetchTypePlate(Cy4Address address)
{
    Cy4Message request  = Cy4Message(address, Cy4Location(0xFFFFF0), 7);
    Cy4Message response = transceive(request);
    return Cy4TypePlate(response);
}

void
Cy4Net::fetchTypePlate(Cy4Address address, ResponseHandler callback, void* context)
{
    Cy4Message request = Cy4Message(address, Cy4Location(0xFFFFF0), 7);
    post(request, callback, context);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  enterApplication(): Wechselt in die Applikation (oder startet sie neu).             //
//  ------------------  Die Funktion existiert in einer synchronen und einer asyn-      //
//  chronen Variante.                                                                   //
//                                                                                      //
//  Eingabe: · address:  Adresse des betroffenen Gerätes.                               //
//           · callback: Der nach Abschluss der Anfrage aufzurufende Antwortbehandler   //
//                       oder 'nullptr'.                                                //
//           · context:  Der dem Antwortbehandler mitgegebene Parameter (optional,      //
//                       voreingestellt ist 'nullptr').                                 //
//  Ausgabe: [Synchrone Variante]: -                                                    //
//           [Asynchrone Variante]: 'true' wenn der Start/Neustart vom Gerät angenommen //
//           wurde.                                                                     //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::enterApplication(Cy4Address address)
{
    vector<UInt08> data(1, 0x5E);
    Cy4Message request  = Cy4Message(address, Cy4Location(0xFFFFF8), data);
    Cy4Message response = transceive(request);
    if (!response.isValid()) return false;
    postPause();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::enterApplication(Cy4Address address, ResponseHandler callback, void* context)
{
    vector<UInt08> data(1, 0x5E);
    Cy4Message request = Cy4Message(address, Cy4Location(0xFFFFF8), data);
    post(request, callback, context);
    postPause();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  enterProgramLoader(): Wechselt in den Programmlader (oder startet ihn neu).         //
//  --------------------  Die Funktion existiert in einer synchronen und einer asyn-    //
//  chronen Variante.                                                                   //
//                                                                                      //
//  Eingabe: · address:  Adresse des betroffenen Gerätes.                               //
//           · callback: Der nach Abschluss der Anfrage aufzurufende Antwortbehandler   //
//                       oder 'nullptr'.                                                //
//           · context:  Der dem Antwortbehandler mitgegebene Parameter (optional,      //
//                       voreingestellt ist 'nullptr').                                 //
//  Ausgabe: [Synchrone Variante]: -                                                    //
//           [Asynchrone Variante]: 'true' wenn der Start/Neustart vom Gerät angenommen //
//           wurde.                                                                     //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::enterProgramLoader(Cy4Address address)
{
    vector<UInt08> data(1, 0x5E);
    Cy4Message request  = Cy4Message(address, Cy4Location(0xFFFFFA), data);
    Cy4Message response = transceive(request);
    if (!response.isValid()) return false;
    postPause();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::enterProgramLoader(Cy4Address address, ResponseHandler callback, void* context)
{
    vector<UInt08> data(1, 0x5E);
    Cy4Message request = Cy4Message(address, Cy4Location(0xFFFFFA), data);
    post(request, callback, context);
    postPause();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  changeCy4Address(): Wechselt die Adresse eines Gerätes von <currentAddress> zu      //
//  ------------------  <newAddress>. Die Funktion prüft nicht, ob die neue Adresse     //
//  durch ein anderen Gerät belegt ist. Ist <newAddress> ungültig oder eine Rundruf-    //
//  adresse, wird das betroffene Gerät den Adresswechsel verweigern.                    //
//  Die Funktion existiert in einer synchronen und einer asynchronen Variante.          //
//                                                                                      //
//  Eingabe: · currentAddress: Bestehende Adresse des Gerätes.                          //
//           · newAddress:     Neue Adresse des Gerätes.                                //
//           · callback:       Der nach Abschluss der Anfrage aufzurufende Antwortbe-   //
//                             handler oder 'nullptr'.                                  //
//           · context:        Der dem Antwortbehandler mitgegebene Parameter (opt-     //
//                             ional, voreingestellt ist 'nullptr').                    //
//  Ausgabe: [Synchrone Variante]: -                                                    //
//           [Asynchrone Variante]: 'true' wenn der Wechsel stattgefunden hat.          //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::changeCy4Address(Cy4Address currentAddress, Cy4Address newAddress)
{
    vector<UInt08> data(2);
    data[0] = UInt08(newAddress.net());
    data[1] = UInt08(newAddress.dev());
    Cy4Message request  = Cy4Message(currentAddress, Cy4Location(0xFFFFFC), data);
    Cy4Message response = transceive(request);
    return response.isValid();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::changeCy4Address(Cy4Address currentAddress, Cy4Address newAddress, ResponseHandler callback, void* context)
{
    vector<UInt08> data(2);
    data[0] = UInt08(newAddress.net());
    data[1] = UInt08(newAddress.dev());
    Cy4Message request  = Cy4Message(currentAddress, Cy4Location(0xFFFFFC), data);
    post(request, callback, context);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  queryLabel(): Erfragt eine UTF-8-Zeichenkette an FFFF00h:61:RW. Dieser Dienst, der  //
//  ------------  als Gerätebezeichnung bekannt ist, ist zwar kein Pflichtdienst, wird  //
//  aber dennoch von den meisten Geräten unterstützt. Programmlader verfügen üblicher-  //
//  weise nicht über eine Gerätebezeichnung.                                            //
//  Die Funktion existiert in einer synchronen und einer asynchronen Variante.          //
//                                                                                      //
//  Eingabe: · address:  Adresse des betroffenen Gerätes.                               //
//           · callback: Der nach Abschluss der Anfrage aufzurufende Antwortbehandler   //
//                       oder 'nullptr'.                                                //
//           · context:  Der dem Antwortbehandler mitgegebene Parameter (optional,      //
//                       voreingestellt ist 'nullptr').                                 //
//  Ausgabe: [Synchrone Variante]: -                                                    //
//           [Asynchrone Variante]: Die Gerätebezeichnung oder eine Leere Zeichenkette. //
//////////////////////////////////////////////////////////////////////////////////////////
QString
Cy4Net::queryLabel(Cy4Address address)
{
    Cy4Message request  = Cy4Message(address, Cy4Location(0xFFFF00), 61);
    Cy4Message response = transceive(request);
    response[60] = '\0';   // Nullbyte-Terminierung sicherstellen
    QString label;
    if (response.isValid())
    {   label = QString::fromUtf8(reinterpret_cast<const char*>(response.data().data()));
    }
    return label;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::queryLabel(Cy4Address address, ResponseHandler callback, void* context)
{
    Cy4Message request = Cy4Message(address, Cy4Location(0xFFFF00), 61);
    post(request, callback, context);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  postPause(): Stellt in die Schlange abzuarbeitender Anfragen eine Pausen-Anfrage    //
//  -----------  ein. Diese spezielle Art der Anfrage wird weder tatsächlich gesendet   //
//  noch beantwortet sondern sie verzögert, wenn sie an der Reihe ist, das Aussenden    //
//  nachfolgender Anfragen um ca. eine Sekunde. Das kann sinnvoll sein, wenn einem      //
//  Geräte für die Verarbeitung einer Nachricht etwas Zeit eingeräumt werden soll,      //
//  beispielsweise nach einem Neustart oder dem Zurücksetzen auf Werkseinstellungen.    //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::postPause(void)
{
    post(postPauseMarkerMessage());
}

//////////////////////////////////////////////////////////////////////////////////////////
//  isBusy(): Siehe unten.                                                              //
//  --------                                                                            //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: 'true', solange Überrtagungen stattfinden.                                 //
//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Net::isBusy(void) const
{
    return mIsTransceiving || !mPostingQueue.empty();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  dropPostings(): Löscht ausstehende Anfragen. Eventuell mitgegebene Antwortbehandler //
//  --------------  werden nicht mehr aufgerufen. Das betrifft auch den Antwort-        //
//  behandler einer laufenden Anfrage.                                                  //
//                                                                                      //
//  Eingabe: -                                                                          //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Net::dropPostings(void)
{
    mPostingQueue.clear();
    emit transceiveCompleted(Cy4Message());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Cy4Net Singelton                                                                     //
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Net&
cy4Net(void)
{
    static Cy4Net instance;
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
