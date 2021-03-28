#include "utils.h"

#include "console.h"
#include "mbcommon.h"

namespace {

int mikkon_func_45_expacted_len(const QByteArray &ba)
{
    if (ba[1] == 0x45)
    {
        switch (ba[2])
        {
        case (0):
            return (1 + 1 + 1) + 4 + 2; // firmware: request for reset
        case (1):
            return (1 + 1 + 1) + 4 + 2; // firmware: response for reset
        case (2):
            return (1 + 1 + 1) + 4 + 2; // firmware: request for firmware
        case (3):
            return (1 + 1 + 1) + 4 + 2; // firmware: response for firmware
        case (4):
            return (1 + 1 + 1) + 1 + (unsigned char)ba[7] + 2; // firmware: read
        case (5):
            return (1 + 1 + 1) + 4 + 4 + 2; // firmware: erase sectors
        case (6):
            return (1 + 1 + 1) + 1 + (unsigned char)ba[5]
                 + 2; // firmware: write to buffer
        case (7):
            return (1 + 1 + 1) + 4 + 4 + 2; // firmware: flash buffer
        case (8):
            return (1 + 1 + 1) + 1 + (unsigned char)ba[3]
                 + 2; // firmware: information
        case (9):
            return (1 + 1 + 1) + 2 + 2 + 4 + 2; // firmware: flash loader
        case (12):
            return (1 + 1 + 1) + 1 + 4 + (unsigned char)ba[7]
                 + 2; // firmware: read (version 2)
        case (14):
            return (1 + 1 + 1) + 2 + 1 + (unsigned char)ba[5]
                 + 2; // firmware: write to buffer (version 2)

        case (33):
            return ba.size() + 2; // create/open file
        case (34):
            return ba.size() + 1; // close file
        case (35):
            return ba.size() + 2 + (unsigned char)ba[8]; // read file
        case (36):
            return (1 + 1 + 1) + 1 + 4 + 1 + 1 + 1 + 2; // write file
        case (40):
            return ba.size() + 2; // open dir
        case (41):
            return (1 + 1 + 1) + 1 + 1 + 4 + 2 + 2 + 1 + 1 + 13 + 2; // read dir
        case (43):
            return ba.size() + 1 + (4 + 2 + 2 + 1 + 1 + 13); // file status
        case (44):
            return ba.size() + 1; // create dir
        case (45):
            return ba.size() + 1; // remove file or dir
        }
    }
    return 255;
}

} // namespace

class ProtocolAnalyserPrivate
{
    friend class ProtocolAnalyser;

    enum class Protocol
    {
        UNKNOWN,
        MIKKON_READ,
        MIKKON_WRITE_SET,
        MIKKON_WRITE_AND,
        MIKKON_WRITE_OR,
        MIKKON_WRITE_XOR,
        MIKKON32_READ,
        MIKKON32_WRITE_SET,
        MIKKON32_WRITE_AND,
        MIKKON32_WRITE_OR,
        MIKKON32_WRITE_XOR,
    };

    Protocol protocol;
    int responseLength;
    int dataAddr;
    int dataSize;
};

ProtocolAnalyser::ProtocolAnalyser()
    : d {std::make_unique<ProtocolAnalyserPrivate>()}
{
}

ProtocolAnalyser::~ProtocolAnalyser() {}

void ProtocolAnalyser::reset()
{
    d->protocol = ProtocolAnalyserPrivate::Protocol::UNKNOWN;
    d->responseLength = 0;
    d->dataAddr = 0;
    d->dataSize = 0;
}

void ProtocolAnalyser::setRequest(const QByteArray &request)
{
    QByteArray data;
    int len = request.size();
    if (len < 3)
    {
        Console::Print(Console::ModbusPacket, "Protocol: Unknown protocol.^\n");
        return;
    }
    if ((request[1] == 0x41) || (request[1] == 0x43))
    {
        if (request[2] & 0x08)
        {
            // MIKKON32
            d->dataAddr = (static_cast<uint8_t>(request[3]) << 24)
                        | (static_cast<uint8_t>(request[4]) << 16)
                        | (static_cast<uint8_t>(request[5]) << 8)
                        | static_cast<uint8_t>(request[6]);
            d->dataSize = static_cast<uint8_t>(request[7])
                        | (static_cast<uint8_t>(request[8]) << 8);
            switch (request[2] & 0x07)
            {
            case (0x00): // read
                d->protocol = ProtocolAnalyserPrivate::Protocol::MIKKON32_READ;
                d->responseLength = d->dataSize + 7;
                Console::Print(
                    Console::ModbusPacket,
                    QString("Protocol: MIKKON32 READ Addr=%1 DataSize=%2 \n")
                        .arg(intToHex(d->dataAddr, 8))
                        .arg(d->dataSize));
                break;
            case (0x01): // set
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_SET;
                d->responseLength = d->dataSize + 11;
                data = request.mid(9, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON32 WRITE(SET) Addr=%1 "
                                       "DataSize=%2 Data=%3\n")
                                   .arg(intToHex(d->dataAddr, 8))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            case (0x03): // and
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_AND;
                d->responseLength = d->dataSize + 11;
                data = request.mid(9, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON32 WRITE(AND) Addr=%1 "
                                       "DataSize=%2 Data=%3\n")
                                   .arg(intToHex(d->dataAddr, 8))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            case (0x05): // or
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_OR;
                d->responseLength = d->dataSize + 11;
                data = request.mid(9, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON32 WRITE(OR) Addr=%1 "
                                       "DataSize=%2 Data=%3\n")
                                   .arg(intToHex(d->dataAddr, 8))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            case (0x07): // xor
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_XOR;
                d->responseLength = d->dataSize + 11;
                data = request.mid(9, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON32 WRITE(XOR) Addr=%1 "
                                       "DataSize=%2 Data=%3\n")
                                   .arg(intToHex(d->dataAddr, 8))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            }

        } else
        {

            // MIKKON
            d->dataAddr = (static_cast<uint8_t>(request[3]) << 8)
                        | static_cast<uint8_t>(request[4]);
            d->dataSize = static_cast<uint8_t>(request[5]);
            switch (request[2] & 0x07)
            {
            case (0x00): // read
                d->protocol = ProtocolAnalyserPrivate::Protocol::MIKKON_READ;
                d->responseLength = d->dataSize + 6;
                Console::Print(
                    Console::ModbusPacket,
                    QString("Protocol: MIKKON READ Addr=%1 DataSize=%2 \n")
                        .arg(intToHex(d->dataAddr, 4))
                        .arg(d->dataSize));
                break;
            case (0x01): // set
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_SET;
                d->responseLength = d->dataSize + 8;
                data = request.mid(6, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON WRITE(SET) Addr=%1 "
                                       "DataSize=%2 Data=[%3]\n")
                                   .arg(intToHex(d->dataAddr, 4))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            case (0x03): // and
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_AND;
                d->responseLength = d->dataSize + 8;
                data = request.mid(6, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON WRITE(AND) Addr=%1 "
                                       "DataSize=%2 Data=[%3]\n")
                                   .arg(intToHex(d->dataAddr, 4))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            case (0x05): // or
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_OR;
                d->responseLength = d->dataSize + 8;
                data = request.mid(6, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON WRITE(OR) Addr=%1 "
                                       "DataSize=%2 Data=[%3]\n")
                                   .arg(intToHex(d->dataAddr, 4))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            case (0x07): // xor
                d->protocol
                    = ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_XOR;
                d->responseLength = d->dataSize + 8;
                data = request.mid(6, d->dataSize);
                Console::Print(Console::ModbusPacket,
                               QString("Protocol: MIKKON WRITE(XOR) Addr=%1 "
                                       "DataSize=%2 Data=[%3]\n")
                                   .arg(intToHex(d->dataAddr, 4))
                                   .arg(d->dataSize)
                                   .arg(QByteArray2QString(data)));
                break;
            }
        }
    } else if (request[1] == 0x45)
    {
        Console::Print(Console::ModbusPacket,
                       "Protocol: MIKKON function 0x45\n");
        d->responseLength = mikkon_func_45_expacted_len(request);
    }
}

void ProtocolAnalyser::setResponse(const QByteArray &response)
{
    int size = 0, addr = 0;
    QByteArray data;

    switch (d->protocol)
    {
    case ProtocolAnalyserPrivate::Protocol::MIKKON_READ:
        size = static_cast<uint8_t>(response[3]);
        if (size != d->dataSize)
        {
            Console::Print(Console::ModbusPacket,
                           QString("Protocol: ERROR in response: data size=%1 "
                                   "expected value=%2\n")
                               .arg(size)
                               .arg(d->dataSize));
        }
        data = response.mid(4, size);
        Console::Print(Console::ModbusPacket,
                       QString("Protocol: MIKKON READ Data=[%1]\n")
                           .arg(QByteArray2QString(data)));
        break;
    case ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_SET:
    case ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_AND:
    case ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_OR:
    case ProtocolAnalyserPrivate::Protocol::MIKKON_WRITE_XOR:
        addr = (static_cast<uint8_t>(response[3]) << 8)
             | static_cast<uint8_t>(response[4]);
        ;
        size = static_cast<uint8_t>(response[5]);
        if (addr != d->dataAddr)
        {
            Console::Print(Console::ModbusPacket,
                           QString("Protocol: ERROR in response: data addr=%1 "
                                   "expected value=%2\n")
                               .arg(intToHex(addr, 4))
                               .arg(intToHex(d->dataAddr, 4)));
        }
        if (size != d->dataSize)
        {
            Console::Print(Console::ModbusPacket,
                           QString("Protocol: ERROR in response: data size=%1 "
                                   "expected value=%2\n")
                               .arg(size)
                               .arg(d->dataSize));
        }
        data = response.mid(6, size);
        Console::Print(Console::ModbusPacket,
                       QString("Protocol: MIKKON WRITE Data=[%1]\n")
                           .arg(QByteArray2QString(data)));
        break;


    case ProtocolAnalyserPrivate::Protocol::MIKKON32_READ:
        size = static_cast<uint8_t>(response[3])
             | (static_cast<uint8_t>(response[4]) << 8);
        if (size != d->dataSize)
        {
            Console::Print(Console::ModbusPacket,
                           QString("Protocol: ERROR in response: data size=%1 "
                                   "expected value=%2\n")
                               .arg(size)
                               .arg(d->dataSize));
        }
        data = response.mid(5, size);
        Console::Print(Console::ModbusPacket,
                       QString("Protocol: MIKKON32 READ Data=[%1]\n")
                           .arg(QByteArray2QString(data)));
        break;
    case ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_SET:
    case ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_AND:
    case ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_OR:
    case ProtocolAnalyserPrivate::Protocol::MIKKON32_WRITE_XOR:
        addr = (static_cast<uint8_t>(response[3]) << 24)
             | (static_cast<uint8_t>(response[4]) << 16)
             | (static_cast<uint8_t>(response[5]) << 8)
             | static_cast<uint8_t>(response[6]);
        size = static_cast<uint8_t>(response[7])
             | (static_cast<uint8_t>(response[8]) << 8);
        if (addr != d->dataAddr)
        {
            Console::Print(Console::ModbusPacket,
                           QString("Protocol: ERROR in response: data addr=%1 "
                                   "expected value=%2\n")
                               .arg(intToHex(addr, 8))
                               .arg(intToHex(d->dataAddr, 8)));
        }
        if (size != d->dataSize)
        {
            Console::Print(Console::ModbusPacket,
                           QString("Protocol: ERROR in response: data size=%1 "
                                   "expected value=%2\n")
                               .arg(size)
                               .arg(d->dataSize));
        }
        data = response.mid(9, size);
        Console::Print(Console::ModbusPacket,
                       QString("Protocol: MIKKON32 WRITE Data=[%1]\n")
                           .arg(QByteArray2QString(data)));
        break;
    default:
        break;
    }
}

int ProtocolAnalyser::expectedResponseLength() const
{
    return d->responseLength;
}
