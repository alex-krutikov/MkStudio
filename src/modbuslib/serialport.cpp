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
  mode                   = ModbusRTU;
  //mode                   = XBee;
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
//!
//
//!
//===================================================================
void SerialPort::setMode( PortMode mode )
{
  this->mode = mode;
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
  d->xbee_post_route_table.clear();
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
//!
//===================================================================
void SerialPort::addXBeePostRoute( int a1, int a2, int id )
{
  st_XBeePostRoute st;
  st.a1   = a1;
  st.a2   = a2;
  st.id   = id;
  d->xbee_post_route_table << st;

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
  int xbee_addr = 1;
  int xbee_id   = 0x7E;
  if( mode == XBee && request.size() )
  { int n = d->xbee_route_table.size();
    int a = request[0];
    for( int i=0;i<n;i++ )
    { const st_XBeeRoute &st = d->xbee_route_table.at(i);
      if( ( a >= st.a1 ) && ( a <= st.a2 ) )
      { xbee_addr = st.addr;
        break;
      }
    }
    n = d->xbee_post_route_table.size();
    for( int i=0;i<n;i++ )
    { const st_XBeePostRoute &st = d->xbee_post_route_table.at(i);
      if( ( a >= st.a1 ) && ( a <= st.a2 ) )
      { xbee_id = st.id;
        break;
      }
    }
  }


  switch( mode )
  { case( ModbusRTU ): return d->query(     request, answer,errorcode );
    case( XBee      ): return d->queryXBee( request, answer,errorcode, xbee_addr, xbee_id );
  }
  return 0;
}

//===================================================================
//! ������� ���������� ���� ������
//===================================================================
void SerialPort::resetLastErrorType()
{
  d->last_error_id = 0;
}
