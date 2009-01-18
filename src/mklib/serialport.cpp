#include <QtGui>

#include "consolewidget.h"
#include "mbcommon.h"

#include "serialport.h"
#include "serialport_p.h"

//###################################################################
//
//###################################################################
SerialPort::SerialPort()
  : d( new SerialPortPrivate(this) )
{
  console_out_packets = false;
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
//
//===================================================================
void SerialPort::setName( const QString &portname_arg )
{
  d->close();
  portname=portname_arg;
}
//===================================================================
//
//===================================================================
void SerialPort::setSpeed( const int speed_arg )
{
  d->close();
  portspeed=speed_arg;
}

//===================================================================
//
//===================================================================
bool SerialPort::open()
{
  return d->open();
}

//===================================================================
//
//===================================================================
void SerialPort::close()
{
  d->close();
}

//==============================================================================
// ����� �������������� � ������� COM ������
//==============================================================================
QStringList SerialPort::queryComPorts()
{
  return SerialPortPrivate::queryComPorts();
}


//===================================================================
//
//===================================================================
int SerialPort::request( const QByteArray &request, QByteArray &answer,
                         int *errorcode)
{
  return d->request( request, answer,errorcode );
}

//===================================================================
//
//===================================================================
void SerialPort::reset_current_error()
{
  d->last_error_id = 0;
}
