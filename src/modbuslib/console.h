#ifndef __CONSOLE_H__

/*!
  \file console.h
  \brief  онсоль

  ћеханизм дл€ отображени€ отладочных сообщений. ¬ывод сообщений
  осуществл€етс€ из любого места программы при помощи
  статического метода Console::Print.

*/

#include <QString>

//=============================================================================
///  ласс дл€ отображени€ отладочных сообщений
//=============================================================================
class ConsolePrivate;

class Console
{
public:
  static void Print( const QString &message );
  static QString takeMessage();
};

#endif
