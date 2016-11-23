#ifndef __main_h__
#define __main_h__

#include <QtGui>

#include "mainwindow.h"

#if defined( Q_OS_WIN32 )
  #define QSETTINGS_PARAM (qApp->applicationDirPath()+"/baseedit.ini"),QSettings::IniFormat
#elif defined( Q_OS_UNIX )
  #define QSETTINGS_PARAM "baseedit"
#else
  #error Wrong OS
#endif


extern QApplication *appication;
extern MainWindow   *mainwindow;
extern QString       app_header;

#endif
