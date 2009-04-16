#include <QtCore>

#include "serialport.h"
#include "mbtcp.h"
#include "xfiles.h"

QTextStream cout( stdout );
QTextStream cerr( stderr );

AbstractSerialPort *sp;
XFiles *xf;

QString remote_path;
QString local_path;
int node=1;
bool fastmode;

//=============================================================================
// Вывод информации о программе
//=============================================================================
static void print_help()
{
  QString str =
  "\n"
  "\n"
  "usage: mksync [options]\n"
  "\n"
  "  Options:\n"
  "    -port=[Serial port name]\n"
  "    -baud=[Serial port baud rate]\n"
  "    -host=[Modbus TCP host name]\n"
  "    -node=[Modbus node]\n"
  "    -rpath=[Remote path]\n"
  "    -lpath=[Local path]\n"
  "    -fast\n"
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
    } else if( str.startsWith("-rpath=") )
    { str.remove(0,7);
      if( str.startsWith('"') ) str.remove(0,1);
      if( str.endsWith('"') ) str.chop(1);
      remote_path = str;
    } else if( str.startsWith("-lpath=") )
    { str.remove(0,7);
      if( str.startsWith('"') ) str.remove(0,1);
      if( str.endsWith('"') ) str.chop(1);
      local_path = str;
    } else if( str.startsWith("-node=") )
    { str.remove(0,6);
      node = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: node is invalid";
        return 1;
      }
    } else if( str == "-fast" )
    { fastmode = true;
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
  { cerr << "Configuration error: Serial port and tcp/ip host can not be used together.";
    print_help();
    return 1;
  }

  if( local_path.isEmpty() )
  { cerr << "Configuration error: Local path does not set.";
    print_help();
    return 1;
  }

  if( remote_path.isEmpty() )
  { cerr << "Configuration error: Remote path does not set.";
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
// Анализ ошибок XFiles
//=============================================================================
static void xfile_print_error( XFiles::Result res )
{
  if( res == XFiles::Ok ) return;
  cerr <<  "XFiles error: " + XFiles::decodeResult( res ) << endl;
}

//=============================================================================
// Синхронизация файлов
//=============================================================================
bool sync_file(const QString filename, const int filesize, const QString &path )
{
  QByteArray ba;
  XFiles::Result res;
  bool ok;
  int id,j,len;

  //--- проверка наличия и размера файла
  QFile file( local_path + path +  filename );
  if( file.exists() && ( file.size() >= filesize ) ) return true;

  //--- дочитка файла
  ok = file.open(QIODevice::WriteOnly|QIODevice::Append);
  if( !ok )
  { cerr << "ERROR: Can't open file for write. " << file.fileName() << endl;
    return false;
  }

  j = file.size();
  file.seek(j);

  cout << "Update file: " << file.fileName() << " at " << j <<"-" << filesize << endl;

  res = xf->openFile( remote_path + path + filename, XFiles::FA_OPEN_EXISTING, &id );
  xfile_print_error( res );
  if( res != XFiles::Ok ) return false;


  //--- получение и запись данных
  while( j < filesize )
  {
    len = filesize - j;
    if( len > 80 ) len = 80;

    res = xf->readFile( id, j, len, ba );
    xfile_print_error( res );
    if( res != XFiles::Ok ) goto error;

    file.write( ba );
    j += len;
  }

  res = xf->closeFile( id );
  xfile_print_error( res );

  return true;
error:
  res = xf->closeFile( id );
  return false;
}

//=============================================================================
// Синхронизация каталога (рекурсивная функция)
//=============================================================================
bool process_dir( QString path )
{
  bool ok;
  QString dirname = local_path + path;

  //--- создание каталога
  QDir dir( dirname );
  if( !dir.exists() )
  {  cout << "Create path: " << dirname << endl;
     dir.mkpath( dirname );
  }

  //--- чтение содержимого каталога
  int id;
  XFiles::Result res;
  QList<XFilesFileInfo> flist;

  res = xf->openDirectory( path, &id );
  xfile_print_error( res );
  if( res != XFiles::Ok ) return false;

  res = xf->readDirectory( id, flist );
  xfile_print_error( res );
  if( res != XFiles::Ok ) return false;

  //--- обработка файлов катлога
  QStringList sl_dirs;
  foreach( XFilesFileInfo fi, flist )
  {
    if( fi.isDir() )
    { sl_dirs << fi.name;
      continue;
    }
    ok = sync_file( fi.name, fi.size, path );
    if( !ok ) return false;
  }

  //--- рекурсивный вызов для всех подкаталогов
  foreach( QString str, sl_dirs )
  { if( str == "."  ) continue;
    if( str == ".." ) continue;
    ok = process_dir( path + str + "/" );
    if( !ok ) return false;
  }
  return true;
}

//=============================================================================
// Синхронизация файла минутного тренда по дате
//=============================================================================
static void fast_sync_date( const QDate &date, const QString &ext )
{
  XFiles::Result res;
  XFilesFileInfo fi;

  QString path     = date.toString("yyMM/");
  QString filename = date.toString("~yyMMdd") + ext ;

  //--- получение описания файла
  res = xf->getFileInfo( remote_path+path+filename, fi );
  xfile_print_error( res );
  if( res != XFiles::Ok ) return;

  //--- создание каталога
  QString dirname = local_path + path;
  QDir dir( dirname );
  if( !dir.exists() )
  {  cout << "Create path: " << dirname << endl;
     dir.mkpath( dirname );
  }

  //--- синхронизация файла
  sync_file(  filename, fi.size, path );
}

//=============================================================================
// Быстрая синхронизация на основе анализа текущей даты
//=============================================================================
static void fast_sync()
{
  QDate date = QDate::currentDate();
  fast_sync_date( date, ".RPM" );
  fast_sync_date( date, ".RNM" );
  date = date.addDays(-1);
  fast_sync_date( date, ".RPM" );
  fast_sync_date( date, ".RNM" );
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

  if( !remote_path.endsWith('/') ) remote_path +="/";
  if( !local_path.endsWith('/') )  local_path  +="/";

  xf = new XFiles;
  xf->setTransport( sp );
  xf->setNode( node );

  if( sp->speed() )
  { //cout << "Serial Port: " << sp->name() << " at " << sp->speed() << endl;
  } else
  { //cout << "Modbus TCP host: " << sp->name() << endl;
  }

  if( fastmode )
  { fast_sync();
  } else
  { process_dir("");
  }

  delete xf;
  delete sp;

  return 0;
}
