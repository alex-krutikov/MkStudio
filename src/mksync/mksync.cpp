#include <QtCore>
#include <QtXml>

#include "serialport.h"
#include "mbtcp.h"
#include "xfiles.h"

QTextStream cout( stdout );
//QTextStream cerr( stderr );

AbstractSerialPort *sp;
XFiles *xf;

QString remote_path;
QString local_path;
int node=1;
int maxlen=230;
int timeout=200;
int repeats = 5;
int daysbefore = 1;
int monthsbefore = 1;
bool fastmode;
bool dtmode, dtchange;
bool dontusedirs;
bool onlyprevused;
QHash<QString,QString> dt_files, dt_files_saved;
QString xmlf;
QString hostportnode;

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
  "    -maxlen=[Maximum data length in packet]\n"
  "    -timeout=[Serial port timeout (ms)]\n"
  "    -repeats=[number of repats in programm]\n"
  "    -fast\n"
  "    -daysbefore=[Number for set synchronize period on days: days before today is start, today is end]\n"
  "    -monthsbefore=[Number for set synchronize period on month: months before current month is start, current month is end]\n"
  "    -dt=[Synchronize files on date/time]\n"
  "    -nodirs=[Synchronize files at indicated directory. Don't use any dirs]\n"
  "    -onlyprevused=[Synchronize only files that exists on target]\n"
  ;
  cout << str << endl;
}
//=============================================================================
//
//=============================================================================
QString getCoutParams()
{
  return QDateTime::currentDateTime().toString( "ddMMyyhhmmss" ) + " " + hostportnode;
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
      hostportnode += " host="+hostname;
      if( hostportnode.startsWith(" ")) hostportnode = hostportnode.right( hostportnode.length()-1 );
    } else if( str.startsWith("-port=") )
    { str.remove(0,6);
      portname = str;
      hostportnode += " port="+portname;
      if( hostportnode.startsWith(" ")) hostportnode = hostportnode.right( hostportnode.length()-1 );
    } else if( str.startsWith("-baud=") )
    { str.remove(0,6);
      portspeed = str.toInt(&ok);
      if( !ok )
      { cout << getCoutParams() + " Error: port speed is invalid" << endl;
        return 1;
      }
    } else if( str.startsWith("-timeout=") )
    { str.remove(0,9);
      timeout = str.toInt(&ok);
      if( !ok )
      { cout << getCoutParams() + " Error: port timeout is invalid" << endl;
        return 1;
      }
      if( timeout < 50 )    timeout=50;
      if( timeout > 30000 ) timeout=30000;
    } else if( str.startsWith("-repeats=") )
    { str.remove(0,9);
      repeats = str.toInt(&ok);
      if( !ok )
      { cout << getCoutParams() + " Error: Nnumber of repeats is invalid" << endl;
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
      hostportnode += " node=";
      if( hostportnode.startsWith(" ")) hostportnode = hostportnode.right( hostportnode.length()-1 );

      if( !ok )
      { hostportnode += str;
        cout << getCoutParams() + " Error: node is invalid" << endl;
        return 1;
      }
      hostportnode += QString::number( node );
    } else if( str.startsWith("-maxlen=") )
    { str.remove(0,8);
      maxlen = str.toInt(&ok);
      if( !ok || maxlen < 1 || maxlen > 230 )
      { cout << getCoutParams() + " Error: maximum data length is invalid" << endl;
        return 1;
      }
    } else if( str == "-fast" )
    { str.remove(0,5);
      fastmode = true;
    } else if( str.startsWith("-daysbefore=") )
    { str.remove(0,12);
      daysbefore = str.toInt(&ok);
      if( !ok )
      { cout << getCoutParams() + " Error: Nnumber of daysbefore is invalid" << endl;
        return 1;
      }
    } else if( str.startsWith("-monthsbefore=") )
    { str.remove(0,14);
      monthsbefore = str.toInt(&ok);
      if( !ok )
      { cout << getCoutParams() + " Error: Nnumber of monthsbefore is invalid" << endl;
        return 1;
      }
    } else if( str == "-dt" )
    { str.remove(0,3);
      dtmode = true;
    } else if( str == "-nodirs" )
    { str.remove(0,7);
      dontusedirs = true;
    } else if( str == "-onlyprevused" )
    { str.remove(0,13);
      onlyprevused = true;
    } else
    { cout << getCoutParams() +  " Configuration error: unknown command line argument: " + str << endl;
      print_help();
      return 1;
    }
  }

  if( fastmode && dtmode )
  { cout << getCoutParams() + " Configuration error: sinchronization on date-time not supported in fast mode." << endl;
    print_help();
    return 1;
  }

  if( dtmode )
  {
    if(node < 10) xmlf = "/mksync_00" + QString::number( node ) + ".xml";
    else if(node < 100) xmlf = "/mksync_0" + QString::number( node ) + ".xml";
    else xmlf = "/mksync_" + QString::number( node ) + ".xml";
    QDomDocument doc;
    QDomNodeList list;
    QDomElement  element;
    QFile xmlfile( QCoreApplication::applicationDirPath() + xmlf );
    if ( !xmlfile.exists() )
    {
      cout << getCoutParams() + " Xml file error: can't find " + xmlf + " in current directory" << endl;
      return 1;
    }

    if ( !xmlfile.open( QIODevice::ReadOnly ) )
    {
      cout << getCoutParams() + " Xml file error: Can't open " + xmlf + " for read" << endl;
      return 1;
    }
    if ( !doc.setContent( &xmlfile ) )
    {
     cout << getCoutParams() + " Xml file error: " + xmlf + " have incorrect structure" << endl;
     xmlfile.close();
     return 1;
    }
    list = doc.elementsByTagName("File");

    for( int i=0; i<list.count(); i++ )
    {
      element = list.item(i).toElement();
      dt_files.insert( element.attribute("file","0"), element.attribute("date_time","0") );
    }
    xmlfile.close();
  }

  //--- анализ конфигурации
  if( portname.isEmpty() && hostname.isEmpty())
  { cout << getCoutParams() + " Configuration error: Serial port or tcp/ip host does not set." << endl;
    print_help();
    return 1;
  }

  if( !portname.isEmpty() && !hostname.isEmpty())
  { cout << getCoutParams() + " Configuration error: Serial port and tcp/ip host can not be used together." << endl;
    print_help();
    return 1;
  }

  if( local_path.isEmpty() )
  { cout << getCoutParams() + " Configuration error: Local path does not set." << endl;
    print_help();
    return 1;
  }

  if( remote_path.isEmpty() )
  { cout << getCoutParams() + " Configuration error: Remote path does not set." << endl;
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
    { cout << getCoutParams() + " Error: Unable to open port. (" + portname + ")" << endl;
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
    { cout << getCoutParams() + " Error: Unable to open port. (" + portname + ")" << endl;
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
  cout << getCoutParams() + " XFiles error: " + XFiles::decodeResult( res ) << endl;
}

//=============================================================================
// Синхронизация файлов
//=============================================================================
bool sync_file(const QString filename, const int filesize, const QString &path )
{
  QByteArray ba;
  XFiles::Result res;
  XFilesFileInfo xfi;
  bool ok;
  int id,j=0,len;
  QString hashDT;
  int rep = repeats;
  if( dtmode )
  {
    while( rep-- )
    {
      res = xf->getFileInfo( remote_path + path + filename, xfi );
      xfile_print_error( res );
      if( res == XFiles::Ok ) break;
    }

    if( res != XFiles::Ok ) return false;

    if( !dt_files.contains( remote_path + path + filename ) )
    {
      dt_files.insert(remote_path+path+filename, xfi.mtime.addDays(1).toString("ddMMyyhhmmss"));
    }
    hashDT = dt_files.value( remote_path + path + filename );
    dt_files_saved.insert( remote_path+path+filename, hashDT );

    QFile filecheck( local_path + path +  filename );
    if( hashDT == xfi.mtime.toString("ddMMyyhhmmss") && filecheck.exists() ) return true;
    hashDT = xfi.mtime.toString("ddMMyyhhmmss");
    dtchange = true;
  }

  //--- проверка наличия и размера файла
  QFile file( local_path + path +  filename );

  if( !file.exists() && onlyprevused )
  {
    if( dtmode ) dt_files_saved.remove( remote_path+path+filename );

    return true;
  }

  if( file.exists() && ( file.size() >= filesize ) && !dtmode ) return true;

  //--- дочитка файла
  if( dtmode ) ok = file.open(QIODevice::WriteOnly|QIODevice::Truncate);
  else         ok = file.open(QIODevice::WriteOnly|QIODevice::Append  );

  if( !ok )
  { cout << getCoutParams() + " File error: Can't open file " + file.fileName() + " for write" << endl;
    return false;
  }

  if( !dtmode )
  {
    j = file.size();
    file.seek(j);
  }

  cout << getCoutParams() + " Update file: " << file.fileName() << " at " << j <<"-" << filesize << endl;
  rep = repeats;
  while( rep-- )
  {
    res = xf->openFile( remote_path + path + filename, XFiles::FA_OPEN_EXISTING, &id );
    xfile_print_error( res );
    if( res == XFiles::Ok ) break;
  }
  if( res != XFiles::Ok ) return false;


  //--- получение и запись данных
  while( j < filesize )
  {
    len = filesize - j;
    if( len > maxlen ) len = maxlen;

    rep = repeats;
    while( rep-- )
    {
      res = xf->readFile( id, j, len, ba );
      xfile_print_error( res );
      if( res == XFiles::Ok ) break;
    }
    if( res != XFiles::Ok ) goto error;

    file.write( ba );
    j += len;
  }

  rep = repeats;
  while( rep-- )
  {
    res = xf->closeFile( id );
    xfile_print_error( res );
    if( res == XFiles::Ok ) break;
  }

  if( dtmode )
  {
    dt_files[remote_path + path + filename]       = hashDT;
    dt_files_saved[remote_path + path + filename] = hashDT;
  }

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
  {  cout << getCoutParams() + " Create path: " << dirname << endl;
     dir.mkpath( dirname );
  }

  //--- чтение содержимого каталога
  int id;
  XFiles::Result res;
  QList<XFilesFileInfo> flist;

  int rep = repeats;
  while( rep-- )
  {
    res = xf->openDirectory( path, &id );
    xfile_print_error( res );
    if( res == XFiles::Ok ) break;
  }

  if( res != XFiles::Ok ) return false;

  rep = repeats;
  while( rep-- )
  {
    res = xf->readDirectory( id, flist );
    xfile_print_error( res );
    if( res == XFiles::Ok ) break;
  }
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

  if( dontusedirs ) return true;

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
static int fast_sync_date( const QDate &date, const QString &ext )
{
  XFiles::Result res;
  XFilesFileInfo fi;

  QString path = date.toString("yyMM/");
  if( ext==".RPH" || ext==".RNH" ) path = date.toString("yy/");

  QString filename = date.toString("~yyMMdd") + ext ;
  //--- получение описания файла
  int rep = repeats;
  while( rep-- )
  {
    res = xf->getFileInfo( remote_path+path+filename, fi );
    xfile_print_error( res );
    if( res == XFiles::Ok ) break;
  }
  if( res != XFiles::Ok ) return 1;

  //--- создание каталога
  QString dirname = local_path + path;
  QDir dir( dirname );
  if( !dir.exists() )
  {  cout << getCoutParams() + " Create path: " << dirname << endl;
     dir.mkpath( dirname );
  }

  //--- синхронизация файла;
  bool ok =  sync_file(  filename, fi.size, path );
  return (ok) ? (0) : (1);
}

//=============================================================================
// Быстрая синхронизация на основе анализа текущей даты
//=============================================================================
static int fast_sync()
{
    QByteArray ba;
    ba.resize(7);
  int i;

  QDate date;
  while( daysbefore >= 0 )
  {
   date = QDate::currentDate();
   date = date.addDays( ((-1)*daysbefore) );

   //минутные тренды
   i = fast_sync_date( date, ".RPM" );
   if( i ) break;
   i = fast_sync_date( date, ".RNM" );
   if( i ) break;

   daysbefore--;
  }
  if( i ) return i;

  while( monthsbefore >= 0 )
  {
   date = QDate::currentDate();
   date = QDate(date.year(), date.month(), 1);//1 число данного месяца
   date = date.addMonths( ((-1)*monthsbefore) );

   //часовые тренды
   i = fast_sync_date( date, ".RPH" );
   if( i ) break;
   i = fast_sync_date( date, ".RNH" );
   if( i ) break;

   monthsbefore--;
  }
  return i;
}


//=============================================================================
// MAIN
//=============================================================================
int main(int argc, char *argv[])
{
  QCoreApplication app( argc, argv );

#ifdef Q_OS_WIN32
  cout.setCodec( QTextCodec::codecForName("CP-866") );
  //cerr.setCodec( QTextCodec::codecForName("CP-866") );
#endif

  int ret = 0;

  ret = init();
  cout << getCoutParams() + " mksync started" << endl;
  if( ret )
  {
    cout << getCoutParams() + " mksync done" << endl;
    return ret;
  }

  QString lockName;

  if( node < 10 )       lockName = "mksync_00" + QString::number( node );
  else if( node < 100 ) lockName = "mksync_0"  + QString::number( node );
  else                  lockName = "mksync_"   + QString::number( node );

  QSharedMemory secondCopyBlock( lockName );
  if( !( secondCopyBlock.create(1)
         && secondCopyBlock.error()!=QSharedMemory::AlreadyExists ) )
  {
    cout << getCoutParams() + " second copy of programm was started" << endl;
    cout << getCoutParams() + " mksync done" << endl;
    return ret;
  }

  if( !remote_path.endsWith('/') ) remote_path +="/";
  if( !local_path.endsWith('/')  ) local_path  +="/";

  xf = new XFiles;
  xf->setTransport( sp );
  xf->setNode( node );

  if( fastmode )
  { ret = fast_sync();
  } else
  { bool ok = process_dir("");
    ret = (ok) ? (0) : (1);
  }

  delete xf;
  delete sp;

  if( dtmode && dtchange )
  {
    QFile file( QCoreApplication::applicationDirPath() + xmlf );
    if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      cout << getCoutParams() + " Xml file error: Can't open " + xmlf + " for write" << endl;
      cout << getCoutParams() + " mksync done" << endl;
      return 1;
    }
    QDomDocument doc;
    QDomElement  root = doc.createElement( "Files" );

    QHash<QString,QString>::iterator iit = dt_files_saved.begin();
    while (iit != dt_files_saved.end() )
    {
     QDomElement fl = doc.createElement( "File" );
     fl.setAttribute( "file",      iit.key()   );
     fl.setAttribute( "date_time", iit.value() );
     root.appendChild( fl );
     iit++;
    }
    doc.appendChild( root );

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251"  );
    file.write( "<?xml version=\"1.0\" encoding=\"cp-1251\"?>\n" );
    file.write( codec->fromUnicode( doc.toString(2) )            );
    file.close();
  }

   cout << getCoutParams() + " mksync done" << endl;

   return ret;
}
