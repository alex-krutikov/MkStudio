#include "console.h"

#include <QMutex>

static QMutex mutex;
static QString message;
static int message_types;


//=============================================================================
/// Напечатать сообщение
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
/// Возвратить все сообщения и очистить буфер
//=============================================================================
QString Console::takeMessage()
{
  QMutexLocker locker( &mutex );
  QString ret = message;
  message.clear();
  return ret;
}

//=============================================================================
/// Установить типы отображаемых сообщений
//
/// \param type   тип сообщения
/// \param status true - установить отображение, false - убрать отображение
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
/// Маска типов отображаемых сообщений
//
/// \return маска типов отображаемых сообщений
//=============================================================================
int  Console::messageTypes()
{
  return message_types;
}

