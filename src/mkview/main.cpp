#include "mainwindow.h"
#include "consolewidget.h"
#include "main.h"
#include "interface.h"

#include <QTranslator>
#include <QMessageBox>
#include <QPluginLoader>

QString       app_header;

#ifdef Q_OS_WIN32
  #define DLL_MASK ".dll"
#endif
#ifdef Q_OS_UNIX
  #define DLL_MASK ".so"
#endif

//==============================================================================
// MAIN
//==============================================================================
int main(int argc, char *argv[])
{
  int ret=0,ret2;
#ifdef Q_OS_UNIX
  QApplication::setStyle("cleanlooks");
#endif

  QApplication application( argc, argv);

  QTranslator *qt_translator = new QTranslator;
  if ( qt_translator->load( ":tr/qt_ru.qm" ) )
  { application.installTranslator( qt_translator );
  }
  app_header = "MKView";

  InitDialog *initdialog;
  initdialog = new InitDialog(0);
  if( initdialog->portname().isEmpty() )
  { QMessageBox::critical( 0, app_header, "В вашей системе не обнаружено последовательных портов.\n\n"
                                          "Работа программы невозможна." );
    return 0;
  }

  if( QApplication::arguments().contains("--force") )
  { ret2 = QDialog::Accepted;
  } else
  { ret2 = initdialog->exec();
    if (ret2 != QDialog::Accepted)
    {
        return 0;
    }
    initdialog->setEnabled( false );
    initdialog->show();
  }

  if( ret2 == QDialog::Accepted )
  {
    //--------------------------------------------------------
    if( initdialog->filename().contains( DLL_MASK ) )
    {
      QObject *plugin;
      MKViewPluginInterface *ptr;
      QPluginLoader pluginLoader( initdialog->filename() );
      plugin = pluginLoader.instance();
      if( plugin )
      { ptr = qobject_cast<MKViewPluginInterface*>(plugin);
        if( ptr )
        { initdialog->hide();
          ptr->mainWindow( 0, initdialog->portname(),
                              initdialog->portspeed(),
                              initdialog->int_module_node(),
                              initdialog->int_module_subnode()   );
          if( !pluginLoader.unload() )
          { QMessageBox::critical( 0,app_header,"Ошибка при выгрузке плагина.");
          }
        }
      }
    }
    //--------------------------------------------------------
    if( initdialog->filename().contains(".xml") )
    {
      qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );

      MainWindowXml mainwindow(0,  initdialog->portname(),
                                   initdialog->portspeed(),
                                   initdialog->host(),
                                   initdialog->filename(),
                                   initdialog->int_module_node(),
                                   initdialog->int_module_subnode() );
      mainwindow.setWindowTitle( initdialog->module_name() + "  - " + app_header );
      mainwindow.show();
      delete initdialog;
      initdialog=0;
      qApp->restoreOverrideCursor();
      ret = application.exec();
    }
  }
  delete initdialog;

  return ret;
}
