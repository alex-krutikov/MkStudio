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
  current_answer_timeout = answer_timeout;
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
//!
//===================================================================
void SerialPort::clearXBeeRouteTable()
{
  d->xbee_route_table.clear();
}

//===================================================================
//!
//===================================================================
void SerialPort::addXBeeRoute( int a1, int a2, int addr )
{
  st_XBeeRoute st;
  st.a1   = a1;
  st.a2   = a2;
  st.addr = addr;
  d->xbee_route_table << st;
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
  if( !request.size() ) return 0;

  bool xbee_mode = false;
  int xbee_addr = 1;
  int n = d->xbee_route_table.size();
  int a = (unsigned char)request[0];
  for( int i=0; i<n; i++ )
  { const st_XBeeRoute &st = d->xbee_route_table.at(i);
    if( ( a >= st.a1 ) && ( a <= st.a2 ) )
    { xbee_addr = st.addr;
      xbee_mode = true;
      break;
    }
  }

  int ret = 0;
  if( xbee_mode )
  { ret = d->queryXBee( request, answer,errorcode, xbee_addr );
  } else
  { ret = d->query( request, answer,errorcode );
  }
  return ret;
}

//===================================================================
//! ������� ���������� ���� ������
//===================================================================
void SerialPort::resetLastErrorType()
{
  d->last_error_id = 0;
}
