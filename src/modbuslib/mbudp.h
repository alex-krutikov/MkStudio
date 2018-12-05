#ifndef __MBUDP_H__
#define __MBUDP_H__

#include "abstractserialport.h"
#include "mbl_global.h"

#include <memory>

class MbUdpPortPrivate;

//---------------------------------------------------------
//! Последовательный интерфейс передачи данных \ingroup API_group
//---------------------------------------------------------
class MBL_EXPORT MbUdpPort : public AbstractSerialPort
{
public:
  MbUdpPort();
  virtual ~MbUdpPort()  override;

  void setName( const QString &portname )  override;
  void setSpeed( const int speed )  override;
  void setAnswerTimeout(int timeout )  override;
  bool open() override;
  void close()  override;

  int query( const QByteArray &request, QByteArray &answer, int *errorcode = nullptr) override;

  QString name()  override; //!< Имя интерфейса
  int speed()  override { return 0;   } //!< Скорость интерфейса
  int answerTimeout() const  override;
  QString lastError() const override ;
  void resetLastErrorType()  override;

private:
  std::unique_ptr<MbUdpPortPrivate> d;
};

#endif
