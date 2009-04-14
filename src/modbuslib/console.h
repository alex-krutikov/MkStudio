#ifndef __CONSOLE_H__

/*!
  \file console.h
  \brief Консоль

  Механизм для отображения отладочных сообщений. Вывод сообщений
  осуществляется из любого места программы при помощи
  статического метода Console::Print.

*/

#include <QString>

#include "mbl_global.h"

//=============================================================================
/// Класс для поддержки отладочных сообщений \ingroup Private_group
//=============================================================================
class ConsolePrivate;

class Console
{
public:
  static MBL_EXPORT void Print( const QString &message );
  static MBL_EXPORT QString takeMessage();
};

#endif
