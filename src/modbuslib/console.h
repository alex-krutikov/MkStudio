#ifndef __CONSOLE_H__

/*!
  \file console.h
  \brief Консоль

  Механизм для отображения отладочных сообщений. Вывод сообщений
  осуществляется из любого места программы при помощи
  статического метода Console::Print.

*/

#include "mbl_global.h"

#include <QString>

class ConsolePrivate;

//=============================================================================
/// Класс для поддержки отладочных сообщений \ingroup Private_group
//=============================================================================
class Console
{
public:
  /// Типы сообщений
  enum MessageType
    { AllTypes          =  -1, ///< Любой тип
      Error             =   1, ///< Ошибка
      Warning           =   2, ///< Предупреждение
      Information       =   4, ///< Информацтонное сообщение
      Debug             =   8, ///< Отладочное сообщение
      ModbusPacket      =  16, ///< Пакет Modbus
      ModbusError       =  32, ///< Ошибка связи (Modbus)
    };
  static MBL_EXPORT void Print( MessageType mtype, const QString &message );
  static MBL_EXPORT QString takeMessage();
  static MBL_EXPORT void setMessageTypes( int type, bool status = true );
  static MBL_EXPORT int messageTypes();
  static MBL_EXPORT void setTimestampShow(bool show);
};

#endif
