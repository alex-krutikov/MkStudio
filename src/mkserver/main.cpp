#include "main.h"
#include "console.h"
#include "mainwindow.h"

#include <QTranslator>

//=======================================================================================
// MAIN
//=======================================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator qt_translator;
    if (qt_translator.load(":tr/qt_ru.qm"))
    {
        QApplication::installTranslator(&qt_translator);
    }

    Console::setMessageTypes(Console::AllTypes);

    MainWindow mainwindow;
    mainwindow.show();

    QApplication::exec();

    return 0;
}
