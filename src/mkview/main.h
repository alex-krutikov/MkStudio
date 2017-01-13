#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"

class MBMasterXML;
class QString;

class Settings : public QSettings
{
public:
    Settings()
        : QSettings(QDir::homePath() + "/.mkview_config", QSettings::IniFormat)
    {
    }
};

extern QString        app_header;

#endif
