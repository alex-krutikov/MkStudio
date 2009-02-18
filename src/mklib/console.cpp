#include <QtCore>

#include "console.h"

static QMutex mutex;
static QString message;


//=============================================================================
/// Напечатать сообщение
//=============================================================================
void Console::Print( const QString &message )
{
  QMutexLocker locker( &mutex );
  ::message += message + "\n";
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
