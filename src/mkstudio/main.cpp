#include <QtGui>

#include "main.h"
#include "mainwindow.h"
#include "dialogs.h"
#include "mbmasterxml.h"
#include "consolewidget.h"
#include "abstractserialport.h"

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
  QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1251"));
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("Windows-1251"));

  QTranslator *qt_translator = new QTranslator;
  if ( qt_translator->load( ":tr/qt_ru.qm" ) )
  { QApplication::installTranslator( qt_translator );
  }

  app_header = "MKStudio";


  InitDialog initdialog;
  if( initdialog.exec() != QDialog::Accepted ) goto exit;

  if( !port->open() )
  { QMessageBox::critical(0,app_header,
                             "������ �������� �����.\n\n"
                             "��������, ��� ���� ������������\n"
                             "������ �����������." );
    goto exit;
  }

  app_header = QString("%1 %2 - MKStudio").arg(port->name()).arg(port->speed() );

  mbmaster = new MBMasterXML;
  mbmaster->setTransport( port );
  mbmaster->setMaximumPacketLength( initdialog.sb_max_len->value() );

  helpwidget = new HelpWidget;
  helpwidget->setWindowTitle("����������� MKStudio");
  helpwidget->resize(900,600);
  helpwidget->setContents( QUrl::fromLocalFile(":/html/help/index.html" ) );

  mainwindow = new MainWindow;
  mainwindow->show();

  mainwindow->mbmasterwidget->setMBMaster( mbmaster );

  ret=application->exec();

  delete mainwindow;
  delete mbmaster;
exit:
  delete port;
  delete qt_translator;

  return ret;
}
