#ifndef __main_h__
#define __main_h__

#include "mainwindow.h"
#include "modbus.h"
#include "thread.h"

extern MainWindow   *mainwindow;
extern ModbusMaster *modbus;
extern Thread       *thread1;
extern HelpDialog   *helpdialog;

extern QString temp_str;
extern QString temp_str2;
extern QString temp_str3;

extern volatile BYTE secret_mode;
extern volatile BYTE secret_mode2;

#define QSETTINGS_PARAM (qApp->applicationDirPath()+"/mmpm.ini"),QSettings::IniFormat

#endif
