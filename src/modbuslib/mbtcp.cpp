#include <QtCore>
#include <QtNetwork>

#include "mbtcp.h"
#include "console.h"
#include "mbcommon.h"

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
  QByteArray header,resp;

  if( !d->socket )
  { d->socket = new QTcpSocket;
    d->socket->connectToHost(d->portname,502);
    d->socket->waitForConnected(1000);
  }

  if( d->socket->state() != QAbstractSocket::ConnectedState )
  { delete d->socket;
    d->socket = 0;
    return 0;
  }

  if( errorcode ) *errorcode = 0;
  if( d->socket->bytesAvailable() )
  { if( errorcode ) *errorcode = 56;
    return 0;
  }

  header.append( (char)0 );
  header.append( (char)0 );
  header.append( (char)0 );
  header.append( (char)0 );
  header.append( (char)0 );
  header.append( (char)request.size() );

  d->socket->write( header  );
  d->socket->write( request );

  d->socket->flush();
  d->socket->waitForBytesWritten(5000);

  while(1)
  { if( d->socket->state() != QAbstractSocket::ConnectedState )
    { open();
      return 0;
    }

    d->socket->waitForReadyRead(1000);
    resp.append( d->socket->readAll() );
    if( resp.size() < 6 ) continue;
    if( resp[2] || resp[3] || resp[4] ) goto error;
    if( ( (int)(unsigned char)resp[5] + 6 ) == resp.size() ) break;
    goto error;
  }

  resp.remove(0,6);
  int j = qMin( resp.size(), answer.size() );
  resp.resize( answer.size() );
  answer = resp;
  return j;

error:
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

