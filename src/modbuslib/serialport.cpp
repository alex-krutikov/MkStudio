#include <QtCore>

#include "mbcommon.h"
#include "console.h"

#include "serialport.h"
#include "serialport_p.h"

//###################################################################
//
//###################################################################
SerialPort::SerialPort()
  : d( new SerialPortPrivate(this) )
{
  answer_timeout         = 100;
  current_answer_timeout = -1;
}

//===================================================================
//
//===================================================================
SerialPort::~SerialPort()
{
  delete d;
}


//===================================================================
//!  ������ ��� �����
//
//! \param portname ��� �����
//===================================================================
void SerialPort::setName( const QString &portname )
{
  d->close();
  this->portname=portname;
}
//===================================================================
//!  ������ �������� �������� ������
//
//! \param speed ��������
//===================================================================
void SerialPort::setSpeed( const int speed )
{
  d->close();
  this->portspeed=speed;
}

//===================================================================
//!  ������� ����
//===================================================================
bool SerialPort::open()
{
  return d->open();
}

//===================================================================
//!  ������� ����
//===================================================================
void SerialPort::close()
{
  d->close();
}

//==============================================================================
//!  ����� �������������� � ������� ���������������� ������
//
//! \return ������ �������������� � ������� ���������������� ������
//! � �������: [���];[��������]
//==============================================================================
QStringList SerialPort::queryComPorts()
{
  return SerialPortPrivate::queryComPorts();
}


//===================================================================
//!  ��������� ������-�����
//
//! \par ���������� 1.
//! ����� ��������� ������� \a answer ������ ��������� �
//! ��������� ����������� ���� � ������.
//! \par ���������� 2.
//! ������� �� �������� ����� ��������� ������� \a answer.
//
//! \param[in]  request ������
//! \param[out] answer  �����.
//! \param[out] errorcode ��� ������ � ������ ������ � �������
//! \return �������� ���������� ���� � ������. ���� ����� 0 - ��� ������.
//===================================================================
int SerialPort::query( const QByteArray &request, QByteArray &answer,
                         int *errorcode)
{
  return d->query( request, answer,errorcode );
}

//===================================================================
//! ������� ���������� ���� ������
//===================================================================
void SerialPort::resetLastErrorType()
{
  d->last_error_id = 0;
}
