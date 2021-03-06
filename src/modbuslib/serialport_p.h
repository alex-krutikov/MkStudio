﻿#ifndef __SERIALPORTPRIVATE__H__
#define __SERIALPORTPRIVATE__H__

#include <QMutex>
#include <QVector>

#ifdef Q_OS_WIN32
    #ifndef UNICODE
        #define UNICODE
    #endif

    #ifndef _UNICODE
        #define _UNICODE
    #endif

    #include <windows.h>
#endif

class SerialPort;
class QStringList;
class QString;
class QByteArray;
class QStringList;
class QWidget;

struct st_XBeeRoute
{
    int a1;
    int a2;
    int addr;
};

class SerialPortPrivate
{
    friend class SerialPort;
#ifdef Q_OS_WIN32
    friend LRESULT CALLBACK SerialPortWndProc(HWND hwnd, UINT Message,
                                              WPARAM wparam, LPARAM lparam);
#endif

    SerialPortPrivate(SerialPort *sp);
    virtual ~SerialPortPrivate();

    bool open();
    bool reopen();
    void close();

    int query(const QByteArray &request, QByteArray &answer, int *errorcode);

    int queryXBee(const QByteArray &request, QByteArray &answer, int *errorcode,
                  int xbee_addr);

    static QStringList queryComPorts();

    QVector<st_XBeeRoute> xbee_route_table;
    QMutex mutex;

#ifdef Q_OS_WIN32
    void usleep(DWORD us);

    HANDLE hport;
    LARGE_INTEGER freq;
    bool perf_cnt_ok;
    HWND hwnd;
#endif
#ifdef Q_OS_UNIX
    int fd;
#endif
    SerialPort *sp;
    int last_error_id;
};

#endif
