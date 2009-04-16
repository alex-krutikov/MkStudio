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
int node;

//=============================================================================
// Инициализация
//=============================================================================
static int init()
{
  QString hostname;
  QString portname;
  int portspeed;
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
    } else if( str.startsWith("-speed=") )
    { str.remove(0,7);
      portspeed = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: port speed is invalid";
        return 1;
      }
    } else if( str.startsWith("-rdir=") )
    { str.remove(0,6);
      remote_path = str;
    } else if( str.startsWith("-ldir=") )
    { str.remove(0,6);
      local_path = str;
    } else if( str.startsWith("-node=") )
    { str.remove(0,6);
      node = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: node is invalid";
        return 1;
      }
    } else
    { cerr << "Error: unknown command line argument: " + str;
      return 1;
    }
  }

  if( !portname.isEmpty() )
  { sp = new SerialPort();
    sp->setName( portname );
    sp->setSpeed( portspeed );

    ok = sp->open();
    if( !ok )
    { cerr << "Unable to open port. (" + portname + ")";
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
    { cerr << "Unable to open port. (" + portname + ")";
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
void sync_file(const QString filename, const int filesize, const QString &path )
{
  QByteArray ba;
  XFiles::Result res;
  bool ok;
  int id,j,len;

  //--- проверка наличия и размера файла
  QFile file( local_path + path +  filename );
  if( file.exists() && ( file.size() >= filesize ) ) return;

  //--- дочитка файла
  ok = file.open(QIODevice::WriteOnly|QIODevice::Append);
  if( !ok )
  { cerr << "ERROR: Can't open file for write. " << file.fileName() ;
    return;
  }

  j = file.size();
  file.seek(j);

  cout << "Update file: " << file.fileName() << " at " << j <<"-" << filesize << endl;

  res = xf->openFile( remote_path + path + filename, XFiles::FA_OPEN_EXISTING, &id );
  xfile_print_error( res );
  if( res != XFiles::Ok ) return;


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

  return;
error:
    res = xf->closeFile( id );
    xfile_print_error( res );
    if( res != XFiles::Ok ) return;
}

//=============================================================================
// Синхронизация каталога (рекурсивная функция)
//=============================================================================
void process_dir( QString path )
{
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
  if( res != XFiles::Ok ) return;

  res = xf->readDirectory( id, flist );
  xfile_print_error( res );
  if( res != XFiles::Ok ) return;

  //--- обработка файлов катлога
  QStringList sl_dirs;
  foreach( XFilesFileInfo fi, flist )
  {
    if( fi.isDir() )
    { sl_dirs << fi.name;
      continue;
    }
    sync_file( fi.name, fi.size, path );
  }

  //--- рекурсивный вызов для всех подкаталогов
  foreach( QString str, sl_dirs )
  { if( str == "."  ) continue;
    if( str == ".." ) continue;
    process_dir( path + str + "/" );
  }
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
  { cout << "Serial Port: " << sp->name() << " at " << sp->speed() << endl;
  } else
  { cout << "Modbus TCP host: " << sp->name() << endl;
  }
  cout << endl;

  process_dir("");

  delete xf;
  delete sp;

  return 0;
}

