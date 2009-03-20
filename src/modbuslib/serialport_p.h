#ifndef __SERIALPORTPRIVATE__H__
#define __SERIALPORTPRIVATE__H__

#ifdef Q_OS_WIN32
  #include <windows.h>
#endif

class SerialPort;
class QStringList;
class QString;
class QByteArray;
class QStringList;
class QWidget;

class SerialPortPrivate
{
  friend class SerialPort;
#ifdef Q_OS_WIN32
  friend LRESULT CALLBACK SerialPortWndProc(HWND hwnd, UINT Message, WPARAM wparam,LPARAM lparam);
#endif

  SerialPortPrivate( SerialPort *sp);
  ~SerialPortPrivate();

  bool open();
  bool reopen();
  void close();

  int  query( const QByteArray &request, QByteArray &answer, int *errorcode);

  static QStringList queryComPorts();

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
