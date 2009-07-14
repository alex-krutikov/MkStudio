#ifndef __MBTCPSERVER_H__
#define __MBTCPSERVER_H__

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
  ModbusTcpServerThreadItem()
  { timeout_timer=0;
  }

  QByteArray req,ans;
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
  void timerEvent( QTimerEvent * event );
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
