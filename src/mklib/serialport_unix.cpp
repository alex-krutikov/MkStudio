#include <QtCore>

#include "serialport_p.h"

#include "consolewidget.h"
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

  tcgetattr( fd, &ts );
  cfmakeraw( &ts );
  cfsetispeed( &ts, br );
  cfsetospeed( &ts, br );
  ts.c_oflag |= CLOCAL;
  ts.c_oflag |= CREAD;
  ts.c_cc[VTIME] = 2	; // 200 ms total timeout
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
  { ::close( fd );
    fd = 0;
  }
}

//===================================================================
//
//===================================================================
int SerialPortPrivate::request( const QByteArray &request, 
                                      QByteArray &answer, int *errorcode)
{
  Q_UNUSED( errorcode );
  
  int i,ret;
  int request_size = request.size();
  int answer_size  = answer.size();
  
  if( fd == 0 ) return 0;

  if( sp->console_out_packets )
  { CONSOLE_OUT( "MODBUS: Request: "+QByteArray2QString( request )+"\n");
  }
  
  //usleep( 1000*((50000/sp->portspeed)+2) );
  
  tcflush( fd, TCIOFLUSH );
  
  i=0;  
  while(1)
  {  ret = write( fd, request.data()+i, request_size-i );
     if( ret < 0 )
     { CONSOLE_OUT("MODBUS: ERROR: Can't write to port.\n");
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
     { CONSOLE_OUT("MODBUS: ERROR: Can't read from port.\n");
       return 0;
     }
     i += ret;
     if( i == answer_size ) break;
     if( ret == 0 ) break;
  }
  
  if( i != answer_size )
  { CONSOLE_OUT( QString("MODBUS: ERROR: Wrong answer length. (expected=%1, real=%2) ")
                       .arg( answer_size ).arg( i ))
    QByteArray ba = answer;
    ba.resize( i );
    CONSOLE_OUT( " Answer: "+QByteArray2QString( ba )+"\n");
  }
  
  if( sp->console_out_packets )
  { CONSOLE_OUT( "MODBUS:  Answer: "+QByteArray2QString( answer )+"\n");
  }
  return i;
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
  
  foreach( QString str, dir.entryList(QDir::System, QDir::Name) )
  { if( !rx.exactMatch( str ) ) continue;
    str = "/dev/" + str;
    fd = ::open( str.toLocal8Bit(), O_RDONLY|O_NOCTTY );
    if( fd < 0 ) continue;
    ::close(fd);
    sl << str + QString("; Serial Port");
  }
  return sl;
}
