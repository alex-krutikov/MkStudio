#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"

#include <QDir>
#include <QSettings>

class Settings : public QSettings
{
public:
    Settings()
        : QSettings(QDir::homePath() + "/.baseedit_config",
                    QSettings::IniFormat)
    {
    }
};

extern QApplication *appication;
extern MainWindow *mainwindow;
extern QString app_header;

#endif
