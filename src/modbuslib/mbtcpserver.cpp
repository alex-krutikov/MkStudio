#include <QtCore>
#include <QtNetwork>

#include "mbtcpserver.h"
#include "abstractserialport.h"
#include "crc.h"

#include "console.h"
#include "mbcommon.h"

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

//#############################################################################
//
//#############################################################################
void ModbusTcpServerThread::run()
{
  bool ok;

  // запуск сервера
  server = new QTcpServer(this);
  ok = server->listen( QHostAddress::Any, mb_tcp_port );

  QString message = ok ? QString("Server started on port %1.\n").arg(mb_tcp_port)
                       : QString("Unable to listen port %1.\n").arg(mb_tcp_port);
  Console::Print( Console::Information, message );

  connect( server, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );


  startTimer( 1000 );

  // запуск цикла сообщеий
  exec();

  // завершение работы сервера
  server->close();
  while( server->hasPendingConnections() )
  { QTcpSocket *socket = server->nextPendingConnection();
    socket->disconnectFromHost();
    socket->waitForDisconnected();
  }
  foreach(QTcpSocket *socket, map.keys() )
  { socket->disconnectFromHost();
  }
  foreach(QTcpSocket *socket, map.keys() )
  { socket->waitForDisconnected();
  }

  delete server;
}

//=============================================================================
/// Новое подключение
//=============================================================================
void ModbusTcpServerThread::newConnection()
{
  QTcpSocket *socket = server->nextPendingConnection();
  if( !socket ) return;
  Console::Print( Console::Information, QString("Connected from %1\n")
      .arg( socket->peerAddress().toString() ) );
  map.insert( socket, ModbusTcpServerThreadItem() );
  connect( socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
  connect( socket, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
}

//=============================================================================
/// Чтение новых данных из сокета
//=============================================================================
void ModbusTcpServerThread::readyRead()
{
  QTcpSocket *socket = (QTcpSocket*) qobject_cast<QTcpSocket*>( sender() );
  if( !socket ) return;

  int j;

  ModbusTcpServerThreadItem &item = map[socket];
  QByteArray &tcp_req = item.req;
  QByteArray &tcp_ans = item.ans;

  item.timeout_timer=0; // сброс счетчика таймаута неактивности

  tcp_req.append( socket->readAll() );

  //Console::Print( "###:" + QByteArray2QString( req ) + "\n" );

  if( tcp_req.size() < 7 ) return;

  if( tcp_req.size() > 300 || tcp_req[2] || tcp_req[3] || tcp_req[4] )
  { map.remove( socket );
    socket->deleteLater();
    return;
  }

  int len = (unsigned char)tcp_req[5];

  if( tcp_req.size() == len + 6 )
  {
    Console::Print( Console::ModbusPacket, "TCP Req:" + QByteArray2QString( tcp_req ) + "\n" );

    QByteArray mb_ans;
    QByteArray mb_req = tcp_req.mid(6,len);
    CRC::appendCRC16( mb_req );
    Console::Print( Console::ModbusPacket, "MB  Req:" + QByteArray2QString( mb_req ) + "\n" );


    j = expected_answer_size( mb_req );
    mb_ans.resize( j );
    j = sp->query( mb_req, mb_ans, 0 );
    mb_ans.resize( j );

    Console::Print( Console::ModbusPacket, "MB  Ans:" + QByteArray2QString( mb_ans ) + "\n" );
    mb_ans.chop(2);

    tcp_ans.resize(0);
    tcp_ans.append( tcp_req[0] );
    tcp_ans.append( tcp_req[1] );
    tcp_ans.append( (char) 0 );
    tcp_ans.append( (char) 0 );
    tcp_ans.append( (char) 0 );
    tcp_ans.append( (char) mb_ans.size() );
    tcp_ans.append( mb_ans );

    Console::Print( Console::ModbusPacket, "TCP Ans:" + QByteArray2QString( tcp_ans ) + "\n\n" );

    int j = socket->write( tcp_ans );
    if( j != tcp_ans.size() )
    { Console::Print(  Console::Error, "!:\n" );
    }
    socket->flush();

    tcp_req.resize(0);

  } else
  { Console::Print( Console::ModbusPacket, "################3\n" );
    delete socket;
    return;
  }
}

//=============================================================================
/// Таймер проверки таймаута неактивности клиента
//=============================================================================
void ModbusTcpServerThread::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  QMutableMapIterator< QTcpSocket*, ModbusTcpServerThreadItem  > i(map);
  while (i.hasNext())
  { i.next();
    QTcpSocket *socket = (QTcpSocket*)i.key();
    ModbusTcpServerThreadItem &item = i.value();

    item.timeout_timer++;

    if( item.timeout_timer > 60 ) // 1 минута
    { Console::Print( Console::Information, "Host: "
                        + socket->peerAddress().toString() + " - time out.\n");
      item.timeout_timer=0;
      socket->disconnectFromHost();
    }
  }
}

//=============================================================================
/// Отсоедиение сокета
//=============================================================================
void ModbusTcpServerThread::disconnected()
{
  QTcpSocket *socket = (QTcpSocket*)qobject_cast<QTcpSocket*>( sender() );
  if( !socket ) return;

  Console::Print(  Console::Information, QString("Disconnected %1\n")
  .arg( socket->peerAddress().toString() ) );

  map.remove( socket );
  socket->deleteLater();
}

//#############################################################################
//
//#############################################################################
ModbusTcpServer::ModbusTcpServer( QObject *parent, AbstractSerialPort *sp,  int mb_tcp_port )
  : QObject( parent ), sp( sp ), mb_tcp_port( mb_tcp_port )
{
  sp_thread = new ModbusTcpServerThread;

  sp_thread->sp = sp;
  sp_thread->mb_tcp_port = mb_tcp_port;
  sp_thread->moveToThread( sp_thread );
  sp_thread->start();
}

//=============================================================================
//
//=============================================================================
ModbusTcpServer::~ModbusTcpServer()
{
  sp_thread->exit();
  bool ok = sp_thread->wait(5000);

  if( !ok )
  { sp_thread->terminate();
    sp_thread->wait(5000);
  }
  Console::Print( Console::Information, QString("Server stoped on port %1.\n").arg(mb_tcp_port) );
  delete sp_thread;
}

