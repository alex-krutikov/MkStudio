#include "mbudpserver.h"

#include "abstractserialport.h"
#include "crc.h"
#include "console.h"
#include "mbcommon.h"

#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QList>

//==============================================================================
/// Предсказание длины ответного пакета в соответствии с протоколом MIKKON Modbus
//==============================================================================
static int expected_answer_size( const QByteArray &ba )
{
  int len = ba.size();
  if( len < 3 ) return 255;
  if(     ( ba[1] == 0x41 )
       || ( ba[1] == 0x43 ) )
  { int dsize = (unsigned char)ba[5];
    switch( ba[2] & 0x0F )
    { case( 0x00 ): // read
        return dsize + 6;
        break;
      case( 0x01 ): // set
      case( 0x03 ):
      case( 0x05 ):
      case( 0x07 ):
        return dsize + 8;
        break;
    }
  }

  if( ba[1] == 0x45 )
  { switch( ba[2] )
    { case(  0 ): return (1+1+1)+4+2;                      // firmware: request for reset
      case(  1 ): return (1+1+1)+4+2;                      // firmware: response for reset
      case(  2 ): return (1+1+1)+4+2;                      // firmware: request for firmware
      case(  3 ): return (1+1+1)+4+2;                      // firmware: response for firmware
      case(  4 ): return (1+1+1)+1+(unsigned char)ba[7]+2; // firmware: read
      case(  5 ): return (1+1+1)+4+4+2;                    // firmware: erase sectors
      case(  6 ): return (1+1+1)+1+(unsigned char)ba[5]+2; // firmware: write to buffer
      case(  7 ): return (1+1+1)+4+4+2;                    // firmware: flash buffer
      case(  8 ): return (1+1+1)+1+(unsigned char)ba[3]+2; // firmware: information
      case(  9 ): return (1+1+1)+2+2+4+2;                  // firmware: flash loader
      case( 12 ): return (1+1+1)+1+4+(unsigned char)ba[7]+2;  // firmware: read (version 2)
      case( 14 ): return (1+1+1)+2+1+(unsigned char)ba[5]+2;  // firmware: write to buffer (version 2)
      
      case( 33 ): return ba.size()+2;                      // create/open file
      case( 34 ): return ba.size()+1;                      // close file
      case( 35 ): return ba.size()+2+(unsigned char)ba[8]; // read file
      case( 36 ): return (1+1+1)+1+4+1+1+1+2;              // write file
      case( 40 ): return ba.size()+2;                      // open dir
      case( 41 ): return (1+1+1)+1+1+4+2+2+1+1+13+2;       // read dir
      case( 43 ): return ba.size()+1+(4+2+2+1+1+13);       // file status
      case( 44 ): return ba.size()+1;                      // create dir
      case( 45 ): return ba.size()+1;                      // remove file or dir
    }
  }


  return 255;
}


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
  volatile bool exit_flag;
};

//#############################################################################
//
//#############################################################################
void ModbusUdpServerThread::run()
{
  QUdpSocket socket;

  QList<QNetworkDatagram> udp_requests;

  bool ok = socket.bind(QHostAddress::Any, udp_port);

  QString message = ok ? QString("UDP Server started on port %1.\n").arg(udp_port)
                       : QString("UDP Server: Unable to listen port %1.\n").arg(udp_port);
  Console::Print( Console::Information, message );

  exit_flag = false;

  int j;

  while(!exit_flag) {

      if (!socket.isValid())  {
          socket.close();
          bool ok = socket.bind(QHostAddress::Any, udp_port);
          QString message = ok ? QString("UDP Server started on port %1.\n").arg(udp_port)
                               : QString("UDP Server: Unable to listen port %1.\n").arg(udp_port);
          Console::Print( Console::Information, message );
          if (!ok) {
              QThread::msleep(500);
          }
      }

      while(socket.hasPendingDatagrams()) {

          QNetworkDatagram req_datagram = socket.receiveDatagram();

          if (!req_datagram.isValid()) {
            Console::Print( Console::ModbusPacket, "UDP Received request invalid\n");
            Console::Print( Console::Information, "UDP socket error: \"" + socket.errorString() + "\"\n");
            continue;
          }

          for (auto it = udp_requests.begin(); it != udp_requests.end(); ++it) {
              if (req_datagram.senderPort() == it->senderPort()) {
                  if (req_datagram.senderAddress() == it->senderAddress()) {
                    udp_requests.erase(it);
                    Console::Print( Console::Warning, QString("UDP: Request overrun from host: %1:%2\n")
                                    .arg(req_datagram.senderAddress().toString()).arg(req_datagram.senderPort()));
                    break;
                  }
              }

          }

          udp_requests.push_front(req_datagram);

          Console::Print( Console::ModbusPacket, "UDP Received request:" + QByteArray2QString( req_datagram.data() ) + "\n" );
      }

      if (udp_requests.isEmpty()) {
            socket.waitForReadyRead(100);
      }

      if(!udp_requests.isEmpty()) {
          Console::Print( Console::ModbusPacket, "\nUDP Process request (queue size:   " + QString::number(udp_requests.count())+ ")\n" );

          QNetworkDatagram req_datagram = udp_requests.first();
          udp_requests.pop_front();

          QByteArray udp_req = req_datagram.data();
          QByteArray udp_ans;

          if( udp_req.size() > 6 )
          {

              int len = (unsigned char)udp_req[5];

              if( udp_req.size() == len + 6 )
              {
                  Console::Print( Console::ModbusPacket, "UDP Req:" + QByteArray2QString( udp_req ) + "\n" );

                  QByteArray mb_ans;
                  QByteArray mb_req = udp_req.mid(6,len);
                  CRC::appendCRC16( mb_req );
                  Console::Print( Console::ModbusPacket, "MB  Req:" + QByteArray2QString( mb_req ) + "\n" );


                  j = expected_answer_size( mb_req );
                  mb_ans.resize( j );
                  j = sp->query( mb_req, mb_ans, nullptr );
                  mb_ans.resize( j );

                  Console::Print( Console::ModbusPacket, "MB  Ans:" + QByteArray2QString( mb_ans ) + "\n" );
                  mb_ans.chop(2);

                  udp_ans.resize(0);
                  udp_ans.append( udp_req[0] );
                  udp_ans.append( udp_req[1] );
                  udp_ans.append( (char) 0 );
                  udp_ans.append( (char) 0 );
                  udp_ans.append( (char) (mb_ans.size() >> 8) );
                  udp_ans.append( (char) (mb_ans.size()) );
                  udp_ans.append( mb_ans );

                  Console::Print( Console::ModbusPacket, "UDP Ans:" + QByteArray2QString( udp_ans ) + "\n\n" );

                  int j = socket.writeDatagram(std::move(req_datagram).makeReply(udp_ans));
                  if( j != udp_ans.size() )
                  {
                      Console::Print(  Console::Error, "UDP Ans: Cannot write datagram!:\n" );
                  }

                  udp_req.resize(0);
              }
              else
              {
                  Console::Print(Console::ModbusPacket, "UDP Req: Wrong request (too short):" + QByteArray2QString( udp_req ) + "\n\n" );
              }

          }
          else
          {
              Console::Print(Console::ModbusPacket, "UDP Req: Wrong request format:" + QByteArray2QString( udp_req ) + "\n\n" );
          }
      }
  }
}

//#############################################################################
//
//#############################################################################
ModbusUdpServer::ModbusUdpServer( QObject *parent, AbstractSerialPort *sp,  uint16_t udp_port )
  : QObject( parent )
  , sp_thread(std::make_unique<ModbusUdpServerThread>())
{
  sp_thread->sp = sp;
  sp_thread->udp_port = udp_port;
  sp_thread->start();

}

//=============================================================================
//
//=============================================================================
ModbusUdpServer::~ModbusUdpServer()
{

  sp_thread->exit_flag = true;

  bool ok = sp_thread->wait(5000);

  if( !ok )
  { sp_thread->terminate();
    sp_thread->wait(5000);
  }
  Console::Print( Console::Information, QString("UDP Server stopped on port %1.\n").arg(sp_thread->udp_port) );
}

