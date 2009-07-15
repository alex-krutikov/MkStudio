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
  int packet_counter;
};

//=============================================================================
//
//=============================================================================
MbTcpPort::MbTcpPort()
  : d ( new MbTcpPortPrivate )
{
  d->socket=0;
  d->packet_counter=0;
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
  Q_UNUSED( speed );
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
  if( !d->socket ) return;
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
  int j;
  QByteArray tcp_header,tcp_ans;

  if( !d->socket )
  { QString host = d->portname.section(':',0,0);
    int port = d->portname.section(':',1,1).toInt();
    if( !port ) port=502; // default TCP Modbus port;
    d->socket = new QTcpSocket;
    d->socket->connectToHost( host, port );
    bool ok = d->socket->waitForConnected(4000);
    if( !ok )
    { Console::Print( Console::Error, "MB_TCP: Error: Unable to connect to host `"+d->portname+"`.\n" );
    }
  }

  if( d->socket->state() != QAbstractSocket::ConnectedState )
  {
    delete d->socket;
    d->socket = 0;
    return 0;
  }

  if( errorcode ) *errorcode = 0;
  if( d->socket->bytesAvailable() )
  { if( errorcode ) *errorcode = 56;
    return 0;
  }

  //-------------------------------------------
  d->packet_counter++;
  QByteArray tcp_req = request;
  tcp_req.chop(2);

  tcp_header.resize(0);
  tcp_header.append( (char)(d->packet_counter)    );
  tcp_header.append( (char)(d->packet_counter>>8) );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)0 );
  tcp_header.append( (char)tcp_req.size() );

  Console::Print( Console::ModbusPacket, "\n" );
  Console::Print( Console::ModbusPacket, "MB_TCP: tcp_header  = " + QByteArray2QString( tcp_header ) +"\n" );
  Console::Print( Console::ModbusPacket, "MB_TCP: tcp_request = " + QByteArray2QString( tcp_req ) +"\n" );

  d->socket->write( tcp_header  );
  d->socket->write( tcp_req );
  d->socket->flush();

  tcp_ans.resize(0);

  while(1)
  { if( d->socket->state() != QAbstractSocket::ConnectedState )
    { open();
      Console::Print( Console::Error, "MB_TCP: Error: Socket state is not `Connected`\n" );
      return 0;
    }
    d->socket->waitForReadyRead(1000);
    tcp_ans.append( d->socket->readAll() );
    if( tcp_ans.size() < 6 ) continue;
    if( tcp_ans[2] || tcp_ans[3] || tcp_ans[4] ) goto error;
    if( tcp_ans[0] != tcp_header[0] ) goto error;
    if( tcp_ans[1] != tcp_header[1] ) goto error;
    if( ( (int)(unsigned char)tcp_ans[5] + 6 ) == tcp_ans.size() ) break;
    goto error;
  }

  Console::Print( Console::ModbusPacket, "MB_TCP:  tcp_answer = " + QByteArray2QString( tcp_ans ) +"\n" );

  tcp_ans.remove(0,6);
  if( tcp_ans.size() ) CRC::appendCRC16( tcp_ans );
  j = qMin( tcp_ans.size(), answer.size() );
  tcp_ans.resize( answer.size() );
  answer = tcp_ans;
  Console::Print( Console::ModbusPacket, "MB_TCP:   mb_answer = " + QByteArray2QString( answer.left(j) ) +"\n" );
  return j;
error:
  Console::Print( Console::Error, "MB_TCP: Error: Wrong tcp anser: " + QByteArray2QString( tcp_ans ) +"\n" );
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

