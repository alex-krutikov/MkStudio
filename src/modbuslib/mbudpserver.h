#ifndef __MBUDPSERVER_H__
#define __MBUDPSERVER_H__

#include <QObject>
#include <QThread>

#include <memory>

class AbstractSerialPort;
class ModbusUdpServerThread;

//=============================================================================
//
//=============================================================================
class ModbusUdpServer : public QObject
{
  Q_OBJECT
public:
  ModbusUdpServer( QObject *parent = 0, AbstractSerialPort *sp = 0, uint16_t udp_port = 502 );
  ~ModbusUdpServer();

private:
  std::unique_ptr<ModbusUdpServerThread> sp_thread;

};

#endif
