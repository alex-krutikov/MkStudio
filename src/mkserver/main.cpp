#include <QtGui>

#include "main.h"
#include "mainwindow.h"
#include "console.h"

//=======================================================================================
// глобальные переменные
//=======================================================================================
MainWindow   *mainwindow;
QString       app_header;

//=======================================================================================
// MAIN
//=======================================================================================
int main(int argc, char *argv[])
{
    //QApplication::setStyle("cleanlooks");
    //QApplication::setStyle("plastique");

    QApplication app(argc, argv);

    QTranslator *qt_translator = new QTranslator;
    if ( qt_translator->load( ":tr/qt_ru.qm" ) )
    { QApplication::installTranslator( qt_translator );
    }

    app_header = "Modbus TCP Server";

    Console::setMessageTypes( Console::AllTypes );

    mainwindow = new MainWindow;
    mainwindow -> show();

    QApplication::exec();

    delete mainwindow;
    delete qt_translator;

    return 0;
}
