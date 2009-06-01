#ifndef __SERVER_H__
#define __SERVER_H__

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <QMap>

class AbstractSerialPort;
class QTcpServer;
class QTcpSocket;


//=============================================================================
//
//=============================================================================
struct ModbusTcpServerThreadItem
{
  QByteArray req,ans;
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
public slots:
  void newConnection();
  void readyRead();
  void disconnected();
private:
  AbstractSerialPort *sp;
  int mb_tcp_port;
  QTcpServer *server;
  QMap< QTcpSocket*, ModbusTcpServerThreadItem  > map;
};

//=============================================================================
//
//=============================================================================
class ModbusTcpServer : public QObject
{
  Q_OBJECT
public:
  ModbusTcpServer( QObject *parent = 0, AbstractSerialPort *sp = 0, int mb_tcp_port=502 );
  ~ModbusTcpServer();

private:
  ModbusTcpServerThread *sp_thread;

  AbstractSerialPort *sp;
  int mb_tcp_port;
};

#endif
