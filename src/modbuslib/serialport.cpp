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
//!  Задать имя порта
//
//! \param portname имя порта
//===================================================================
void SerialPort::setName( const QString &portname )
{
  d->close();
  this->portname=portname;
}
//===================================================================
//!  Задать скорость передачи данных
//
//! \param speed скорость
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
//!  Открыть порт
//===================================================================
bool SerialPort::open()
{
  return d->open();
}

//===================================================================
//!  Закрыть порт
//===================================================================
void SerialPort::close()
{
  d->close();
}

//==============================================================================
//!  Поиск присутствующих в системе последовательных портов
//
//! \return Список присутствующих в системе последовательных портов
//! в формате: [имя];[описание]
//==============================================================================
QStringList SerialPort::queryComPorts()
{
  return SerialPortPrivate::queryComPorts();
}


//===================================================================
//!  Транзакия запрос-ответ
//
//! \par ПРИМЕЧАНИЕ 1.
//! Длина байтового массива \a answer должна совпадать с
//! ожидаемым количеством байт в ответе.
//! \par ПРИМЕЧАНИЕ 2.
//! Функция не изменяет длину байтового массива \a answer.
//
//! \param[in]  request запрос
//! \param[out] answer  ответ.
//! \param[out] errorcode код ошибки в пакете ответа с ошибкой
//! \return Реальное количество байт в ответе. Если равно 0 - нет ответа.
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
//! Очистка последнего типа ошибки
//===================================================================
void SerialPort::resetLastErrorType()
{
  d->last_error_id = 0;
}
