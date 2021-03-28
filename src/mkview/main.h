#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"

#include <QDir>
#include <QSettings>

class MBMasterXML;
class QString;

class Settings : public QSettings
{
public:
    Settings()
        : QSettings(QDir::homePath() + "/.mkview/mkview.conf",
                    QSettings::IniFormat)
    {
    }
};

extern QString app_header;

#endif
