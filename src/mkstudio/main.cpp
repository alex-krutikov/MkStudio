#include <QtGui>

#include "main.h"
#include "mainwindow.h"
#include "dialogs.h"
#include "mbmaster.h"
#include "consolewidget.h"

#include "helpwidget.h"

QApplication  *application;
MainWindow    *mainwindow;
MBMaster      *mbmaster;
SerialPort    *port;
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
  application = new QApplication( argc, argv);
  QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1251"));
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("Windows-1251"));

  QTranslator *qt_translator = new QTranslator;
  if ( qt_translator->load( ":tr/qt_ru.qm" ) )
  { application->installTranslator( qt_translator );
  }

  app_header = "MKStudio";

  port = new SerialPort;

  InitDialog initdialog;
  if( initdialog.exec() != QDialog::Accepted ) goto exit;

  if( !port->open() )
  { QMessageBox::critical(0,app_header,
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением." );
    goto exit;
  }

  mbmaster = new MBMaster;
  mbmaster->setTransport( port );

  helpwidget = new HelpWidget;
  helpwidget->setWindowTitle("Руководство MKStudio");
  helpwidget->resize(900,600);
  helpwidget->tb->setSource( QUrl::fromLocalFile(":/html/help/index.html" ) );

  mainwindow = new MainWindow;
  mainwindow->show();

  global_colsole = mainwindow->console;
  mainwindow->mbmasterwidget->setMBMaster( mbmaster );

  ret=application->exec();

  delete mainwindow;
  delete mbmaster;
exit:
  delete port;
  delete application;

  return ret;
}
