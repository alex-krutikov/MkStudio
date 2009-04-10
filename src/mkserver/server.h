#ifndef __SERVER_H__
#define __SERVER_H__

#include <QObject>
#include <QThread>

class AbstractSerialPort;

//=============================================================================
//
//=============================================================================
class TcpServerThread : public QThread
{
  friend class ModbusTcpServer;
  Q_OBJECT
protected:
  void run();

private:
};

//=============================================================================
//
//=============================================================================
class SerialPortThread : public QThread
{
  friend class ModbusTcpServer;
  Q_OBJECT
protected:
  void run();

private:
  AbstractSerialPort *sp;
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

  TcpServerThread  tcp_thread;
  SerialPortThread sp_thread;

  AbstractSerialPort *sp;
};

#endif
