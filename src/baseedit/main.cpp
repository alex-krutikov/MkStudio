#include <QtGui>

#include "main.h"
#include "mainwindow.h"

//=======================================================================================
// глобальные переменные
//=======================================================================================
QApplication *application;
MainWindow   *mainwindow;
QString       app_header;

//=======================================================================================
// MAIN
//=======================================================================================
int main(int argc, char *argv[])
{
    QTranslator qt_translator;

    QApplication application(argc, argv);

    if ( qt_translator.load( ":tr/qt_ru.qm" ) )
    { application.installTranslator( &qt_translator );
    }

    app_header = "BaseEdit";

    MainWindow mainwindow;
    mainwindow.show();

    QFile file( QCoreApplication::arguments().value(1) );
    if( file.exists() )
    { mainwindow.load_file( QFileInfo(file).absoluteFilePath() );
    }

    application.exec();

    return 0;
}
