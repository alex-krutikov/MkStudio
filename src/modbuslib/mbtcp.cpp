#include <QtCore>
#include <QtNetwork>

#include "mbtcp.h"
#include "console.h"
#include "mbcommon.h"
#include "crc.h"

//=============================================================================
//
//=============================================================================
class MbTcpPortPrivate
{
  friend class MbTcpPort;

  QString portname;
  int answer_timeout;
  QString lastError_str;
  QTcpSocket *socket;
};

//=============================================================================
//
//=============================================================================
MbTcpPort::MbTcpPort()
  : d ( new MbTcpPortPrivate )
{
  d->socket=0;
}

//=============================================================================
//
//=============================================================================
MbTcpPort::~MbTcpPort()
{
  close();
  delete d;
}


//=============================================================================
//
//=============================================================================
void MbTcpPort::setName( const QString &portname )
{
  d->portname = portname;
}

//=============================================================================
//
//=============================================================================
void MbTcpPort::setSpeed( const int speed )
{
}

//=============================================================================
//
//=============================================================================
bool MbTcpPort::open()
{
  return true;
}

//=============================================================================
//
//=============================================================================
void MbTcpPort::close()
{
  d->socket->disconnect();
  d->socket->waitForDisconnected(1000);
  delete d->socket;
  d->socket = 0;
}

//=============================================================================
//
//=============================================================================
void MbTcpPort::resetLastErrorType()
{
}

//=============================================================================
//
//=============================================================================
int MbTcpPort::query( const QByteArray &request, QByteArray &answer, int *errorcode)
{
  Console::setMessageTypes( Console::AllTypes );

  Console::Print( Console::Debug, "1request = " + QByteArray2QString( request ) +"\n" );

  QByteArray tcp_header,tcp_ans;

  if( !d->socket )
  { QString host = d->portname.section(':',0,0);
    int port = d->portname.section(':',1,1).toInt();
    if( !port ) port=502; // default TCP Modbus port;
    d->socket = new QTcpSocket;
    d->socket->connectToHost( host, port );
    d->socket->waitForConnected(5000);
    Console::Print( Console::Debug, "1%%%%%%%%%%%\n" );
  }

  if( d->socket->state() != QAbstractSocket::ConnectedState )
  {
    Console::Print( Console::Debug, "2%%%%%%%%%%%%\n" );
    delete d->socket;
    d->socket = 0;
    return 0;
  }

  if( errorcode ) *errorcode = 0;
  if( d->socket->bytesAvailable() )
  { if( errorcode ) *errorcode = 56;
    Console::Print( Console::Debug, "$$$$$$$$$$$$$$$$$\n" );
    return 0;
  }

  //-------------------------------------------

  QByteArray tcp_req = request;
  tcp_req.chop(2);

  tcp_header.resize(0);
  tcp_header.append( (char)0 );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)tcp_req.size() );

  Console::Print( Console::Debug, "2header = " + QByteArray2QString( tcp_header ) +"\n" );
  Console::Print( Console::Debug, "3tcpreq = " + QByteArray2QString( tcp_req ) +"\n" );

  d->socket->write( tcp_header  );
  d->socket->write( tcp_req );
  d->socket->flush();
  d->socket->waitForBytesWritten(5000);

  tcp_ans.resize(0);

  while(1)
  { if( d->socket->state() != QAbstractSocket::ConnectedState )
    { open();
      Console::Print( Console::Debug, "return 0\n" );
      return 0;
    }
    Console::Print( Console::Debug, "******\n" );
    d->socket->waitForReadyRead(1000);
    tcp_ans.append( d->socket->readAll() );
    if( tcp_ans.size() < 6 ) continue;
    if( tcp_ans[2] || tcp_ans[3] || tcp_ans[4] ) goto error;
    if( ( (int)(unsigned char)tcp_ans[5] + 6 ) == tcp_ans.size() ) break;
    goto error;
  }

  Console::Print( Console::Debug, "4 tcp_ans = " + QByteArray2QString( tcp_ans ) +"\n" );

  tcp_ans.remove(0,6);
  if( tcp_ans.size() ) CRC::appendCRC16( tcp_ans );
  int j = qMin( tcp_ans.size(), answer.size() );
  tcp_ans.resize( answer.size() );
  answer = tcp_ans;
  Console::Print( Console::Debug, "5 mb_ans = " + QByteArray2QString( answer ) +"\n\n" );
  return j;
error:
  Console::Print( Console::Debug, "GOto error\n" );
  close();
  return 0;
}

//=============================================================================
//
//=============================================================================
QString MbTcpPort::name() { return d->portname; } //!< Имя интерфейса

//! Таймаут ответа (в мс)
int MbTcpPort::answerTimeout() const
{ return d->answer_timeout;
}

//=============================================================================
//
//=============================================================================
//! Задать таймаут ответа (в мс)
void MbTcpPort::setAnswerTimeout(int timeout )
{ d->answer_timeout = timeout;
}

//=============================================================================
//
//=============================================================================
//! Описание последней ошибки
QString MbTcpPort::lastError() const
{ return d->lastError_str;
}

