#include <QtCore>

#include "serialport.h"
#include "mbtcp.h"
#include "mbtcpserver.h"

#include "mbcommon.h"
#include "crc.h"

QTextStream cout( stdout );
QTextStream cerr( stderr );

int tcpport=502;
AbstractSerialPort *sp;

struct XBeeRoute
{
  XBeeRoute() {}
  XBeeRoute( int start, int end, int addr )
    : start(start), end(end), addr(addr) {}
  int start;
  int end;
  int addr;
};

struct XBeePostRoute
{
  XBeePostRoute() {}
  XBeePostRoute( int start, int end, int id )
    : start(start), end(end), id(id) {}
  int start;
  int end;
  int id;
};

//=============================================================================
// ����� ���������� � ���������
//=============================================================================
static void print_help()
{
  QString str =
  "\n"
  "\n"
  "usage: mkserverd [options] \n"
  "\n"
  "  Options:\n"
  "    -port=[Serial port name]\n"
  "    -baud=[Serial port baud rate]\n"
  "    -host=[Modbus TCP remote server's host name]\n"
  "    -tcpport=[Modbus TCP port number]\n"
  "    -xbeeroute=[start]:[end]:[addr]\n"
  "    -xbeepostroute=[start]:[end]:[addr]\n"
  "\n\n"
  "  Comments\n\n"
  "    1. \"-host\" option may be used to make the Modbus TCP proxy server.\n\n"
  "    2. Default Modbus TCP port number is 502.\n\n"
  "    3. If \"-xbeeroute\" option is set the \"XBee API protocol\" will be\n"
  "       used instead of Modbus RTU. [start]:[end] - the modbus nodes interval\n"
  "       that will be routed to [addr] XBee module.\n\n"
  ;
  cout << str;
}


//=============================================================================
// �������������
//=============================================================================
static int init()
{
  QString hostname;
  QString portname;
  int portspeed = 115200;
  bool ok;

  QVector<XBeeRoute> xbeeroute;
  QVector<XBeePostRoute> xbeepostroute;

  //--- ������ ���������� ��������� ������
  QStringList command_arguments = QCoreApplication::arguments();
  command_arguments.removeAt(0);

  foreach( QString str, command_arguments )
  { if( str.startsWith("-host=") )
    { str.remove(0,6);
      hostname = str;
    } else if( str.startsWith("-port=") )
    { str.remove(0,6);
      portname = str;
    } else if( str.startsWith("-baud=") )
    { str.remove(0,6);
      portspeed = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: port speed is invalid";
        return 1;
      }
    } else if( str.startsWith("-tcpport=") )
    { str.remove(0,9);
      tcpport = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: Tcp port number is invalid";
        return 1;
      }
    } else if( str.startsWith("-xbeeroute=") )
    { str.remove(0,11);
      QRegExp rx("(\\d+):(\\d+):(\\d+)");
      if( rx.indexIn( str ) == 0 )
      { xbeeroute << XBeeRoute( rx.cap(1).toInt(), rx.cap(2).toInt(),rx.cap(3).toInt() );
      } else
      { cerr << "Error: XBee route format is invalid.";
        return 1;
      }
    } else if( str.startsWith("-xbeepostroute=") )
    { str.remove(0,15);
      QRegExp rx("(\\d+):(\\d+):(\\d+)");
      if( rx.indexIn( str ) == 0 )
      { xbeepostroute << XBeePostRoute( rx.cap(1).toInt(), rx.cap(2).toInt(),rx.cap(3).toInt() );
      } else
      { cerr << "Error: XBee post route format is invalid.";
        return 1;
      }
    } else
    { cerr << "Configuration error: unknown command line argument: " + str;
      print_help();
      return 1;
    }
  }

  //--- ������ ������������
  if( portname.isEmpty() && hostname.isEmpty())
  { cerr << "Configuration error: Serial port or tcp/ip host does not set.";
    print_help();
    return 1;
  }

  if( !portname.isEmpty() && !hostname.isEmpty())
  { cerr << "Configuration error: Serial port and tcp/ip host can not be used simultaneously.";
    print_help();
    return 1;
  }

  if( !portname.isEmpty() )
  { sp = new SerialPort();
    sp->setName( portname );
    sp->setSpeed( portspeed );

    if( xbeeroute.size() )
    { SerialPort *p = qobject_cast<SerialPort*>(sp);
      p->setMode( SerialPort::XBee );
      foreach( XBeeRoute route, xbeeroute )
      { p->addXBeeRoute( route.start, route.end, route.addr );
      }
      foreach( XBeePostRoute route, xbeepostroute )
      { p->addXBeePostRoute( route.start, route.end, route.id );
      }
    }

    ok = sp->open();
    if( !ok )
    { cerr << "Error: Unable to open port. (" + portname + ")";
      delete sp;
      sp=0;
      return 1;
    }
    return 0;
  }
  if( !hostname.isEmpty() )
  { sp = new MbTcpPort();
    sp->setName( hostname );

    ok = sp->open();
    if( !ok )
    { cerr << "Error: Unable to open port. (" + portname + ")";
      delete sp;
      sp=0;
      return 1;
    }
    return 0;
  }

  return 1;
}




//=============================================================================
// MAIN
//=============================================================================
int main(int argc, char *argv[])
{
  QCoreApplication app( argc, argv );

#ifdef Q_OS_WIN32
  cout.setCodec( QTextCodec::codecForName("CP-866") );
  cerr.setCodec( QTextCodec::codecForName("CP-866") );
#endif

  int ret;

  ret = init();
  if( ret ) return ret;

  ModbusTcpServer server( 0, sp, tcpport );

  app.exec();

  delete sp;

  return ret;
}
