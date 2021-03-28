#ifndef __MBTCPSERVER_H__
#define __MBTCPSERVER_H__

#include "abstractmbserver.h"

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QThread>

#include <atomic>

class AbstractSerialPort;
class QTcpServer;
class QTcpSocket;


//=============================================================================
//
//=============================================================================
struct ModbusTcpServerThreadItem
{
    ModbusTcpServerThreadItem() { timeout_timer = 0; }

    QByteArray req, ans;
    int timeout_timer;
};

//=============================================================================
//
//=============================================================================
class ModbusTcpServerThread : public QThread
{
    friend class ModbusTcpServer;
    Q_OBJECT
protected:
    void run();
    void timerEvent(QTimerEvent *event);
public slots:
    void newConnection();
    void readyRead();
    void disconnected();

private:
    AbstractSerialPort *sp;
    int mb_tcp_port;
    QTcpServer *server;
    std::atomic_int replay_delay;

    QMap<QTcpSocket *, ModbusTcpServerThreadItem> map;
};

//=============================================================================
//
//=============================================================================
class ModbusTcpServer : public AbstractMBServer
{
public:
    ModbusTcpServer(AbstractSerialPort *sp = 0, int mb_tcp_port = 502);
    ~ModbusTcpServer();

    void setReplyDelay(int delay) override;

private:
    ModbusTcpServerThread *sp_thread;

    AbstractSerialPort *sp;
    int mb_tcp_port;
};

#endif
