#include <QtCore>

#include "serialport.h"
#include "mbtcp.h"
#include "mbtcpserver.h"

#include "mbcommon.h"
#include "crc.h"


QTextStream cout( stdout );
QTextStream cerr( stderr );

AbstractSerialPort *sp;

int verb_mode;
int repeats_count = 5;
enum Mode { UnknownMode=0, Read, Write } operation_mode;
bool unsigned_mode;
int addr;
int data_count;
QStringList data;

enum  DataType { UnknownType=0, Bits, Bytes, Words, DWords, Floats } data_type;

int node=1;

//=============================================================================
// Вывод информации о программе
//=============================================================================
static void print_help()
{
  QString str =
  "\n"
  "\n"
  "usage: mkquery [options] [data]\n"
  "\n"
  "  Options:\n"
  "    -port=[Serial port name]\n"
  "    -baud=[Serial port baud rate]\n"
  "    -host=[Modbus TCP host name]\n"
  "    -node=[Modbus node]\n"
  "    -addr=[Modbus memory address]\n"
  "    -type=[bits|bytes|words|dwords|float]\n"
  "    -count=[Parameters count]\n"
  "    -read\n"
  "    -write\n"
  "    -unsigned\n"
  "    -repeats=[Repeats count]\n"
  "    -v\n"
  ;
  cout << str;
}


//=============================================================================
// Инициализация
//=============================================================================
static int init()
{
  QString hostname;
  QString portname;
  int portspeed = 115200;
  bool ok;

  //--- разбор аргументов командной строки
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
    } else if( str.startsWith("-node=") )
    { str.remove(0,6);
      node = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: node is invalid";
        return 1;
      }
    } else if( str.startsWith("-type=") )
    { str.remove(0,6);
      str=str.toUpper();
      if( str.endsWith("S") ) str.chop(1);
      if( str=="BIT" ) data_type = Bits;
      else if( str=="BYTE" )  data_type = Bytes;
      else if( str=="WORD" )  data_type = Words;
      else if( str=="DWORD" ) data_type = DWords;
      else if( str=="FLOAT" ) data_type = Floats;
    } else if( !str.startsWith("-") )
    { data << str;
    } else if( str == "-read" )
    { operation_mode = Read;
    } else if( str == "-write" )
    { operation_mode = Write;
    } else if( str == "-v" )
    { verb_mode = 1;
    } else if( str.startsWith( "-repeats=" ) )
    { str.remove(0,9);
      repeats_count = str.toInt( &ok );
      if( !ok )
      {  cerr << "Error: repeats count is invalid";
        return 1;
      }
    } else if( str == "-unsigned" )
    { unsigned_mode = 1;
    } else if( str.startsWith("-count=") )
    { str.remove(0,7);
      data_count = str.toInt( &ok );
      if( !ok )
      {  cerr << "Error: data count is invalid";
        return 1;
      }
    } else if( str.startsWith("-addr=") )
    { str.remove(0,6);
      ok=false;
      if( str.startsWith( "0x" ) )
      { str.remove(0,2);
        addr = str.toInt( &ok, 16 );
      } else
      { addr = str.toInt( &ok );
      }
      if( !ok )
      {  cerr << "Error: memory address is invalid";
        return 1;
      }
    } else
    { cerr << "Configuration error: unknown command line argument: " + str;
      print_help();
      return 1;
    }
  }

  //--- анализ конфигурации
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

  if( data_type == UnknownType )
  { cerr << "Configuration error:  Data type does not set.";
    print_help();
    return 1;
  }

  if( operation_mode == UnknownMode )
  { cerr << "Configuration error:  Mode of operation (read or write) does not set.";
    print_help();
    return 1;
  }

  if( operation_mode == Read  && data.size() )
  { cerr << "Configuration error: Data can not be set in read mode.";
    print_help();
    return 1;
  }

  if( operation_mode == Write &&( data.count() < 1 ) )
  { cerr << "Configuration error: Data not found.";
    print_help();
    return 1;
  }

  if( ( operation_mode ) == Write && ( data_type  == Bits ) && ( data.count()> 1 ) )
  { cerr << "Configuration error: It is possible to write only a single bit.";
    print_help();
    return 1;
  }

  if( !portname.isEmpty() )
  { sp = new SerialPort();
    sp->setName( portname );
    sp->setSpeed( portspeed );

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

  if( verb_mode > 0 )
  {
  }

  ModbusTcpServer server( 0, sp, 502 );

  app.exec();

  delete sp;

  return ret;
}
