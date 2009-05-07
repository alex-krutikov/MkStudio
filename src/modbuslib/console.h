#ifndef __CONSOLE_H__

/*!
  \file console.h
  \brief �������

  �������� ��� ����������� ���������� ���������. ����� ���������
  �������������� �� ������ ����� ��������� ��� ������
  ������������ ������ Console::Print.

*/

#include <QString>

#include "mbl_global.h"

class ConsolePrivate;

//=============================================================================
/// ����� ��� ��������� ���������� ��������� \ingroup Private_group
//=============================================================================
class Console
{
public:
  /// ���� ���������
  enum MessageType
    { AllTypes          =  -1, ///< ����� ���
      Error             =   1, ///< ������
      Warning           =   2, ///< ��������������
      Information       =   4, ///< �������������� ���������
      Debug             =   8, ///< ���������� ���������
      ModbusPacket      =  16, ///< ����� Modbus
      ModbusError       =  32, ///< ������ ����� (Modbus)
    };
  static MBL_EXPORT void Print( MessageType mtype, const QString &message );
  static MBL_EXPORT QString takeMessage();
  static MBL_EXPORT void setMessageTypes( int type, bool status = true );
  static MBL_EXPORT int messageTypes();
};

#endif
