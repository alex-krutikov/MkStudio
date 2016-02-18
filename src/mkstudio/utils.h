#ifndef __UTILS_H__
#define __UTILS_H__

#include <QString>

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

}

#endif
