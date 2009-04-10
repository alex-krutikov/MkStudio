#ifndef __main_h__
#define __main_h__

#include <QtGui>

#include "mainwindow.h"

#define QSETTINGS_PARAM (qApp->applicationDirPath()+"/ftp.ini"),QSettings::IniFormat

extern MainWindow   *mainwindow;
extern QString       app_header;

#endif
