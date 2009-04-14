#ifndef __MBTCP_H__
#define __MBTCP_H__

#include "abstractserialport.h"
#include "mbl_global.h"

class MbTcpPortPrivate;

//---------------------------------------------------------
//! ���������������� ��������� �������� ������ \ingroup API_group
//---------------------------------------------------------
class MBL_EXPORT MbTcpPort : public AbstractSerialPort
{
public:
  MbTcpPort();
  virtual ~MbTcpPort();

  void setName( const QString &portname );
  void setSpeed( const int speed );
  void setAnswerTimeout(int timeout );
  bool open();
  void close();

  int query( const QByteArray &request, QByteArray &answer, int *errorcode=0);

  QString name(); //!< ��� ����������
  int speed() { return 0;   } //!< �������� ����������
  int answerTimeout() const;
  QString lastError() const;
  void resetLastErrorType();

  static QStringList queryComPorts();

  bool console_out_packets;
private:
  MbTcpPortPrivate *d;
};

#endif
