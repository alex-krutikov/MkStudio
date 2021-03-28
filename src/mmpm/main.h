#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"
#include "modbus.h"
#include "thread.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QSettings>

#ifndef DEFAULT_SERVER_URL
    #define DEFAULT_SERVER_URL "http://37.139.19.214/mmpm"
#endif

extern MainWindow *mainwindow;
extern ModbusMaster *modbus;
extern Thread *thread1;
extern HelpDialog *helpdialog;

extern QString temp_str;
extern QString temp_str2;
extern QString firmware_filename;
extern QString firmware_server_url;

extern QNetworkAccessManager network_manager;

extern volatile BYTE secret_mode;
extern volatile BYTE secret_mode2;

class Settings : public QSettings
{
public:
    Settings()
        : QSettings(QDir::homePath() + "/.mmpm_config", QSettings::IniFormat)
    {
    }
};

#endif
