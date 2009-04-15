#include <QtCore>

#include "serialport.h"
#include "xfiles.h"

QTextStream cout( stdout );
QTextStream cerr( stderr );

QString port_name;
int port_speed = 115200;

int main(int argc, char *argv[])
{
  QCoreApplication app( argc, argv );

#ifdef Q_OS_WIN32
  cout.setCodec( QTextCodec::codecForName("CP-866") );
  cerr.setCodec( QTextCodec::codecForName("CP-866") );
#endif


  QString port_name;

  QStringList command_arguments = QCoreApplication::arguments();
  command_arguments.removeAt(0);

  foreach( QString str, command_arguments )
  { if( str.startsWith("-port=") )
    { str.remove(0,6);
      port_name = str;
    } else if( str.startsWith("-speed=") )
    { str.remove(0,7);
      bool ok;
      port_speed = str.toInt(&ok);
      if( !ok )
      { cerr << "Error: port speed is invalid";
        return 1;
      }
    } else
    { cerr << "Error: unknown command line argument: " + str;
      return 1;
    }
  }

  SerialPort port;
  port.setName( port_name );
  port.setSpeed( port_speed );
  if( !port.open() )
  {   cerr << QString("Unable to open port %1 at %2.").arg(port_name).arg(port_speed);
      return 1;
  }

  XFiles xf;
  xf.setTransport( &port );
  xf.setNode( 8 );

  int id;
  XFiles::Result res;
  QList<XFilesFileInfo> flist;

  res = xf.openDirectory( "/", &id );
  if( res != XFiles::Ok ) return 1;
  res = xf.readDirectory( id, flist );
  if( res != XFiles::Ok ) return 1;

  foreach( XFilesFileInfo fi, flist )
  {
    cout << fi.name << endl;
  }


  return 0;
}

