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

//=============================================================================
/// ����� ��� ��������� ���������� ��������� \ingroup Private_group
//=============================================================================
class ConsolePrivate;

class Console
{
public:
  static MBL_EXPORT void Print( const QString &message );
  static MBL_EXPORT QString takeMessage();
};

#endif
