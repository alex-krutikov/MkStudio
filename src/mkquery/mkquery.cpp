#include <QtCore>

#include "serialport.h"
#include "mbtcp.h"

#include "mbcommon.h"
#include "crc.h"


QTextStream cout( stdout );
QTextStream cerr( stderr );

AbstractSerialPort *sp;

int verb_mode;
int repeats_count = 5;
int timeout = 200;
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
  "    -timeout=[Serial port timeout (ms)]\n"
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
    } else if( str.startsWith("-timeout=") )
    { str.remove(0,9);
      timeout = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: port timeout is invalid";
        return 1;
      }
      if( timeout < 50 )    timeout=50;
      if( timeout > 30000 ) timeout=30000;
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
    sp->setAnswerTimeout( timeout );

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
    sp->setAnswerTimeout( timeout );

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
//
//=============================================================================
static QString ptr_value( int offset, const char* ptr  )
{
  QString str;

  switch( data_type )
  { case( Bits   ):
      return (*(ptr+(offset/8)) & ( 1<<offset & 0x07 )) ? "1" : "0" ;
      break;
    case( Bytes  ):
      if( unsigned_mode )
      { return QString::number( (unsigned int)(*(unsigned char*)(ptr+offset)) );
      } else
      { return QString::number( (int)(*(char*)(ptr+offset)) );
      }
      break;
    case( Words  ):
      if( unsigned_mode )
      { return QString::number( (unsigned int)(*(unsigned short*)(ptr+2*offset)) );
      } else
      { return QString::number( (unsigned int)(*(short*)(ptr+2*offset)) );
      }
      break;
    case( DWords ):
      if( unsigned_mode )
      { return QString::number( (unsigned int)(*(unsigned int*)(ptr+4*offset)) );
      } else
      { return QString::number( (int)(*(int*)(ptr+4*offset)) );
      }
      break;
    case( Floats ):
      return QString::number( (double)(*(float*)(ptr+4*offset)) );
      break;
    default: break;
  }
  return QString();
}

//=============================================================================
//
//=============================================================================
bool read_data()
{
  QByteArray req,ans;

  int len=0,tr=0,i,j;

  switch( data_type )
  { case( Bits   ): len = ((data_count-1)/8)+1; break;
    case( Bytes  ): len = data_count; break;
    case( Words  ): len = 2*data_count; break;
    case( DWords ): len = 4*data_count; break;
    case( Floats ): len = 4*data_count; break;
    default: break;
  }

  if( len < 1 || len > 255 )
  { cerr << "Error: Data length invalid";
    return 1;
  }

  req.resize(6);
  req[0] = node;
  req[1] = 0x43;
  req[2] = 0x00;
  req[3] = addr >> 8;
  req[4] = addr;
  req[5] = len;
  CRC::appendCRC16( req );

  ans.resize( 6 + len );

  tr=repeats_count;
  while( tr-- )
  { j = sp->query( req, ans );

    if( verb_mode > 0 )
    { QByteArray ba_ans = ans;
      ba_ans.resize( j );
      cout << "R: " << QByteArray2QString( req    ) << endl;
      cout << "A: " << QByteArray2QString( ba_ans ) << endl;
      cout << endl << "  Data:" << endl << endl;
    }

    if( j != ans.size()    ) continue;
    if( CRC::CRC16( ans )  ) continue;
    if( ans[0] != req[0]   ) continue;
    if( ans[1] != req[1]   ) continue;
    if( ans[2] != req[2]   ) continue;
    if( (unsigned char)ans[3] != (unsigned char)len      ) continue;
    break;
  }
  if( tr<0 )
  { return 1;
  }

  for( i=0; i<data_count; i++ )
  { cout << ptr_value( i, ans.constData()+4 ) << endl;
  }

  return 0;
}

//=============================================================================
//
//=============================================================================
static bool value_ptr( const QString &value, char* ptr, int offset )
{
  bool ok=false;

  switch( data_type )
  { case( Bits  ):
     { char mask = ( 1<<offset & 0x07 );
       char *p = ptr+(offset/8);
       int v = value.toInt( &ok );
       if( !ok ) return false;
       (*p) = (v) ? (mask) : (~mask);
     }
     break;
    case( Bytes  ):
      if( unsigned_mode )
      { unsigned char v = value.toUInt( &ok );
        if( !ok ) return false;
         *((unsigned char*)(ptr+offset)) = v;
      } else
      { char v = value.toInt( &ok );
        if( !ok ) return false;
         *((char*)(ptr+offset)) = v;
      }
      break;
    case( Words  ):
      if( unsigned_mode )
      { unsigned short v = value.toUInt( &ok );
        if( !ok ) return false;
         *((unsigned short*)(ptr+2*offset)) = v;
      } else
      { char v = value.toInt( &ok );
        if( !ok ) return false;
         *((short*)(ptr+2*offset)) = v;
      }
      break;
    case( DWords  ):
      if( unsigned_mode )
      { unsigned int v = value.toUInt( &ok );
        if( !ok ) return false;
         *((unsigned int*)(ptr+4*offset)) = v;
      } else
      { int v = value.toInt( &ok );
        if( !ok ) return false;
         *((int*)(ptr+4*offset)) = v;
      }
      break;
    case( Floats  ):
      { double v = value.toDouble( &ok );
        if( !ok ) return false;
         *((float*)(ptr+4*offset)) = (float)v;
      }
      break;
    default: break;
  }
  return true;
}

//=============================================================================
//
//=============================================================================
static bool write_data()
{
  data_count = data.count();

  QByteArray req,ans;

  int len=0,tr=0,i,j;
  bool ok;

  switch( data_type )
  { case( Bits   ): len = ((data_count-1)/8)+1; break;
    case( Bytes  ): len = data_count; break;
    case( Words  ): len = 2*data_count; break;
    case( DWords ): len = 4*data_count; break;
    case( Floats ): len = 4*data_count; break;
    default: break;
  }

  if( len < 1 || len > 200 )
  { cerr << "Error: Data length invalid";
    return 1;
  }

  req.resize(6+len);
  req[0] = node;
  req[1] = 0x43;
  req[2] = 0x01;
  req[3] = addr >> 8;
  req[4] = addr;
  req[5] = len;

  for( i=0; i<data_count; i++ )
  { ok = value_ptr( data[i], req.data()+6, i );
    if( !ok )
    { cerr << "Error: Data is invalid." << endl;
      return 0;
    }
  }

  if( data_type == Bits )
  {  req[2] = data[0].toInt() ? 0x05 : 0x03;
  }

  CRC::appendCRC16( req );

  ans.resize( req.size() );

  tr=repeats_count;
  while( tr-- )
  { j = sp->query( req, ans );

    if( verb_mode > 0 )
    { QByteArray ba_ans = ans;
      ba_ans.resize( j );
      cout << "R: " << QByteArray2QString( req    ) << endl;
      cout << "A: " << QByteArray2QString( ba_ans ) << endl;
    }

    if( j != ans.size()    ) continue;
    if( CRC::CRC16( ans )  ) continue;
    if( ans[0] != req[0]   ) continue;
    if( ans[1] != req[1]   ) continue;
    if( ans[2] != req[2]   ) continue;
    if( ans[3] != req[3]   ) continue;
    if( ans[4] != req[4]   ) continue;
    if( ans[5] != req[5]   ) continue;
    break;
  }
  if( tr<0 )
  { return 1;
  }

  return 0;
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

  if( operation_mode == Read  ) ret = read_data();
  if( operation_mode == Write ) ret = write_data();

  delete sp;

  return ret;
}
