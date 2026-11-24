//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4NET Firmware                                                                     //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <vector>
#include <fstream>
#include "Global.h"
#include "Cy4Firmware.h"

using namespace std;

static const char Cy4CodeMark[]      = "Cy4Code";
static const int  MemSegmentCapacity = 0xFFFF;
static const int  TextLineCapacity   = 1+8+2*MemSegmentCapacity+2+1;

//////////////////////////////////////////////////////////////////////////////////////////
// Speichersegment ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class MemSegment
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        int                mAddress;
        UInt08*            mBlock;
        int                mSize;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                           MemSegment(void);
                           MemSegment(const MemSegment&);
                           MemSegment(int, int, UInt08*);
                          ~MemSegment(void);
        const MemSegment&  operator=(const MemSegment& src);
        bool               operator>(const MemSegment& src) const;
        bool               operator<(const MemSegment& src) const;
        bool               isEmpty(void) const;
        void               setEmpty(void);
        int                address(void) const;
        int                size(void) const;
        const UInt08*      data(void) const;
        UInt08*            data(void);
        UInt08             operator[](int) const;
        UInt08&            operator[](int);
        Cy4Message         toMessage(const Cy4Address&) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Speichersegment  /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
MemSegment::MemSegment(void)
{
    mBlock = nullptr;
    setEmpty();
}

//////////////////////////////////////////////////////////////////////////////////////////
MemSegment::MemSegment(const MemSegment& src)
{
    mBlock = nullptr;
    setEmpty();
    operator=(src);
}

//////////////////////////////////////////////////////////////////////////////////////////
MemSegment::MemSegment(int byteAddress, int segmentSize, UInt08* segmentData)
{
    mBlock = nullptr;
    setEmpty();
    mAddress = byteAddress;
    if (segmentSize < 0) mSize = 0;
    else                 mSize = segmentSize;
    if (mSize > 0) mBlock = new UInt08[mSize];
    if (segmentData != nullptr)
    {   for (int i=0; i<mSize; i++) mBlock[i] = segmentData[i];
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
MemSegment::~MemSegment(void)
{
    setEmpty();
}

//////////////////////////////////////////////////////////////////////////////////////////
const MemSegment&
MemSegment::operator=(const MemSegment& other)
{
    if (&other != this)
    {   setEmpty();
        mAddress = other.mAddress;
        mBlock = other.mBlock;
        mSize = other.mSize;
        if (mBlock != nullptr)
        {   mBlock = new UInt08[mSize];
            for (int i=0; i<mSize; i++) mBlock[i] = other.mBlock[i];
        }
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
MemSegment::operator>(const MemSegment& other) const
{
    return mAddress > other.mAddress;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
MemSegment::operator<(const MemSegment& other) const
{
    return mAddress < other.mAddress;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
MemSegment::isEmpty(void) const
{
    return mSize == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MemSegment::setEmpty(void)
{
    delete[] mBlock;
    mAddress = 0;
    mBlock = nullptr;
    mSize = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
MemSegment::address(void) const
{
    return mAddress;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
MemSegment::size(void) const
{
    return mSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
const UInt08*
MemSegment::data(void) const
{
    return mBlock;
}

//////////////////////////////////////////////////////////////////////////////////////////
UInt08*
MemSegment::data(void)
{
    return const_cast<UInt08*>(static_cast<const MemSegment&>(*this).data());
}

//////////////////////////////////////////////////////////////////////////////////////////
UInt08
MemSegment::operator[](int index) const
{
    return const_cast<MemSegment&>(*this).operator[](index);
}

//////////////////////////////////////////////////////////////////////////////////////////
UInt08&
MemSegment::operator[](int index)
{
    assert(index >= 0 && index < mSize);
    return mBlock[index];
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Message
MemSegment::toMessage(const Cy4Address& address) const
{
    if( isEmpty() ) return Cy4Message();
    std::vector<UInt08> byteBlock(static_cast<USize>(mSize));
    for( USize i=0; i<byteBlock.size(); i++ ) byteBlock[i] = mBlock[i];
    return Cy4Message(address, Cy4Location(mAddress), byteBlock);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Speicherblock  ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class MemBlock
{
    public:
        UInt32          base;

    private:
        vector<UInt16>  memory;

    public:
                MemBlock(void)              { base = 0;                            }
        void    clear(void)                 { base = 0; memory.clear();            }
        int     size(void) const            { return int(memory.size());           }
        void    resize(int size)            { memory.resize(USize(size));          }
        UInt08  operator[](int index) const { return UInt08(memory[USize(index)]); }
        void    set(int index, UInt08 byte) { memory[USize(index)] = byte;         }
        void    setUnused(int index)        { memory[USize(index)] = 0xFFFF;       }
        bool    isUsed(int index) const     { return !(memory[USize(index)]>>8);   }
};

//////////////////////////////////////////////////////////////////////////////////////////
//  parseHexRecord(): Analysiert den in <hexLine> übergebenen Hex-Datensatz. Ist die    //
//  ----------------  Funktion erfolgreich, wird das extrahierte Speichersegment in     //
//  <hexRecord> abgelegt.                                                               //
//  Hinweis: Die Startadresse des Byteblocks basiert auf der aktuell eingestellten Seg- //
//           ment-Basisadresse. Das Verstellen bzw. Setzen der Segment-Basisadresse ist //
//           mit Hex-Datensätzen des Types 2 und 4 möglich. So kann z.B. durch den Auf- //
//           ruf von parseHexRecord(":020000040000FA", dummy) die Segment-Basisadresse  //
//           auf 0 (Voreinstellung) gesetzt werden.                                     //
//                                                                                      //
//  Eingabe: · hexLine:    Die auszulesende Textzeile.                                  //
//           · memSegemnt: Referenz auf einen MemSegemnt, das im Erfolgsfall das extra- //
//                         hierte Speichersegment aufnimmt.                             //
//  Ausgabe: 'true', wenn in <hexline> ein gültiger Hex-Datensatz steht.                //
//           Hinweis: Auch eine Hex-Kontrollsequenz wie z.B. die Ende-Kennung ist ein   //
//                    korrekter Hex-Datensatz. Entsprechend ist der Rückgabewert        //
//                    'true', <hexRecord> ist in dem Fall jedoch leer.                  //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
parseHexRecord(const char* hexLine, MemSegment& memSegemnt)
{                    //                         +-+--+----+--+-- ·· --+--+···
                     // Record State machine:   |:|03|0030|00|02 33 7A|1E|  :
    typedef enum     //                         +-+--+----+--+-- ·· --+--+···
    {   LENGTH,      // <--------------------------+  | |  |    \ /    |   |
        ADDRESS_HI,  // <-----------------------------+ |  |     |     |   |
        ADDRESS_LO,  // <-------------------------------+  |     |     |   |
        TYPE,        // <----------------------------------+     |     |   |
        DATA,        // <----------------------------------------+     |   |
        CHECKSUM,    // <----------------------------------------------+   |
        END_OF_REC   // <--------------------------------------------------+
    } State;

    static UInt32  baseAdress = 0x00000000;
    UInt32         recMemAdr;
    UInt08         recMemBlk[255];
    UInt08         recMemLen = 0;
    State          state;

    memSegemnt.setEmpty();
    recMemAdr = baseAdress;
    state     = LENGTH;
    recMemLen = 0;
    while (*hexLine == ' ') hexLine++;
    if (*hexLine == '\0') return true; // (Leerzeile)
    if (*hexLine != ':' ) return false;
    hexLine++;
    UInt08 type   = 0;
    UInt08 i      = 0;
    UInt08 chksum = 0;
    do
    {   UInt08 nibble;
        {   // High-Nibble lesen
            char hexDigit = *hexLine;
            if (hexDigit >= 'a') hexDigit -= 'a'-'A';
            if      (hexDigit >= '0' && hexDigit <= '9') nibble =  0 + UInt08(hexDigit) - '0';
            else if (hexDigit >= 'A' && hexDigit <= 'F') nibble = 10 + UInt08(hexDigit) - 'A';
            else                                         return false;
        }
        UInt08 byte = UInt08(nibble << 4);
        hexLine++;
        if (*hexLine == '\0') return false;
        {   // Low-Nibble lesen
            char hexDigit = *hexLine;
            if (hexDigit >= 'a') hexDigit -= 'a'-'A';
            if      (hexDigit >= '0' && hexDigit <= '9') nibble =  0 + UInt08(hexDigit) - '0';
            else if (hexDigit >= 'A' && hexDigit <= 'F') nibble = 10 + UInt08(hexDigit) - 'A';
            else                                         return false;
        }
        byte |= nibble;
        hexLine++;
        chksum -= byte;
        switch (state)
        {   case LENGTH:
                recMemLen = byte;
                state = ADDRESS_HI;
                break;
            case ADDRESS_HI:
                recMemAdr = UInt32(UInt16(byte) << 8);
                state = ADDRESS_LO;
                break;
            case ADDRESS_LO:
                recMemAdr |= UInt32(byte);
                recMemAdr += baseAdress;
                state = TYPE;
                break;
            case TYPE:
                type = byte;
                if (type > 5)  return false;
                if (recMemLen > 0) state = DATA;
                else               state = CHECKSUM;
                break;
            case DATA:
                recMemBlk[i] = byte;
                i++;
                if (i == recMemLen) state = CHECKSUM;
                break;
            case CHECKSUM:
                if (chksum != 0) return false;
                state = END_OF_REC;
                break;
            default:
                return false;
        }
        while (*hexLine == ' ') hexLine++;
    } while (*hexLine != '\0');
    if (state != END_OF_REC) return false;
    if (type != 0)
    {   if (recMemAdr != baseAdress) return false;
        if (type == 1)
        {   // End of file record
            return recMemLen == 0;
        }
        else if (type == 2 || type == 4)
        {   // Extended segment/linear address record
            if (recMemLen != 2) return false;
            baseAdress = (UInt32(recMemBlk[0])<<8)+recMemBlk[1];
            baseAdress <<= (1<<type);   // Extended address record
        }
        else
        {   // Typ 3/5 (Start segment/linear address record) werden ignoriert
        }
        recMemLen = 0;
    }
    memSegemnt = MemSegment(int(recMemAdr), recMemLen, recMemBlk);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static const char*
readNextLine(const char*& text)
{
    static char  line[TextLineCapacity+1];

    line[0] = '\0';
    int len = 0;
    char oc = '\r';
    if (text == nullptr) return line;
    while (true)
    {   char c = *text;
        if (c != 0) text++;
        // (1) Tabs in Leerzeichen umwandeln
        if (c == '\t') c = ' ';
        // (2) LF in CR umwandeln
        if (c == '\n') c = '\r';
        // (3) Leere Zeilen ignorieren
        if (oc == '\r' && c == '\r') continue;
        // (4) Leerzeichen am Anfang einer Zeile ignorieren
        if (len == 0 && c == ' ') continue;
        // (5) Bei doppelten Leerzeichen das zweite Leerzeichen ignorieren
        if (oc == ' ' && c == ' ') continue;
        // (6) Aktuell empfangenes Zeichen für den nachfolgenden Durchlauf merken
        oc = c;
        // (7) Im Falle eines "druckbaren Zeichens", Zeichen an Zeilen anfügen
        if (c >= char(' ') && c <= char('~'))
        {   if (len < int(sizeof(line))-1)
            {   line[len] = c;
                len++;
            }
            continue;
        }
        // (8) Zeilenende
        if (c == '\r' || c == '\0')
        {   line[len] = '\0';
            break;
        }
    }
    return line;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  parseHexText(): Liest <text> ein und erzeugt für jede gültige Hex-Text-Zeile einen  //
//  --------------  Eintrag in der MemSegment-Liste <segmentList>. Trat beim Einlesen   //
//  ein Fehler auf, kehrt die Funktion mit False zurück. <segmentList> ist den Fall     //
//  leer.                                                                               //
//                                                                                      //
//  Eingabe: · text:        Text mit Binärdatei im Intel-Hex-Fortmat.                   //
//           · segmentList: Referenz auf eine 'MemSegment'-Liste, die die eingelesenen  //
//                          Speichersegmente aufnimmt.                                  //
//  Ausgabe: 'true', wenn der angegebene Text als HEX-Text interpretiert werden konnte. //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
parseHexText(const char* text, list<MemSegment>& segmentList)
{
    segmentList.clear();
    if (text == nullptr) return true;
    MemSegment dummy;
    parseHexRecord(":020000040000FA", dummy);   // Segment-Basisadresse auf 0 setzen
    do
    {   const char* line = readNextLine(text);
        MemSegment record;
        if (parseHexRecord(line, record))
        {   if (!record.isEmpty()) segmentList.push_back(record);
        }
        else
        {   segmentList.clear();
            return false;
        }
    } while (*text != '\0');
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  binaryStats(): Ermittelt den Adressbereich und die Speicherabdeckung der ange-      //
//  -------------  gebenen Speichersegmenten.                                           //
//                                                                                      //
//  Eingabe: · segmentArray: Feld mit den betrachteten Speichersegmenten.               //
//           · first:        Nimmt die erste überdeckte Speicheradresse auf.            //
//           · last:         Nimmt die letzte überdeckte Speicheradresse auf.           //
//           · size:         Größe des Speicherbereiches (= 'last' - 'first')           //
//           · coverage:     Anzahl genutzter Bytes zwischen 'first' und 'last', d.h.   //
//                           die Speicherabdeckung.                                     //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
static void
binaryStats(const vector<MemSegment>& segmentArray, int& first, int& last, int& size, int& coverage)
{
    first    = INT_MAX;
    last     = 0;
    coverage = 0;
    for (USize i=0; i<segmentArray.size(); i++)
    {   if (first > segmentArray[i].address())
        {   first = segmentArray[i].address();
        }
        if (last  < segmentArray[i].address() + segmentArray[i].size()-1)
        {   last  = segmentArray[i].address() + segmentArray[i].size()-1;
        }
        coverage += segmentArray[i].size();
    }
    size = last - first + 1;
    if (last < first || coverage > size)
    {   first    = INVALID;
        last     = INVALID;
        size     = INVALID;
        coverage = INVALID;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  sliceBlock(): Zerlegt den Speicherblock in Speichersegmente mit eine Größe von      //
//  ------------  (höchstens) <size> Bytes.                                             //
//                                                                                      //
//  Eingabe: · block:        Der zu zerlegenden Speicherblock.                          //
//           · segmentArray: Feld, das die erzeugten Speichersegmente aufnimmt.         //
//           · size:         Maximale Größe eines Speichersegments: 1 .. MemSegment-    //
//                           Capacity [Byte].                                           //
//////////////////////////////////////////////////////////////////////////////////////////
static void
sliceBlock(const MemBlock& block, vector<MemSegment>& segmentArray, int size)
{
    segmentArray.clear();
    if (size < 1) return;
    if (size > MemSegmentCapacity) size = MemSegmentCapacity;
    list<MemSegment> chunkList;
    int index = 0;
    UInt32 base  = block.base;
    do
    {   while (index < block.size() && !block.isUsed(index))
        {   index++;
            base++;
        }
        if (index == block.size()) break;
        UInt32 adr = base;
        int     len = 0;
        UInt08  blk[MemSegmentCapacity];
        while (index < block.size() && block.isUsed(index) && len < size)
        {   blk[len] = block[index];
            len++;
            index++;
            base++;
        }
        chunkList.push_back(MemSegment(int(adr), len, blk));
    }
    while (index < block.size());
    segmentArray.resize(chunkList.size());
    for (USize i=0; i<segmentArray.size(); i++)
    {   segmentArray[i] = chunkList.front();
        chunkList.pop_front();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  mergeSegments(): Verschmilzt die einzelnen Speichersegmente zu einem Speicherblock. //
//  ---------------                                                                     //
//                                                                                      //
//  Eingabe: · segmentArray: Feld mit den zusammenzuführenden Speichersegmenten.        //
//           · block:        Der aus den <segment> entstandene Speicherblock.           //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
static void
mergeSegments(const vector<MemSegment>& segmentArray, MemBlock& block)
{
    block.clear();
    int  first;
    int  last;
    int  dummy;
    binaryStats(segmentArray, first, last, dummy, dummy);
    if (last - first > 1024*1024*20) return;
    block.base = UInt32(first);
    if (!segmentArray.empty())
    {   block.resize(last - int(block.base)+1);
        for (int i=0; i<block.size(); i++) block.setUnused(i);
        for (USize i=0; i<segmentArray.size(); i++)
        {   int base = segmentArray[i].address() - int(block.base);
            for (int j=0; j<segmentArray[i].size(); j++) block.set(base+j, segmentArray[i][j]);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  optimizeBinary(): Reorganisiert die angegebenen Speichersegmente unter Berücksich-  //
//  ----------------  tigung der gewünschten Größe nach Startadressen aufsteigend neu.  //
//                                                                                      //
//  Eingabe: · segmentArray: Referenz auf ein Feld, dass die Speichersegmente bereit-   //
//                           stellt und die reorganisierten Speichersegmente wieder     //
//                           aufnimmt.                                                  //
//           · size:         Maximale Anzahl Bytes in einem Speichersegment.            //
//  Ausgabe: -                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
static void
optimizeBinary(vector<MemSegment>& segmentArray, int size)
{
    MemBlock  block;

    mergeSegments(segmentArray, block);
    sliceBlock(block, segmentArray, size);
}

//////////////////////////////////////////////////////////////////////////////////////////
static list<Cy4TypePlate>
listTypePlates(const vector<MemSegment>& segmentArray)
{
    list<Cy4TypePlate> typePlateList;

    MemBlock  block;
    mergeSegments(segmentArray, block);
    vector<UInt08> memory = vector<UInt08>(USize(block.size()), 0xFF);
    for (int i=0; i<block.size(); i++)
    {   if (block.isUsed(i)) memory[USize(i)] = UInt08(block[i]);
    }
    string cy4CodeMarkText = Cy4CodeMark;
    vector<UInt08> codeMarkMem(cy4CodeMarkText.size());
    for (USize i=0; i<codeMarkMem.size(); i++)
    {   codeMarkMem[i] = UInt08(cy4CodeMarkText[i]);
    }
    USize i = 0;
    while (i<memory.size()-7)
    {   vector<UInt08> currentCodeMark(7);
        for (USize j=0; j<7; j++) currentCodeMark[j] = memory[i+j];
        if (currentCodeMark == codeMarkMem)
        {   // Cy4NET-Markierung im Code gefunden, nun Typenschild auslesen
            i+=7;
            vector<UInt08> typePlateMem(7);
            for (USize j=0; j<7; j++) typePlateMem[j] = memory[i+j];
            Cy4TypePlate tmpTypePlate = Cy4TypePlate(typePlateMem);
            if (tmpTypePlate.isValid())
            {   typePlateList.push_back(tmpTypePlate);
                i+=7;
            }
        }
        else
        {   i++;
        }
    }
    return typePlateList;
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
hexTextToBinary(const std::string& hexText, std::vector<MemSegment>& segmentArray)
{
    segmentArray.clear();
    // Binärdaten aus Hextext extrahieren
    list<MemSegment> binSliceList;
    if (!parseHexText(hexText.data(), binSliceList)) return false;
    segmentArray.resize(binSliceList.size());
    for (USize i=0; i<segmentArray.size(); i++)
    {   segmentArray[i] = binSliceList.front();
        binSliceList.pop_front();
    }
    optimizeBinary(segmentArray, CY4_PAYLOAD_CAPACITY);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Message
createMessage(const Cy4Address& cy4Address, const MemSegment& segment)
{
    if( segment.isEmpty() ) return Cy4Message();
    std::vector<UInt08> data(USize(segment.size()));
    for( int i=0; i<int(data.size()); i++ ) data[USize(i)] = segment[i];
    int memAddress = int(UInt32(segment.address())&0x00FFFFFF); // Nur 24 LSBs der Adresse zulassen (0 .. 0xFFFFFF)
    return Cy4Message(cy4Address, Cy4Location(memAddress), data);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Firmware::Cy4Firmware(const QString& filePath)
{
    load(filePath);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Firmware::load(const QString& filePath)
{
    QString issue;
    return load(filePath, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Firmware::load(const QString& filePath, QString& issue)
{
    *this = Cy4Firmware();
    QString fileText;
    if (!Global::load(filePath, fileText, issue)) return false;
    std::vector<MemSegment> memSegments;
    if (!hexTextToBinary(fileText.toStdString(), memSegments)) return false;
    BinaryInfo tmpBinaryInfo;
    binaryStats(memSegments, tmpBinaryInfo.first, tmpBinaryInfo.last, tmpBinaryInfo.size, tmpBinaryInfo.coverage);
    if (!tmpBinaryInfo.isValid()) return false;
    list<Cy4TypePlate> typePlateList = listTypePlates(memSegments);
    mHexText    = fileText.toStdString();
    mBinaryInfo = tmpBinaryInfo;
    if (typePlateList.size() == 1) mTypePlate  = typePlateList.front();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4Firmware::BinaryInfo&
Cy4Firmware::binaryInfo(void) const
{
    return mBinaryInfo;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4TypePlate&
Cy4Firmware::typePlate(void) const
{
    return mTypePlate;
}

//////////////////////////////////////////////////////////////////////////////////////////
std::list<Cy4Message>
Cy4Firmware::toMessages(const Cy4Address& address) const
{
    if (address.isBroadcast()) return list<Cy4Message>();
    std::vector<MemSegment> memSegments;
    hexTextToBinary(mHexText, memSegments);
    std::list<Cy4Message> list;
    for (USize i=0; i<memSegments.size(); i++)
    {   list.push_back(createMessage(address, memSegments[i]));
    }
    return list;
}

//////////////////////////////////////////////////////////////////////////////////////////


