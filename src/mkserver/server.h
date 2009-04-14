#ifndef __SERVER_H__
#define __SERVER_H__

#include <QObject>
#include <QThread>

class AbstractSerialPort;
class QTcpServer;

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

  QTcpServer *server;
};

//=============================================================================
//
//=============================================================================
class ModbusTcpServer : public QObject
{
  Q_OBJECT
public:
  ModbusTcpServer( QObject *parent = 0, AbstractSerialPort *sp = 0 );
  ~ModbusTcpServer();

private:
  ModbusTcpServerThread *sp_thread;

  AbstractSerialPort *sp;
};

#endif
