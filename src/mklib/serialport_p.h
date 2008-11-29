#ifndef __SERIALPORTPRIVATE__H__
#define __SERIALPORTPRIVATE__H__

#ifdef Q_OS_WIN32
  #include <windows.h>
  #include <QWidget>
#endif

class SerialPort;
class QStringList;
class QString;
class QByteArray;
class QStringList;
class QWidget;

#ifdef Q_OS_WIN32
#include <QWidget>
class SerialPortPrivate;
class SerialPortPrivateWidget : public QWidget
{
protected:
  bool winEvent ( MSG * message, long * result );
public:
  SerialPortPrivate *spp;
};
#endif

class SerialPortPrivate
{
  friend class SerialPort;
  friend class SerialPortPrivateWidget;

  SerialPortPrivate( SerialPort *sp);
  ~SerialPortPrivate();

  bool open();
  bool reopen();
  void close();

  int  request( const QByteArray &request, QByteArray &answer, int *errorcode);

  static QStringList queryComPorts();

#ifdef Q_OS_WIN32
  void usleep(DWORD us);
  
  HANDLE hport;
  SerialPortPrivateWidget window;
  HDEVNOTIFY notify_handle;
  LARGE_INTEGER freq;
  bool perf_cnt_ok;
#endif
#ifdef Q_OS_UNIX
  int fd;
#endif
  SerialPort *sp;
  int last_error_id;
};

#endif
