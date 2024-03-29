﻿#include "console.h"
#include "main.h"
#include "mainwindow.h"
#include "misc.h"
#include "modbus.h"
#include "thread.h"

#include <QDate>
#include <QTime>
#include <QTranslator>

MainWindow *mainwindow;
ModbusMaster *modbus;
Thread *thread1;
HelpDialog *helpdialog;

QString temp_str;
QString temp_str2;
QString firmware_filename;
QString firmware_server_url;

QNetworkAccessManager network_manager;

volatile BYTE secret_mode;
volatile BYTE secret_mode2;

//==============================================================================
// MAIN
//==============================================================================
int main(int argc, char *argv[])
{
    int ret = 0;
    make_crc32table();

    QCoreApplication::setOrganizationName("Umikon");
    QCoreApplication::setApplicationName("MMpM");

    QApplication app(argc, argv);

    QTranslator *qt_translator = new QTranslator;
    if (qt_translator->load(":tr/qt_ru.qm"))
    {
        app.installTranslator(qt_translator);
    }

    Console::setTimestampShow(false);

    helpdialog = new HelpDialog;
    modbus = new ModbusMaster;
    thread1 = new Thread;

    // !!!!!!!!!!!!!!!!!!! ВНИМАНИЕ !!!!!!! 20060710 ksy
    // необходимо изменить алгоритм секретного режима (из-за возможности
    // использования ctrl/e или истории в far, f3 в командной строке)
    //
    // надо: (день недели)*10+(десятки минут)
    // например: вторник, 10:23:15 -> пароль 22
    //           пятница, 12:09:50 -> пароль 50
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    QStringList sl = QCoreApplication::arguments();
    if (sl.contains(QString("-%1%2")
                        .arg(QDate::currentDate().dayOfWeek())
                        .arg(QTime::currentTime().minute() / 10)))
    {
        secret_mode = 1;
        // Console::Print( "secret_mode\n" );
    }
    if (sl.contains("-123456"))
    {
        secret_mode2 = 1;
        // Console::Print( "secret_mode2\n" );
    }

    secret_mode = 1;
    secret_mode2 = 1;

    InitDialog dialog;
    if (!dialog.isPortsFound()) goto exit;
    if (dialog.exec() == QDialog::Rejected) goto exit;

    mainwindow = new MainWindow;
    mainwindow->show();

    mainwindow->setWindowTitle(mainwindow->windowTitle() + " - "
                               + modbus->port());

    ret = app.exec();

    delete mainwindow;
exit:
    delete thread1;
    delete modbus;
    delete helpdialog;

    return ret;
}
