#include "mbtcpserver.h"

#include "abstractserialport.h"
#include "console.h"
#include "crc.h"
#include "mbcommon.h"
#include "utils.h"

#include <QTcpServer>
#include <QTcpSocket>

//#############################################################################
//
//#############################################################################
void ModbusTcpServerThread::run()
{
    bool ok;

    // запуск сервера
    server = new QTcpServer(this);
    ok = server->listen(QHostAddress::Any, mb_tcp_port);

    QString message
        = ok ? QString("TCP Server started on port %1.\n").arg(mb_tcp_port)
             : QString("TCP Server: Unable to listen port %1.\n")
                   .arg(mb_tcp_port);
    Console::Print(Console::Information, message);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));


    startTimer(1000);

    // запуск цикла сообщеий
    exec();

    // завершение работы сервера
    server->close();
    while (server->hasPendingConnections())
    {
        QTcpSocket *socket = server->nextPendingConnection();
        socket->disconnectFromHost();
        socket->waitForDisconnected();
    }
    foreach (QTcpSocket *socket, map.keys())
    {
        socket->disconnectFromHost();
    }
    foreach (QTcpSocket *socket, map.keys())
    {
        socket->waitForDisconnected();
    }

    delete server;
}

//=============================================================================
/// Новое подключение
//=============================================================================
void ModbusTcpServerThread::newConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    if (!socket) return;
    Console::Print(
        Console::Information,
        QString("Connected from %1\n").arg(socket->peerAddress().toString()));
    map.insert(socket, ModbusTcpServerThreadItem());
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

//=============================================================================
/// Чтение новых данных из сокета
//=============================================================================
void ModbusTcpServerThread::readyRead()
{
    QTcpSocket *socket = (QTcpSocket *)qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    int j;

    ProtocolAnalyser protocolAnalyser;

    ModbusTcpServerThreadItem &item = map[socket];
    QByteArray &tcp_req = item.req;
    QByteArray &tcp_ans = item.ans;

    item.timeout_timer = 0; // сброс счетчика таймаута неактивности

    tcp_req.append(socket->readAll());

    // Console::Print( "###:" + QByteArray2QString( req ) + "\n" );

    if (tcp_req.size() < 7) return;

    if (tcp_req.size() > 300 || tcp_req[2] || tcp_req[3] || tcp_req[4])
    {
        map.remove(socket);
        socket->deleteLater();
        return;
    }

    int len = (unsigned char)tcp_req[5];

    if (tcp_req.size() == len + 6)
    {
        Console::Print(Console::ModbusPacket,
                       "TCP Req:" + QByteArray2QString(tcp_req) + "\n");

        QByteArray mb_ans;
        QByteArray mb_req = tcp_req.mid(6, len);
        CRC::appendCRC16(mb_req);
        Console::Print(Console::ModbusPacket,
                       "MB  Req:" + QByteArray2QString(mb_req) + "\n");

        protocolAnalyser.reset();
        protocolAnalyser.setRequest(mb_req);

        j = protocolAnalyser.expectedResponseLength();

        mb_ans.resize(j);
        j = sp->query(mb_req, mb_ans, 0);
        mb_ans.resize(j);

        Console::Print(Console::ModbusPacket,
                       "MB  Ans:" + QByteArray2QString(mb_ans) + "\n");

        protocolAnalyser.setResponse(mb_ans);

        mb_ans.chop(2);

        tcp_ans.resize(0);
        tcp_ans.append(tcp_req[0]);
        tcp_ans.append(tcp_req[1]);
        tcp_ans.append((char)0);
        tcp_ans.append((char)0);
        tcp_ans.append((char)0);
        tcp_ans.append((char)mb_ans.size());
        tcp_ans.append(mb_ans);

        if (replay_delay)
        {
            Console::Print(
                Console::ModbusPacket,
                QString("TCP Ans: Waiting delay %1 ms\n").arg(replay_delay));
            QThread::msleep(replay_delay);
        }

        Console::Print(Console::ModbusPacket,
                       "TCP Ans:" + QByteArray2QString(tcp_ans) + "\n\n");

        int j = socket->write(tcp_ans);
        if (j != tcp_ans.size()) { Console::Print(Console::Error, "!:\n"); }
        socket->flush();

        tcp_req.resize(0);

    } else
    {
        Console::Print(Console::ModbusPacket, "################3\n");
        delete socket;
        return;
    }
}

//=============================================================================
/// Таймер проверки таймаута неактивности клиента
//=============================================================================
void ModbusTcpServerThread::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QMutableMapIterator<QTcpSocket *, ModbusTcpServerThreadItem> i(map);
    while (i.hasNext())
    {
        i.next();
        QTcpSocket *socket = (QTcpSocket *)i.key();
        ModbusTcpServerThreadItem &item = i.value();

        item.timeout_timer++;

        if (item.timeout_timer > 60) // 1 минута
        {
            Console::Print(Console::Information,
                           "Host: " + socket->peerAddress().toString()
                               + " - time out.\n");
            item.timeout_timer = 0;
            socket->disconnectFromHost();
        }
    }
}

//=============================================================================
/// Отсоедиение сокета
//=============================================================================
void ModbusTcpServerThread::disconnected()
{
    QTcpSocket *socket = (QTcpSocket *)qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    Console::Print(
        Console::Information,
        QString("Disconnected %1\n").arg(socket->peerAddress().toString()));

    map.remove(socket);
    socket->deleteLater();
}

//#############################################################################
//
//#############################################################################
ModbusTcpServer::ModbusTcpServer(AbstractSerialPort *sp, int mb_tcp_port)
    : sp(sp)
    , mb_tcp_port(mb_tcp_port)
{
    sp_thread = new ModbusTcpServerThread;

    sp_thread->sp = sp;
    sp_thread->mb_tcp_port = mb_tcp_port;
    sp_thread->replay_delay = 0;
    sp_thread->moveToThread(sp_thread);
    sp_thread->start();
}

//=============================================================================
//
//=============================================================================
void ModbusTcpServer::setReplyDelay(int delay)
{
    sp_thread->replay_delay = delay;
}

//=============================================================================
//
//=============================================================================
ModbusTcpServer::~ModbusTcpServer()
{
    sp_thread->exit();
    bool ok = sp_thread->wait(5000);

    if (!ok)
    {
        sp_thread->terminate();
        sp_thread->wait(5000);
    }
    Console::Print(
        Console::Information,
        QString("TCP Server stopped on port %1.\n").arg(mb_tcp_port));
    delete sp_thread;
}
