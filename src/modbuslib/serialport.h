#ifndef __SERIALPORT_H_
#define __SERIALPORT_H_

#include <QtCore>

#include "mbl_global.h"
#include "abstractserialport.h"


class SerialPortPrivate;

//---------------------------------------------------------
//! ���������������� ��������� �������� ������ \ingroup API_group
//---------------------------------------------------------
class MBL_EXPORT SerialPort : public AbstractSerialPort
{
  friend class SerialPortPrivate;
public:
  SerialPort();
  virtual ~SerialPort();

/** @name ���������
 *
 */
//@{
  void setName( const QString &portname );
  void setSpeed( const int speed );
  inline void setAnswerTimeout(int timeout );
  bool open();
  void close();
//@}

/** @name ������
 *
 */
//@{
  int query( const QByteArray &request, QByteArray &answer, int *errorcode=0);
//@}

/** @name ����������
 *
 */
//@{

  QString name() { return portname; } //!< ��� ����������
  int speed() { return portspeed;   } //!< �������� ����������
  inline int answerTimeout() const;
  inline QString lastError() const;
  void resetLastErrorType();
//@}
  static QStringList queryComPorts();

  bool console_out_packets;
private:
  int answer_timeout;
  int current_answer_timeout;
  QString portname;
  int     portspeed;
  SerialPortPrivate *d;
  QString lastError_str;
};

//! ������� ������ (� ��)
inline int SerialPort::answerTimeout() const
{ return answer_timeout;
}

//! ������ ������� ������ (� ��)
inline void SerialPort::setAnswerTimeout(int timeout )
{ answer_timeout = timeout;
}

//! �������� ��������� ������
inline QString SerialPort::lastError() const
{ return lastError_str;
}

#endif
