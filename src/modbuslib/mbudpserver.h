#ifndef __MBUDPSERVER_H__
#define __MBUDPSERVER_H__

#include "abstractmbserver.h"

#include <memory>

class AbstractSerialPort;
class ModbusUdpServerThread;

//=============================================================================
//
//=============================================================================
class ModbusUdpServer : public AbstractMBServer
{
public:
  ModbusUdpServer(AbstractSerialPort *sp = 0, uint16_t udp_port = 502);
  ~ModbusUdpServer();

  void setReplyDelay(int delay) override;

private:
  std::unique_ptr<ModbusUdpServerThread> sp_thread;

};

#endif
