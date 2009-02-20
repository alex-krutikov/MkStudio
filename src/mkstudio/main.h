#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"

class MBMasterXML;
class QString;
class QApplication;
class MainWindow;
class SerialPort;
class HelpWidget;

#if defined( Q_OS_WIN32 )
  #define QSETTINGS_PARAM (qApp->applicationDirPath()+"/mkstudio.ini"),QSettings::IniFormat
#elif defined( Q_OS_UNIX )
  #define QSETTINGS_PARAM "mkstudio"
#else
  #error Wrong OS
#endif

extern QApplication  *application;
extern MainWindow    *mainwindow;
extern MBMasterXML      *mbmaster;
extern SerialPort    *port;
extern HelpWidget    *helpwidget;
extern QString        app_header;

#endif
