#ifndef __CONSOLE_H__

/*!
  \file console.h
  \brief Консоль

  Механизм для отображения отладочных сообщений. Вывод сообщений
  осуществляется из любого места программы при помощи
  статического метода Console::Print.

*/

#include <QString>

//=============================================================================
/// Класс для поддержки отладочных сообщений \ingroup Private_group
//=============================================================================
class ConsolePrivate;

class Console
{
public:
  static void Print( const QString &message );
  static QString takeMessage();
};

#endif
