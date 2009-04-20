#include <QtCore>
#include <QtNetwork>

#include "server.h"
#include "abstractserialport.h"
#include "crc.h"

#include "console.h"

//==============================================================================
//
//==============================================================================
static int zzz( const QByteArray &ba )
{
  int len = ba.size();
  if( len < 3 ) return 255;
  if(     ( ba[1] == 0x41 )
       || ( ba[1] == 0x43 ) )
  { int dsize = ba[5];
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
    { case( 33 ): return ba.size()+2;                      // create/open file
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

//==============================================================================
// массив ==> строка шестнадцатеричных чисел
//==============================================================================
static QString QByteArray2QString( const QByteArray &ba, int mode = 1 )
{
	int i, len = ba.size();
	const char* p = ba.data();
	QString s;
	QString str;
	if( mode == 0 )
	{ str = QString("(length=%1) ").arg( ba.size(),4 );
	}

	for( i=0; i<len; i++ )
	{ s.sprintf(" %2.2X", *(unsigned char*)(p+i) );
		str = str + s;
	}
	return str;
}


//==============================================================================
//
//==============================================================================
void emulator( const QByteArray &req, QByteArray &ans )
{
  static unsigned char base[100000];

  ans.resize( 300 );

  int addr = ( (unsigned char)req[3] << 8 ) | (unsigned char)req[4];
  int len  = (unsigned char)req[5];
  int ans_len = 0;
  int i;

  ans[0] = req[0];
  ans[1] = req[1];
  ans[2] = req[2];
  ans[3] = req[3];
  ans[4] = req[4];

  switch( req[2] & 0x0F )
  {
    case( 0x00 ): // read
      ans[3] = len;
      for(i=0; i<len; i++) ans[4+i] = base[addr+i];
      ans_len = len + 6;
     break;
    case( 0x01 ): // set
      ans[5] = len;
      for(i=0; i<len; i++) base[addr+i] = req[6+i];
      ans_len = len + 8;
      break;
    case( 0x03 ): // and
      ans[5] = len;
      for(i=0; i<len; i++) base[addr+i] &= req[6+i];
      ans_len = len + 8;
      break;
    case( 0x05 ): // or
      ans[5] = len;
      for(i=0; i<len; i++) base[addr+i] |= req[6+i];
      ans_len = len + 8;
      break;
    case( 0x07 ): // xor
      ans[5] = len;
      for(i=0; i<len; i++) base[addr+i] ^= req[6+i];
      ans_len = len + 8;
    break;
  }

  if( ans_len > 2)
  { ans.resize( ans_len - 2 );
    CRC::appendCRC16( ans );
  }

  ans.resize( ans_len );
}

//#############################################################################
//
//#############################################################################
void ModbusTcpServerThread::run()
{
  bool ok;

  server = new QTcpServer(this);
  ok = server->listen( QHostAddress::Any, mb_tcp_port );

  QString message = ok ? QString("Server started on port %1.\n").arg(mb_tcp_port)
                       : QString("Unable to listen port %1.\n").arg(mb_tcp_port);
  Console::Print( Console::Information, message );

  connect( server, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );
  exec();
/*
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
*/
  delete server;
}

//=============================================================================
//
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
//
//=============================================================================
void ModbusTcpServerThread::readyRead()
{
  QTcpSocket *socket = (QTcpSocket*) qobject_cast<QTcpSocket*>( sender() );
  if( !socket ) return;

  int j;

  ModbusTcpServerThreadItem &item = map[socket];
  QByteArray &req = item.req;
  QByteArray &ans = item.ans;


  req.append( socket->readAll() );

  //Console::Print( "###:" + QByteArray2QString( req ) + "\n" );

  if( req.size() < 7 ) return;

  if( req.size() > 300 || req[2] || req[3] || req[4] )
  { map.remove( socket );
    delete socket;
    return;
  }

  int len = (unsigned char)req[5];

  if( req.size() == len + 6 )
  {
    QByteArray ba_ans;
    QByteArray ba_req = req.mid(6,len);
    req.remove(0,len+6);
     Console::Print( Console::ModbusPacket, "R:" + QByteArray2QString( ba_req ) + "\n" );


    j = zzz( ba_req );
    ba_ans.resize( j );
    j = sp->query( ba_req, ba_ans, 0 );
    ba_ans.resize( j );
    //emulator( ba_req, ba_ans );

    ans.resize(0);
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) ba_ans.size() );
    ans.append( ba_ans );

     Console::Print( Console::ModbusPacket, "A:" + QByteArray2QString( ba_ans ) + "\n" );

    int j = socket->write( ans );
    if( j != ans.size() )
    { Console::Print(  Console::Error, "!:\n" );
    }
    socket->flush();

  } else
  { delete socket;
    return;
  }
}

//=============================================================================
//
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

