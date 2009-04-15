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

    QTextCodec::setCodecForTr(       QTextCodec::codecForName("Windows-1251"));
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("Windows-1251"));

    app_header = "XFiles Ftp server";

    Console::setMessageTypes( Console::AllTypes );

    mainwindow = new MainWindow;
    mainwindow -> show();

    QApplication::exec();

    delete mainwindow;
    delete qt_translator;

    return 0;
}
