#include <QtCore>

#include "serialport_p.h"

#include "console.h"
#include "serialport.h"
#include "mbcommon.h"

#include <termios.h>
#include <fcntl.h>

//===================================================================
//
//===================================================================
SerialPortPrivate::SerialPortPrivate( SerialPort *sp_arg )
  : sp( sp_arg )
{
  fd = 0;
}

//===================================================================
//
//===================================================================
SerialPortPrivate::~SerialPortPrivate()
{
  close();
}

//===================================================================
//
//===================================================================
bool SerialPortPrivate::open()
{
  QMutexLocker mutex_locker( &mutex );
  
  if( fd ) return true;

  struct termios ts;
  fd = ::open( sp->portname.toLocal8Bit() , O_RDWR|O_NOCTTY );
  if( fd < 0 )
  { return false;
  }

  speed_t br;
  switch( sp->portspeed )
  {case(50):     br=B50;     break;
   case(75):     br=B75;     break;
   case(110):    br=B110;    break;
   case(134):    br=B134;    break;
   case(150):    br=B150;    break;
   case(200):    br=B200;    break;
   case(300):    br=B300;    break;
   case(600):    br=B600;    break;
   case(1200):   br=B1200;   break;
   case(1800):   br=B1800;   break;
   case(2400):   br=B2400;   break;
   case(4800):   br=B4800;   break;
   case(9600):   br=B9600;   break;
   case(19200):  br=B19200;  break;
   case(38400):  br=B38400;  break;
   case(57600):  br=B57600;  break;
   case(115200): br=B115200; break;
   case(230400): br=B230400; break;
   default:      br=B9600;   break;
  }

  if( tcgetattr( fd, &ts ) )
  { return false;
  }
  cfmakeraw( &ts );
  cfsetispeed( &ts, br );
  cfsetospeed( &ts, br );
  ts.c_oflag |= CLOCAL;
  ts.c_oflag |= CREAD;
  ts.c_cc[VTIME] = sp->current_answer_timeout/100;
  ts.c_cc[VMIN]  = 0;
  if( tcsetattr( fd, TCSANOW, &ts ) )
  { return false;
  }
  return true;
}

//===================================================================
//
//===================================================================
bool SerialPortPrivate::reopen()
{
  return open();
}

//===================================================================
//
//===================================================================
void SerialPortPrivate::close()
{
  if( fd )
  { QMutexLocker mutex_locker( &mutex );
    ::close( fd );
    fd = 0;
  }
}

//===================================================================
//
//===================================================================
int SerialPortPrivate::query( const QByteArray &request, 
                                      QByteArray &answer, int *errorcode)
{
  Q_UNUSED( errorcode );
  
  int i,ret;
  int request_size = request.size();
  int answer_size  = answer.size();

  if( fd == 0 )
  { usleep(500000);
    open();
    return 0;
  }
  
  QMutexLocker mutex_locker( &mutex );

  if( sp->answer_timeout != sp->current_answer_timeout )
  { sp->current_answer_timeout = sp->answer_timeout;

    struct termios ts;
    if( tcgetattr( fd, &ts ) ) return 0;
    ts.c_cc[VTIME] = sp->current_answer_timeout/100;
    ts.c_cc[VMIN]  = 0;
    if( tcsetattr( fd, TCSANOW, &ts ) ) return 0;
  }

  Console::Print( Console::ModbusPacket, "MODBUS: Request: "+QByteArray2QString( request )+"\n");

  usleep( 80000000/sp->portspeed ); // 8*10 bit time delay
  
  tcflush( fd, TCIOFLUSH );
  
  i=0;  
  while(1)
  {  ret = write( fd, request.data()+i, request_size-i );
     if( ret < 0 )
     { Console::Print( Console::Error, "MODBUS: ERROR: Can't write to port.\n");
       return 0;
     }
     i += ret;
     if( i == request_size ) break;
     if( ret == 0 ) break;
  }

  i=0;
  while(1)
  {  ret = read( fd, answer.data()+i, answer_size-i );
     if( ret < 0 )
     { Console::Print( Console::Error, "MODBUS: ERROR: Can't read from port.\n");
       return 0;
     }
     i += ret;
     if( i == answer_size ) break;
     if( ret == 0 ) break;
  }
  
  if( i != answer_size )
  { Console::Print(  Console::Error, QString("MODBUS: ERROR: Wrong answer length. (expected=%1, real=%2) ")
                       .arg( answer_size ).arg( i ));
    QByteArray ba = answer;
    ba.resize( i );
    Console::Print(  Console::Error, " Answer: "+QByteArray2QString( ba )+"\n");
  }

  Console::Print( Console::ModbusPacket, "MODBUS:  Answer: "+QByteArray2QString( answer )+"\n");

  return i;
}

//===================================================================
// Посылка запроса и получение ответа (протокол XBee)
//===================================================================
int SerialPortPrivate::queryXBee( const QByteArray &request, QByteArray &answer,
                                       int *errorcode, int xbee_addr )
{
  Q_UNUSED( errorcode );

  unsigned char crc;
  int ret,i,a;

  if( fd == 0 )
  { usleep(500000);
    open();
    return 0;
  }
  
  QMutexLocker mutex_locker( &mutex );

  if( sp->answer_timeout != sp->current_answer_timeout )
  { sp->current_answer_timeout = sp->answer_timeout;

    struct termios ts;
    if( tcgetattr( fd, &ts ) ) return 0;
    ts.c_cc[VTIME] = sp->current_answer_timeout/100;
    ts.c_cc[VMIN]  = 0;
    if( tcsetattr( fd, TCSANOW, &ts ) ) return 0;
  }

  // подготовка данных

  QByteArray ba;
  ba.resize(8);
  ba[0] = 0x7E;
  ba[3] = 0x01; // API identifier
  ba[4] = 0x00; // Frame ID
  ba[5] = xbee_addr >> 8;
  ba[6] = xbee_addr;
  ba[7] = 0; // Options
  ba.append( request ); // Data

  a = ba.size()-3;
  ba[1] = a >> 8;
  ba[2] = a;

  a = ba.size();
  crc=0;
  for( i=3; i<a; i++ ) crc += ba[i];
  crc=0xFF-crc;
  ba.append( crc ); // CRC

  Console::Print( Console::ModbusPacket, "XBEE: Request: "+QByteArray2QString( ba )+"\n");


  usleep((8 * 10 * 1000000 / sp->speed()));

  tcflush( fd, TCIOFLUSH );

  i=0;  
  while(1)
  {  ret = write( fd, ba.data()+i, ba.size()-i );
     if( ret < 0 )
     { Console::Print( Console::Error, "XBEE: ERROR: Can't write to port.\n");
       return 0;
     }
     i += ret;
     if( i == ba.size() ) break;
     if( ret == 0 ) break;
  }

  ba.resize( 3 ); // первые три байта ответа

  i=0;
  while(1)
  {  ret = read( fd, ba.data()+i, ba.size()-i );
     if( ret < 0 )
     { Console::Print( Console::Error, "XBEE: ERROR: Can't read from port.\n");
       return 0;
     }
     i += ret;
     if( i == ba.size() ) break;
     if( ret == 0 ) break;
  }

  // анализ первых байт

  if( i != 3 )
  { Console::Print( Console::Error, "XBEE: ERROR: No answer.\n" );
    return 0;
  }

  if( ba[0] != (char)0x7E )
  { Console::Print( Console::Error, "XBEE: ERROR: Receive packet's first byte is not 0x7E.\n" );
    return 0;
  }

  // дочтение остальных данных

  a = ( (unsigned char)ba[1] << 8 ) | (unsigned char)ba[2];
  ba.resize( ba.size() + a + 1 );

  while(1)
  {  ret = read( fd, ba.data()+i, ba.size()-i );
     if( ret < 0 )
     { Console::Print( Console::Error, "XBEE: ERROR: Can't read from port.\n");
       return 0;
     }
     i += ret;
     if( i == ba.size() ) break;
     if( ret == 0 ) break;
  }

  // анализ XBee CRC

  a = ba.size();
  crc=0;
  for( i=3; i<a; i++ ) crc += ba[i];
  if( crc != 0xFF )
  { Console::Print( Console::Error, "XBEE: ERROR: XBee CRC error.\n" );
    return 0;
  }

  // подготовка ответа

  Console::Print( Console::ModbusPacket, "XBEE: Answer:  "+QByteArray2QString( ba )+"\n");
  ba = ba.mid( 8, a-9 );

  answer.replace(0, ba.size(), ba );
  return ba.size();
}

//===================================================================
//
//===================================================================
QStringList SerialPortPrivate::queryComPorts()
{
  QRegExp rx("^tty[A-Z].*");
  QStringList sl;
  QDir dir("/dev");
  int fd;
  struct termios ts;
  
  foreach( QString str, dir.entryList(QDir::System, QDir::Name) )
  { if( !rx.exactMatch( str ) ) continue;
    str = "/dev/" + str;
    fd = ::open( str.toLocal8Bit(), O_RDWR|O_NOCTTY );
    if( fd < 0 ) continue;
    if( tcgetattr( fd, &ts ) == 0 )
    { sl << str + ";Serial Port";
    }
    ::close(fd);
  }
  return sl;
}
