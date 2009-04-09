#ifndef __ABSTRACTSERIALPORT_H_
#define __ABSTRACTSERIALPORT_H_

#include <QString>

class QByteArray;

class AbstractSerialPort
{
public:
  virtual void setName( const QString &portname ) = 0;
  virtual void setSpeed( const int speed ) = 0;
  virtual void setAnswerTimeout(int timeout ) = 0;
  virtual bool open() = 0;
  virtual void close() = 0;
  virtual QString name() = 0;
  virtual int speed() = 0;
  virtual int answerTimeout() const = 0;
  virtual QString lastError() const = 0;
  virtual void resetLastErrorType()  = 0;

  virtual int query( const QByteArray &request, QByteArray &answer, int *errorcode=0) = 0;
};

#endif
