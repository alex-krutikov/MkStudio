#ifndef __UTILS_H__
#define __UTILS_H__

#include <QString>
#include <QSettings>

namespace Utils {

class AppInfo
{
public:
    static void setTitle(const QString &header) {instance().applicationWindowTitle = header;}
    static QString title() {return instance().applicationWindowTitle;}
private:
    static AppInfo &instance()
    {
        static AppInfo appInfo;
        return appInfo;
    }
private:
    QString applicationWindowTitle;
};

#if defined( Q_OS_WIN32 )
class Settings : public QSettings
{
public:
    Settings()
        : QSettings((qApp->applicationDirPath() + "/mkstudio.ini"), QSettings::IniFormat)
    {
    }
};
#elif defined( Q_OS_UNIX )
class Settings : public QSettings
{
public:
    Settings()
        : QSettings("mkstudio")
    {
    }
};
#else
  #error Wrong OS
#endif



}

#endif
