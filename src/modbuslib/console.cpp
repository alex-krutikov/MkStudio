#include "console.h"

#include <QMutex>
#include <QTime>

static QMutex mutex;
static QString message;
static int message_types;
static bool show_timestamp = true;


//=============================================================================
/// Напечатать сообщение
//=============================================================================
void Console::Print(MessageType mtype, const QString &message)
{
    QString text = message;

    if (show_timestamp)
    {
        const QString timeStamp
            = QTime::currentTime().toString("hh:mm:ss.zzz");
        text = QString("%1  %2").arg(timeStamp).arg(message);
    }

    QMutexLocker locker(&mutex);
    if (mtype & message_types)
    {
        ::message += text;
        if (::message.length() > 16384) ::message.remove(0, 4096);
    }
}

//=============================================================================
/// Возвратить все сообщения и очистить буфер
//=============================================================================
QString Console::takeMessage()
{
    QMutexLocker locker(&mutex);
    QString ret = message;
    message.clear();
    return ret;
}

//=============================================================================
/// Установить типы отображаемых сообщений
//
/// \param type   тип сообщения
/// \param status true - установить отображение, false - убрать отображение
//=============================================================================
void Console::setMessageTypes(int type, bool status)
{
    QMutexLocker locker(&mutex);
    if (status)
    { // set
        message_types |= type;
    } else
    { // clear
        message_types &= ~type;
    }
}

//=============================================================================
/// Маска типов отображаемых сообщений
//
/// \return маска типов отображаемых сообщений
//=============================================================================
int Console::messageTypes()
{
    return message_types;
}

//=============================================================================
/// Добавлять или нет метку времени в начало каждой строки
//
/// \param show true - установить отображение, false - убрать отображение
//=============================================================================
void Console::setTimestampShow(bool show)
{
    show_timestamp = show;
}
