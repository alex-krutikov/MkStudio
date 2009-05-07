#include <QtCore>

#include "console.h"

static QMutex mutex;
static QString message;
static int message_types;


//=============================================================================
/// ���������� ���������
//=============================================================================
void Console::Print( MessageType mtype, const QString &message )
{
  QMutexLocker locker( &mutex );
  if( mtype & message_types )
  { ::message += message;
    if( ::message.length() > 16384 ) ::message.remove(0,4096);
  }
}

//=============================================================================
/// ���������� ��� ��������� � �������� �����
//=============================================================================
QString Console::takeMessage()
{
  QMutexLocker locker( &mutex );
  QString ret = message;
  message.clear();
  return ret;
}

//=============================================================================
/// ���������� ���� ������������ ���������
//
/// \param type   ��� ���������
/// \param status true - ���������� �����������, false - ������ �����������
//=============================================================================
void Console::setMessageTypes( int type, bool status )
{
  QMutexLocker locker( &mutex );
  if( status )
  { // set
    message_types |= type;
  } else
  { // clear
    message_types &= ~type;
  }
}

//=============================================================================
/// ����� ����� ������������ ���������
//
/// \return ����� ����� ������������ ���������
//=============================================================================
int  Console::messageTypes()
{
  return message_types;
}

