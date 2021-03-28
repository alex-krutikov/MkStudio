#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"

#include <QDir>
#include <QSettings>

class Settings : public QSettings
{
public:
    Settings()
        : QSettings(QDir::homePath() + "/.mkserver_config",
                    QSettings::IniFormat)
    {
    }
};

// extern MainWindow   *mainwindow;
// extern QString       app_header;

#endif
