#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"

class MBMasterXML;
class QString;

#if defined( Q_OS_WIN32 )
  #define QSETTINGS_PARAM (qApp->applicationDirPath()+"/mkview.ini"),QSettings::IniFormat
#elif defined( Q_OS_UNIX )
  #define QSETTINGS_PARAM "mkview"
#else
  #error Wrong OS
#endif

extern QString        app_header;

#endif
