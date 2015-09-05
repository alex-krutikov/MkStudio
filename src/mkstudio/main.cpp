#include <QtGui>

#include <QMessageBox>

#include "main.h"
#include "mainwindow.h"
#include "dialogs.h"
#include "mbmasterxml.h"
#include "consolewidget.h"
#include "abstractserialport.h"
#include "console.h"

#include "helpwidget.h"

MainWindow    *mainwindow;
MBMasterXML   *mbmaster;
AbstractSerialPort  *port;
HelpWidget    *helpwidget;
QString       app_header;

//==============================================================================
/// MAIN
//==============================================================================
int main(int argc, char *argv[])
{
  int ret=0;
#ifdef Q_OS_UNIX
  QApplication::setStyle("cleanlooks");
#endif
  QApplication app( argc, argv);

  QTranslator *qt_translator = new QTranslator;
  if ( qt_translator->load( ":tr/qt_ru.qm" ) )
  { QApplication::installTranslator( qt_translator );
  }

  app_header = "MKStudio";

  Console::setMessageTypes( Console::AllTypes );
  Console::setMessageTypes( Console::ModbusPacket, false );

  InitDialog initdialog;
  if( initdialog.exec() != QDialog::Accepted ) goto exit;

  if( !port->open() )
  { QMessageBox::critical(0,app_header,
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением." );
    goto exit;
  }

  {
  QString portname = port->name();
  QString portspeed = QString::number(port->speed());
  if(portspeed=="0" ) portspeed.clear();
  app_header = QString("%1 %2 - MKStudio").arg(portname).arg(portspeed);
  }

  mbmaster = new MBMasterXML;
  mbmaster->setTransport( port );
  mbmaster->setMaximumPacketLength( initdialog.sb_max_len->value() );
  mbmaster->setTransactionDelay( initdialog.sb_tr_delay->value() );
  mbmaster->setCycleTime( initdialog.sb_cycle_time->value() );

  helpwidget = new HelpWidget;
  helpwidget->setWindowTitle("Руководство MKStudio");
  helpwidget->resize(900,600);
  helpwidget->setContents( QUrl::fromLocalFile(":/html/help/index.html" ) );

  mainwindow = new MainWindow;
  mainwindow->show();

  mainwindow->mbmasterwidget->setMBMaster( mbmaster );

  ret=QApplication::exec();

  delete mainwindow;
  delete mbmaster;
exit:
  delete port;
  delete qt_translator;

  return ret;
}
