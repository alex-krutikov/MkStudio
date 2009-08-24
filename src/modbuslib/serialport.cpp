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
//! Очистка последнего типа ошибки
//===================================================================
void SerialPort::resetLastErrorType()
{
  d->last_error_id = 0;
}
