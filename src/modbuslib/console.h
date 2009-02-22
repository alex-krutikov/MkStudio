#ifndef __CONSOLE_H__

/*!
  \file console.h
  \brief �������

  �������� ��� ����������� ���������� ���������. ����� ���������
  �������������� �� ������ ����� ��������� ��� ������
  ������������ ������ Console::Print.

*/

#include <QString>

//=============================================================================
/// ����� ��� ��������� ���������� ��������� \ingroup Private_group
//=============================================================================
class ConsolePrivate;

class Console
{
public:
  static void Print( const QString &message );
  static QString takeMessage();
};

#endif
