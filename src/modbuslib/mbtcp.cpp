#include <QtCore>
#include <QtNetwork>

#include "mbtcp.h"


//=============================================================================
//
//=============================================================================
class MbTcpPortPrivate
{
  friend class MbTcpPort;

  QString portname;
  int answer_timeout;
  QString lastError_str;
};

//=============================================================================
//
//=============================================================================
MbTcpPort::MbTcpPort()
  : d ( new MbTcpPortPrivate )
{
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
QString MbTcpPort::name() { return d->portname; } //!< ��� ����������

//! ������� ������ (� ��)
int MbTcpPort::answerTimeout() const
{ return d->answer_timeout;
}

//=============================================================================
//
//=============================================================================
//! ������ ������� ������ (� ��)
void MbTcpPort::setAnswerTimeout(int timeout )
{ d->answer_timeout = timeout;
}

//=============================================================================
//
//=============================================================================
//! �������� ��������� ������
QString MbTcpPort::lastError() const
{ return d->lastError_str;
}

