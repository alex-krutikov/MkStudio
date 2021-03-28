#include "mbudpserver.h"

#include "abstractserialport.h"
#include "console.h"
#include "crc.h"
#include "mbcommon.h"
#include "utils.h"

#include <QList>
#include <QNetworkDatagram>
#include <QThread>
#include <QUdpSocket>

#include <atomic>

//=============================================================================
//
//=============================================================================
class ModbusUdpServerThread : public QThread
{
    friend class ModbusUdpServer;

protected:
    void run() override;

private:
    AbstractSerialPort *sp;
    uint16_t udp_port;
    std::atomic_bool exit_flag;
    std::atomic_int replay_delay;
};

//#############################################################################
//
//#############################################################################
void ModbusUdpServerThread::run()
{
    ProtocolAnalyser protocolAnalyser;

    QUdpSocket socket;

    QList<QNetworkDatagram> udp_requests;

    bool ok = socket.bind(QHostAddress::Any, udp_port);

    QString message
        = ok ? QString("UDP Server started on port %1.\n").arg(udp_port)
             : QString("UDP Server: Unable to listen port %1.\n").arg(udp_port);
    Console::Print(Console::Information, message);

    exit_flag = false;

    int j;

    while (!exit_flag)
    {

        if (!socket.isValid())
        {
            socket.close();
            bool ok = socket.bind(QHostAddress::Any, udp_port);
            QString message
                = ok ? QString("UDP Server started on port %1.\n").arg(udp_port)
                     : QString("UDP Server: Unable to listen port %1.\n")
                           .arg(udp_port);
            Console::Print(Console::Information, message);
            if (!ok) { QThread::msleep(500); }
        }

        while (socket.hasPendingDatagrams())
        {

            QNetworkDatagram req_datagram = socket.receiveDatagram();

            if (!req_datagram.isValid())
            {
                Console::Print(Console::ModbusPacket,
                               "UDP Received request invalid\n");
                Console::Print(Console::Information, "UDP socket error: \""
                                                         + socket.errorString()
                                                         + "\"\n");
                continue;
            }

            for (auto it = udp_requests.begin(); it != udp_requests.end(); ++it)
            {
                if (req_datagram.senderPort() == it->senderPort())
                {
                    if (req_datagram.senderAddress() == it->senderAddress())
                    {
                        udp_requests.erase(it);
                        Console::Print(
                            Console::Warning,
                            QString("UDP: Request overrun from host: %1:%2\n")
                                .arg(req_datagram.senderAddress().toString())
                                .arg(req_datagram.senderPort()));
                        break;
                    }
                }
            }

            udp_requests.push_front(req_datagram);

            Console::Print(Console::ModbusPacket,
                           "UDP Received request:"
                               + QByteArray2QString(req_datagram.data())
                               + "\n");
        }

        if (udp_requests.isEmpty()) { socket.waitForReadyRead(100); }

        if (!udp_requests.isEmpty())
        {
            Console::Print(Console::ModbusPacket,
                           "UDP Process request (queue size:   "
                               + QString::number(udp_requests.count()) + ")\n");

            QNetworkDatagram req_datagram = udp_requests.first();
            udp_requests.pop_front();

            QByteArray udp_req = req_datagram.data();
            QByteArray udp_ans;

            if (udp_req.size() > 6)
            {

                int len = (unsigned char)udp_req[5];

                if (udp_req.size() == len + 6)
                {
                    Console::Print(Console::ModbusPacket,
                                   "UDP Req:" + QByteArray2QString(udp_req)
                                       + "\n");

                    QByteArray mb_ans;
                    QByteArray mb_req = udp_req.mid(6, len);
                    CRC::appendCRC16(mb_req);
                    Console::Print(Console::ModbusPacket,
                                   "MB  Req:" + QByteArray2QString(mb_req)
                                       + "\n");

                    protocolAnalyser.reset();
                    protocolAnalyser.setRequest(mb_req);

                    j = protocolAnalyser.expectedResponseLength();

                    mb_ans.resize(j);
                    j = sp->query(mb_req, mb_ans, nullptr);
                    mb_ans.resize(j);

                    Console::Print(Console::ModbusPacket,
                                   "MB  Ans:" + QByteArray2QString(mb_ans)
                                       + "\n");

                    protocolAnalyser.setResponse(mb_ans);

                    mb_ans.chop(2);

                    udp_ans.resize(0);
                    udp_ans.append(udp_req[0]);
                    udp_ans.append(udp_req[1]);
                    udp_ans.append((char)0);
                    udp_ans.append((char)0);
                    udp_ans.append((char)(mb_ans.size() >> 8));
                    udp_ans.append((char)(mb_ans.size()));
                    udp_ans.append(mb_ans);

                    if (replay_delay)
                    {
                        Console::Print(Console::ModbusPacket,
                                       QString("UDP Ans: Waiting delay %1 ms\n")
                                           .arg(replay_delay));
                        QThread::msleep(replay_delay);
                    }

                    Console::Print(Console::ModbusPacket,
                                   "UDP Ans:" + QByteArray2QString(udp_ans)
                                       + "\n\n");

                    int j = socket.writeDatagram(
                        std::move(req_datagram).makeReply(udp_ans));
                    if (j != udp_ans.size())
                    {
                        Console::Print(Console::Error,
                                       "UDP Ans: Cannot write datagram!:\n");
                    }

                    udp_req.resize(0);
                } else
                {
                    Console::Print(Console::ModbusPacket,
                                   "UDP Req: Wrong request (too short):"
                                       + QByteArray2QString(udp_req) + "\n\n");
                }

            } else
            {
                Console::Print(Console::ModbusPacket,
                               "UDP Req: Wrong request format:"
                                   + QByteArray2QString(udp_req) + "\n\n");
            }
        }
    }
}

//#############################################################################
//
//#############################################################################
ModbusUdpServer::ModbusUdpServer(AbstractSerialPort *sp, uint16_t udp_port)
    : sp_thread(std::make_unique<ModbusUdpServerThread>())
{
    sp_thread->sp = sp;
    sp_thread->udp_port = udp_port;
    sp_thread->replay_delay = 0;
    sp_thread->start();
}

//=============================================================================
//
//=============================================================================
void ModbusUdpServer::setReplyDelay(int delay)
{
    sp_thread->replay_delay = delay;
}

//=============================================================================
//
//=============================================================================
ModbusUdpServer::~ModbusUdpServer()
{

    sp_thread->exit_flag = true;

    bool ok = sp_thread->wait(5000);

    if (!ok)
    {
        sp_thread->terminate();
        sp_thread->wait(5000);
    }
    Console::Print(
        Console::Information,
        QString("UDP Server stopped on port %1.\n").arg(sp_thread->udp_port));
}
