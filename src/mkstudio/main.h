#ifndef __main_h__
#define __main_h__

#include <QtGlobal>

#if defined( Q_OS_WIN32 )
  #define QSETTINGS_PARAM (qApp->applicationDirPath()+"/mkstudio.ini"),QSettings::IniFormat
#elif defined( Q_OS_UNIX )
  #define QSETTINGS_PARAM "mkstudio"
#else
  #error Wrong OS
#endif


#endif
