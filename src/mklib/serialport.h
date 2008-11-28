#ifndef __SERIALPORT_H_
#define __SERIALPORT_H_

#include <QtGui>

#include "ui/ui_serialportdialog.h"

class SerialPort;
class SerialPortPrivate;

//---------------------------------------------------------
//  Диалог выбора и настройки порта
//---------------------------------------------------------
class SerialPortSetupDialog : public QDialog,
                              public Ui::SerialPortDialog
{
Q_OBJECT
public:
  SerialPortSetupDialog();
  SerialPort *port;
private:
  void accept();
};

//---------------------------------------------------------
// Порт на транспортном уровне
//---------------------------------------------------------
class SerialPort
{
  friend class SerialPortPrivate;
public:
  SerialPort();
  virtual ~SerialPort();

  void setName( const QString &portname );
  void setSpeed( const int speed );
  QString getName() { return portname; }
  int getSpeed() { return portspeed;   }
  inline int answerTimeout() const;
  inline void setAnswerTimeout(int timeout );
  bool open();
  void close();

  int setupDialog();
  int request( const QByteArray &request, QByteArray &answer, int *errorcode=0);

  static QStringList queryComPorts();
  QString lastError() const { return lastError_str; }
  void reset_current_error();

  bool console_out_packets;
private:
  int answer_timeout;
  int current_answer_timeout;
  QString portname;
  int     portspeed;
  SerialPortPrivate *d;
  QString lastError_str;
};

inline int SerialPort::answerTimeout() const
{ return answer_timeout;
}

inline void SerialPort::setAnswerTimeout(int timeout )
{ answer_timeout = timeout;
}

#endif
