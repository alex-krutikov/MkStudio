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
static void process( const QByteArray &req, QByteArray &ans )
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
      //memcpy( &ans[4], &base[addr], len );
      ans_len = len + 6;
     break;
    case( 0x01 ): // set
      ans[5] = len;
      for(i=0; i<len; i++) base[addr+i] = req[6+i];
      Console::Print( "###:" + QByteArray2QString( req ) + "\n" );
      Console::Print( QString::number( addr )  + "\n" );
      Console::Print( QString::number( req[3] )  + "\n" );
      Console::Print( QString::number( req[4] )  + "\n" );
      Console::Print( QString::number( req[5] )  + "\n" );

      //memcpy( &base[addr], &req[6], len );
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

//=============================================================================
//
//=============================================================================
typedef struct Item
{
  QByteArray req,ans;
};
QMap< QTcpSocket*, Item  > map;

//#############################################################################
//
//#############################################################################
void ModbusTcpServerThread::run()
{
  //Console::Print( QString("SP thread id=%1\n").arg( (int) currentThreadId() ) );

  server = new QTcpServer(this);
  server->listen( QHostAddress::Any, 502 );
  connect( server, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );
  exec();
}

//=============================================================================
//
//=============================================================================
void ModbusTcpServerThread::newConnection()
{
  Console::Print( QString("New connection\n") );
  QTcpSocket *socket = server->nextPendingConnection();
  map.insert( socket, Item() );
  connect( socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
  connect( socket, SIGNAL( disconnected() ), this, SLOT( disconnected() ) );
}

//=============================================================================
//
//=============================================================================
void ModbusTcpServerThread::readyRead()
{
  QTcpSocket *socket = (QTcpSocket*) qobject_cast<QTcpSocket*>( sender() );

  int j;

  Item &item = map[socket];
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
    // Console::Print( "R:" + QByteArray2QString( ba_req ) + "\n" );

    j = zzz( ba_req );
    ba_ans.resize( j );
    j = sp->query( ba_req, ba_ans, 0 );
    ba_ans.resize( j );
    // process( ba_req, ba_ans );

    ans.resize(0);
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) 0 );
    ans.append( (char) ba_ans.size() );
    ans.append( ba_ans );

    // Console::Print( "A:" + QByteArray2QString( ba_ans ) + "\n" );

    int j = socket->write( ans );
    if( j != ans.size() )
    { Console::Print( "!:\n" );
    }
    socket->flush();

  } else
  { delete socket;
    return;
  }

  //Console::Print( socket->readAll() );
}

//=============================================================================
//
//=============================================================================
void ModbusTcpServerThread::disconnected()
{
  Console::Print( QString("Disconnected\n") );
  QTcpSocket *socket = server->nextPendingConnection();

  map.remove( socket );
  delete socket;
}

//#############################################################################
//
//#############################################################################
ModbusTcpServer::ModbusTcpServer( QObject *parent, AbstractSerialPort *sp )
  : QObject( parent ), sp( sp )
{
  sp_thread.sp = sp;
  sp_thread.moveToThread( &sp_thread );
  sp_thread.start();
}

//=============================================================================
//
//=============================================================================
ModbusTcpServer::~ModbusTcpServer()
{
  sp_thread.exit();
  sp_thread.wait(5000);
}

