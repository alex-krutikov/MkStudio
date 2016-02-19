#include "main.h"

#include "mainwindow.h"
#include "dialogs.h"
#include "utils.h"


#include <abstractserialport.h>
#include <mbmasterxml.h>
#include <console.h>

#include <QMessageBox>

//==============================================================================
/// MAIN
//==============================================================================
int main(int argc, char *argv[])
{
#ifdef Q_OS_UNIX
  QApplication::setStyle("cleanlooks");
#endif
  QApplication app( argc, argv);

  QTranslator qt_translator;
  if ( qt_translator.load( ":tr/qt_ru.qm" ))
  { QApplication::installTranslator(&qt_translator);
  }

  Utils::AppInfo::setTitle("MKStudio");

  Console::setMessageTypes( Console::AllTypes );
  Console::setMessageTypes( Console::ModbusPacket, false );

  InitDialog initdialog;
  if( initdialog.exec() != QDialog::Accepted )
      return 0;

  AbstractSerialPortPtr port = initdialog.port();
  if( !port->open() )
  { QMessageBox::critical(0, Utils::AppInfo::title(),
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением." );
    return 0;
  }

  QString portname = port->name();
  QString portspeed = QString::number(port->speed());
  if(portspeed=="0" ) portspeed.clear();
  Utils::AppInfo::setTitle(QString("%1 %2 - MKStudio").arg(portname).arg(portspeed));

  MBMasterXMLPtr mbmaster(new MBMasterXML);
  mbmaster->setTransport( port );
  mbmaster->setMaximumPacketLength( initdialog.sb_max_len->value() );
  mbmaster->setTransactionDelay( initdialog.sb_tr_delay->value() );
  mbmaster->setCycleTime( initdialog.sb_cycle_time->value() );

  MainWindow mainwindow(mbmaster);
  mainwindow.show();

  return QApplication::exec();
}
