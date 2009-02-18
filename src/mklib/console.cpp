#include <QtCore>

#include "console.h"

static QMutex mutex;
static QString message;


//=============================================================================
/// ���������� ���������
//=============================================================================
void Console::Print( const QString &message )
{
  QMutexLocker locker( &mutex );
  ::message += message + "\n";
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
