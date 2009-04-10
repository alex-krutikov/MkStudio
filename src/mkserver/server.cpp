#include <QtCore>
#include <QtNetwork>

#include "server.h"
#include "abstractserialport.h"

#include "console.h"

//Console::Print("d");

//#############################################################################
//
//#############################################################################
void SerialPortThread::run()
{
  exec();
}

//#############################################################################
//
//#############################################################################
void TcpServerThread::run()
{
  exec();
}

//#############################################################################
//
//#############################################################################
ModbusTcpServer::ModbusTcpServer( QObject *parent, AbstractSerialPort *sp )
  : QObject( parent ), sp( sp )
{
  sp_thread.sp = sp;

  sp_thread.start();
  tcp_thread.start();
}

//=============================================================================
//
//=============================================================================
ModbusTcpServer::~ModbusTcpServer()
{
  tcp_thread.exit();
  tcp_thread.wait(5000);

  sp_thread.exit();
  sp_thread.wait(5000);
}

