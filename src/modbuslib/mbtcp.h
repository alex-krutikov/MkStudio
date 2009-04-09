#ifndef __MBTCP_H__
#define __MBTCP_H__

#include "abstractserialport.h"

class MbTcpPortPrivate;

//---------------------------------------------------------
//! Последовательный интерфейс передачи данных \ingroup API_group
//---------------------------------------------------------
class MbTcpPort : public AbstractSerialPort
{
public:
  MbTcpPort();
  virtual ~MbTcpPort();

  void setName( const QString &portname );
  void setSpeed( const int speed );
  inline void setAnswerTimeout(int timeout );
  bool open();
  void close();

  int query( const QByteArray &request, QByteArray &answer, int *errorcode=0);

  QString name(); //!< Имя интерфейса
  int speed() { return 0;   } //!< Скорость интерфейса
  int answerTimeout() const;
  QString lastError() const;
  void resetLastErrorType();

  static QStringList queryComPorts();

  bool console_out_packets;
private:
//  int answer_timeout;
//  int current_answer_timeout;
//  QString portname;
//  int     portspeed;
  MbTcpPortPrivate *d;
//  QString lastError_str;
};

#endif
