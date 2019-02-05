#include "mbudp.h"

#include "console.h"
#include "mbcommon.h"
#include "crc.h"

#include <QUdpSocket>
#include <QNetworkDatagram>

//=============================================================================
//
//=============================================================================
class MbUdpPortPrivate
{
  friend class MbUdpPort;

  QString portname;
  QString current_portname;
  QString lastError_str;

  std::unique_ptr<QUdpSocket> socket;
  QHostAddress udp_host;
  uint16_t udp_port;

  int answer_timeout;
  int packet_counter;
};

//=============================================================================
//
//=============================================================================
MbUdpPort::MbUdpPort()
  : d(std::make_unique<MbUdpPortPrivate>())
{
  d->answer_timeout = 0;
  d->packet_counter = 0;
  d->udp_port = 502;
}

//=============================================================================
//
//=============================================================================
MbUdpPort::~MbUdpPort()
{
  close();
}


//=============================================================================
//
//=============================================================================
void MbUdpPort::setName( const QString &portname )
{
  d->portname = portname;
}

//=============================================================================
//
//=============================================================================
void MbUdpPort::setSpeed( const int speed )
{
  Q_UNUSED( speed );
}

//=============================================================================
//
//=============================================================================
bool MbUdpPort::open()
{
  return true;
}

//=============================================================================
//
//=============================================================================
void MbUdpPort::close()
{
    d->socket.reset();
}

//=============================================================================
//
//=============================================================================
void MbUdpPort::resetLastErrorType()
{
}

//=============================================================================
//
//=============================================================================
int MbUdpPort::query( const QByteArray &request, QByteArray &answer, int *errorcode)
{
   Q_UNUSED(errorcode)

  int j,timeout_counter;
  QByteArray udp_header, udp_ans;
  int udp_timeout = d->answer_timeout;

  if (d->current_portname != d->portname)
  {
      d->udp_host = QHostAddress(d->portname.section(':',0,0));
      d->udp_port = d->portname.section(':',1,1).toUShort();
      if( !d->udp_port ) d->udp_port=502; // default UDP Modbus port;

      d->socket.reset();

      d->current_portname = d->portname;
  }

  if (!d->socket)
  {
      d->socket.reset(new QUdpSocket);

  }

  //-------------------------------------------
  d->packet_counter++;
  QByteArray udp_req = request;
  udp_req.chop(2);

  udp_header.resize(0);
  udp_header.append( static_cast<char>(d->packet_counter>>8)    );
  udp_header.append( static_cast<char>(d->packet_counter) );
  udp_header.append( static_cast<char>(0) );
  udp_header.append( static_cast<char>(0) );
  udp_header.append( static_cast<char>(udp_req.size() >> 8) );
  udp_header.append( static_cast<char>(udp_req.size()) );

  Console::Print( Console::ModbusPacket, "\n" );
  Console::Print( Console::ModbusPacket, "MB_UDP: udp_header  = " + QByteArray2QString( udp_header ) +"\n" );
  Console::Print( Console::ModbusPacket, "MB_UDP: udp_request = " + QByteArray2QString( udp_req ) +"\n" );

  while(d->socket->hasPendingDatagrams())
  {
      d->socket->receiveDatagram();
  }

  d->socket->writeDatagram((udp_header + udp_req), d->udp_host, d->udp_port);

  timeout_counter=0;
  while(!d->socket->hasPendingDatagrams())
  {
    d->socket->waitForReadyRead(100);
    timeout_counter += 100;
    if( timeout_counter >= udp_timeout )
    {
        Console::Print( Console::Error, "MB_UDP: Error: UDP timeout.\n" );
        return 0;
    }
  }

  QNetworkDatagram datagram = d->socket->receiveDatagram();

  if (!datagram.isValid())
  {
      Console::Print( Console::Error, "MB_UDP: Error: Received UDP datagram is invalid.\n" );
      return 0;
  }

  udp_ans = datagram.data();

  bool ok = true;
  if( udp_ans.size() < 6 ) ok = false;
  else if( udp_ans[0] != udp_header[0] ) ok = false;
  else if( udp_ans[1] != udp_header[1] ) ok = false;
  else if( udp_ans[2] || udp_ans[3]) ok = false;
  else if( (((uint8_t)udp_ans[4] << 8) | (uint8_t)udp_ans[5]) != (udp_ans.size() - 6)) ok = false;

  if (!ok)
  {
      Console::Print( Console::Error, "MB_UDP: Error: Wrong UDP anser: " + QByteArray2QString( udp_ans ) +"\n" );
      return 0;
  }

  Console::Print( Console::ModbusPacket, "MB_UDP:  udp_answer = " + QByteArray2QString( udp_ans ) +"\n" );

  udp_ans.remove(0,6);
  if( udp_ans.size() ) CRC::appendCRC16( udp_ans );
  j = qMin( udp_ans.size(), answer.size() );
  udp_ans.resize( answer.size() );
  answer = udp_ans;

  Console::Print( Console::ModbusPacket, "MB_UDP:   mb_answer = " + QByteArray2QString( answer.left(j) ) +"\n" );
  return j;
}

//=============================================================================
//
//=============================================================================
QString MbUdpPort::name() { return d->portname; } //!< Имя интерфейса

//! Таймаут ответа (в мс)
int MbUdpPort::answerTimeout() const
{
    return d->answer_timeout;
}

//=============================================================================
//
//=============================================================================
//! Задать таймаут ответа (в мс)
void MbUdpPort::setAnswerTimeout(int timeout )
{
    d->answer_timeout = timeout;
}

//=============================================================================
//
//=============================================================================
//! Описание последней ошибки
QString MbUdpPort::lastError() const
{
    return d->lastError_str;
}

