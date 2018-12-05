#include <QScrollBar>
#include <QDebug>

#include "mainwindow.h"
#include "main.h"
#include "mbtcpserver.h"
#include "mbudpserver.h"

#include "serialport.h"
#include "crc.h"
#include "mbcommon.h"
#include "console.h"

namespace {

#if 0 // for tests only

bool testArduinoTransport()
{
  int err = 0;

  for (int i=0; i < 1000; i++)
  {
    bool ok = false;
    QByteArray ba1;

    for (int j=0; j < ((double)qrand())/RAND_MAX*1000; j++)
      ba1.append( (char) qrand() );

    QByteArray res = encodeArduinoTransport(ba1);
    QByteArray ba2 = decodeArduinoTransport(res, ok);

    if (ba1 != ba2 || !ok)
      err++;
  }

  return (err != 0);
}

#endif

}

class SerialPortEmulator : public AbstractSerialPort
{
public:
  void setName( const QString & ) {}
  void setSpeed( const int  ) {}
  void setAnswerTimeout(int ) {}
  bool open() { return true;}
  void close() {}
  QString name() { return "__EMULATOR__"; }
  int speed() { return 0; }
  int answerTimeout() const { return 0;}
  QString lastError() const { return QString(); }
  void resetLastErrorType() {}

  int query( const QByteArray &req, QByteArray &ans, int *errorcode=0)
  {
    Q_UNUSED( errorcode );

    static unsigned char base[100000];

    if( req.size() < 4 ) return 0;
    if( CRC::CRC16( req ) )
    { Console::Print( Console::ModbusError,
            "Request packet: CRC ERROR. [ " + QByteArray2QString( req ) + " ]\n" );
      return 0;
    }

    ans.resize( 300 );

    int ans_len = 0;
    int i;


    ans[0] = req[0];
    ans[1] = req[1];

    switch( req[1] ) // modbus function
    {
      // classic modbus: coils and discrete inputs read
      case( 0x01 ):
      case( 0x02 ):
      {
        if( req.size() != 8 ) return 0;

        int first_reg = (unsigned char)req[2]<<8 | (unsigned char)req[3];
        int quantity  = (unsigned char)req[4]<<8 | (unsigned char)req[5];

        if( ( quantity < 1 ) || ( quantity > 2000 ) )
        { ans[1] = req[1] | 0x80;
          ans[2] = 3; // modbus exception code: illegal data value
          ans_len = 5;
          break;
        }

        int len  = (quantity-1)/8+1;
        ans[2] = len;

        for(i=0; i<len; i++)
        { ans[3+i]=0;
        }

        for(i=0; i<quantity; i++)
        { if( base[(first_reg+i)>>3] & (1<<((first_reg+i)&0x07)) )
          { ans[3+(i>>3)] = ans[3+(i>>3)] | (1<<(i&0x07));
          }
        }
        ans_len = len + 5;
        break;
      }

      // classic modbus: holding and input registers read
      case( 0x03 ):
      case( 0x04 ):
      {
        if( req.size() != 8 ) return 0;

        int first_reg = (unsigned char)req[2]<<8 | (unsigned char)req[3];
        int quantity  = (unsigned char)req[4]<<8 | (unsigned char)req[5];

        if( ( quantity < 1 ) || ( quantity > 125 ) )
        { ans[1] = req[1] | 0x80;
          ans[2] = 3; // modbus exception code: illegal data value
          ans_len = 5;
          break;
        }

        int len  = 2*quantity;
        if( len > 250 ) break;
        ans[2] = len;
        for(i=0; i<len; i++) ans[3+i] = base[2*first_reg+(i^1)];
        ans_len = len + 5;
        break;
      }

      // classic modbus: write single coil
      case( 0x05 ):
      {
        if( req.size() != 8 ) return 0;

        int first_reg = (unsigned char)req[2]<<8 | (unsigned char)req[3];
        int value     = (unsigned char)req[4]<<8 | (unsigned char)req[5];
        ans = req;
        switch( value )
        { case( 0x0000 ):
            base[(first_reg)>>3] &= ~(1<<((first_reg)&0x07));
            ans_len = 8;
            break;
          case( 0xFF00 ):
            base[(first_reg)>>3] |= (1<<((first_reg)&0x07));
            ans_len = 8;
            break;
          default:
           ans[1] = req[1] | 0x80;
           ans[2] = 3; // modbus exception code: illegal data value
           ans_len = 5;
           break;
        }
        break;
      }

      // classic modbus: write single register
      case( 0x06 ):
      {
        if( req.size() != 8 ) return 0;

        int first_reg = (unsigned char)req[2]<<8 | (unsigned char)req[3];
        ans = req;
        base[2*first_reg  ] = req[5];
        base[2*first_reg+1] = req[4];
        ans_len = 8;
        break;
      }

      // classic modbus: write multiple coil
      case( 0x0F ):
      {
        if( req.size() < 9 ) return 0;

        int first_reg = (unsigned char)req[2]<<8 | (unsigned char)req[3];
        int quantity  = (unsigned char)req[4]<<8 | (unsigned char)req[5];

        if( ( quantity < 1 ) || ( quantity > 2000 ) )
        { ans[1] = req[1] | 0x80;
          ans[2] = 3; // modbus exception code: illegal data value
          ans_len = 5;
          break;
        }

        int len  = (quantity-1)/8+1;

        if( (unsigned int)len != (unsigned int)req[6] )
        { ans[1] = req[1] | 0x80;
          ans[2] = 3; // modbus exception code: illegal data value
          ans_len = 5;
          break;
        }

        if( req.size() != ( 9 + len ) ) return 0;

        ans[2] = req[2];
        ans[3] = req[3];
        ans[4] = req[4];
        ans[5] = req[5];

        for( i=0; i<quantity; i++ )
        {
          if( req[7+(i>>3)] & (1<<(i&0x07)) )
          { base[(first_reg+i)>>3] |= (1<<((first_reg+i)&0x07));
          } else
          { base[(first_reg+i)>>3] &= ~(1<<((first_reg+i)&0x07));
          }
        }
        ans_len = 8;
        break;
      }

      // classic modbus: write multiple registers
      case( 0x10 ):
      {
        if( req.size() < 9 ) return 0;

        int first_reg = (unsigned char)req[2]<<8 | (unsigned char)req[3];
        int quantity  = (unsigned char)req[4]<<8 | (unsigned char)req[5];

        if( ( quantity < 1 ) || ( quantity > 125 ) )
        { ans[1] = req[1] | 0x80;
          ans[2] = 3; // modbus exception code: illegal data value
          ans_len = 5;
          break;
        }

        int len  = 2*quantity;

        if( (unsigned int)len != (unsigned int)req[6] )
        { ans[1] = req[1] | 0x80;
          ans[2] = 3; // modbus exception code: illegal data value
          ans_len = 5;
          break;
        }

        if( req.size() != ( 9 + len ) ) return 0;

        ans[2] = req[2];
        ans[3] = req[3];
        ans[4] = req[4];
        ans[5] = req[5];

        for( i=0; i<len; i++ )
        {
          base[2*first_reg+i] = req[ 7+(i^1) ];
        }
        ans_len = 8;
        break;
      }


      // mikkon
      case( 0x41 ):
      case( 0x43 ):
      {
        int addr = ( (unsigned char)req[3] << 8 ) | (unsigned char)req[4];
        int len  = (unsigned char)req[5];
        ans[2] = req[2];
        ans[3] = req[3];
        ans[4] = req[4];
        switch( req[2] & 0x0F )
        {
          case( 0x00 ): // read
            ans[3] = len;
            for(i=0; i<len; i++) ans[4+i] = base[addr+i];
            ans_len = len + 6;
            break;
          case( 0x01 ): // set
            ans[5] = len;
            for(i=0; i<len; i++) base[addr+i] = req[6+i];
            for(i=0; i<len; i++) ans[6+i] = base[addr+i];
            ans_len = len + 8;
            break;
          case( 0x03 ): // and
            ans[5] = len;
            for(i=0; i<len; i++) base[addr+i] &= req[6+i];
            for(i=0; i<len; i++) ans[6+i] = base[addr+i];
            ans_len = len + 8;
            break;
          case( 0x05 ): // or
            ans[5] = len;
            for(i=0; i<len; i++) base[addr+i] |= req[6+i];
            for(i=0; i<len; i++) ans[6+i] = base[addr+i];
            ans_len = len + 8;
            break;
          case( 0x07 ): // xor
            ans[5] = len;
            for(i=0; i<len; i++) base[addr+i] ^= req[6+i];
            for(i=0; i<len; i++) ans[6+i] = base[addr+i];
            ans_len = len + 8;
            break;
          default:
           ans[1] = req[1] | 0x80;
           ans[2] = 4; // modbus exception code: slave device failure
           ans_len = 5;
           break;
        }
        break;
      }
    }

    if( ans_len == 0 ) // error: unknown function
    { ans[1] = req[0] | 0x80;
      ans[2] = 1; // modbus exception code: illegal function
      ans_len = 5;
    }

    if( ans_len > 2)
    { ans.resize( ans_len - 2 );
      CRC::appendCRC16( ans );
    }

    ans.resize( ans_len );
    return ans_len;
  }
};

class SerialPortArduionEmulator : public AbstractSerialPort
{
public:
  void setName( const QString & ) {}
  void setSpeed( const int  ) {}
  void setAnswerTimeout(int ) {}
  bool open() { return true;}
  void close() {}
  QString name() { return "__EMULATOR__"; }
  int speed() { return 0; }
  int answerTimeout() const { return 0;}
  QString lastError() const { return QString(); }
  void resetLastErrorType() {}

  int query( const QByteArray &req, QByteArray &ans, int *errorcode=0)
  {
    Q_UNUSED( errorcode );

    if( req.size() < 4 ) return 0;
    if( CRC::CRC16( req ) )
    { Console::Print( Console::ModbusError,
            "Request packet: CRC ERROR. [ " + QByteArray2QString( req ) + " ]\n" );
      return 0;
    }

    ans.resize( 300 );

    int ans_len = 0;

    ans.resize( ans_len );
    return ans_len;
  }
};

//#######################################################################################
// главное окно
//#######################################################################################
MainWindow::MainWindow()
{
  setupUi( this );
  setWindowTitle( app_header );

  Settings settings;
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(600, 400)).toSize();
  restoreState( settings.value("mainwindow").toByteArray() );
  resize(size);
  move(pos);

  //----------------------------------------------------------------------
  cb_protocol->clear();
  cb_protocol->addItem("UDP");
  cb_protocol->addItem("TCP");
  //----------------------------------------------------------------------
  int i;
  QString str,str2;
  cb_portname->clear();
  QStringList ports = SerialPort::queryComPorts();
  foreach( str, ports )
  { str2=str;
    str2.replace(';',"        ( ");
    cb_portname->addItem( str2+" )", str.section(';',0,0) );
  }
  cb_portname->addItem( u8"Эмулятор", "__EMULATOR__" );
  cb_portname->addItem( u8"Эмулятор Arduino", "__EMULATOR_ARDUINO__" );
  i = cb_portname->findData( settings.value("portname").toString() );
  if( i >= 0 ) cb_portname->setCurrentIndex(i);
  //----------------------------------------------------------------------
  cb_portspeed -> addItems( QStringList() << "300"
                                          << "600"
                                          << "1200"
                                          << "2400"
                                          << "4800"
                                          << "9600"
                                          << "19200"
                                          << "38400"
                                          << "57600"
                                          << "115200"
                                          << "230400"
                                          << "460800"
                                          << "921600" );
  i = cb_portspeed->findText( settings.value("portspeed","115200").toString() );
  if( i >= 0 ) cb_portspeed->setCurrentIndex(i);
  //----------------------------------------------------------------------
  le_timeout->setText( settings.value("timeout","100").toString() );
  //----------------------------------------------------------------------
  le_tcp_port->setText( settings.value("tcp_port","502").toString() );
  //----------------------------------------------------------------------
  cb_mb_packet->setChecked(  settings.value("show_mb_packets").toBool() );
  //----------------------------------------------------------------------

  connect( cb_portname,  SIGNAL( currentIndexChanged(int) ), this, SLOT( settingsChanged() ));
  connect( cb_portspeed, SIGNAL( currentIndexChanged(int) ), this, SLOT( settingsChanged() ));
  connect( le_timeout,   SIGNAL( textChanged(const QString&) ), this, SLOT( settingsChanged() ));
  connect( cb_protocol,  SIGNAL( currentIndexChanged(int)), this, SLOT( settingsChanged() ));
  connect( le_tcp_port,  SIGNAL( textChanged(const QString&) ), this, SLOT( settingsChanged() ));
  connect( cb_mb_packet, SIGNAL( toggled (bool) ),              this, SLOT( settingsChanged() ));

  te->setMaximumBlockCount( 10000 );
  te->setFont( QFont("Courier",8 ) );
  te->setWordWrapMode( QTextOption::NoWrap );

  settingsChanged();
  te->setFocus();
  startTimer( 10 );
}
//==============================================================================
 //
 //==============================================================================
MainWindow::~MainWindow()
{
}
//==============================================================================
//
//==============================================================================
void MainWindow::timerEvent( QTimerEvent* )
{
  console_update();
}

//==============================================================================
//
//==============================================================================
void MainWindow::settingsChanged()
{
  Settings settings;

  QString portname = cb_portname->itemData( cb_portname->currentIndex() ).toString();
  int portspeed    = cb_portspeed->currentText().toInt();
  int timeout      = le_timeout->text().toInt();
  int tcpport      = le_tcp_port->text().toInt();
  bool show_mb_packets   = cb_mb_packet->isChecked();

  settings.setValue("portname",   portname  );
  settings.setValue("portspeed",  portspeed );
  settings.setValue("timeout",   timeout );
  settings.setValue("tcp_port",  tcpport );
  settings.setValue("show_mb_packets",  show_mb_packets );

  if( !serialport.get() || sender() == cb_portname )
  {
    tcpserver.reset();
    if( portname == "__EMULATOR__" )
    { serialport.reset(new SerialPortEmulator);
    } else if( portname == "__EMULATOR_ARDUINO__" )
    { serialport.reset(new SerialPortArduionEmulator);
    } else
    { serialport.reset(new SerialPort);
    }
  }

  serialport -> setName( portname );
  serialport -> setSpeed( portspeed );
  serialport -> setAnswerTimeout( timeout );
  serialport -> open();

  Console::setMessageTypes( Console::ModbusPacket, show_mb_packets );

  if( !tcpserver.get() || sender() == le_tcp_port  || sender() == cb_protocol)
  {
      if (cb_protocol->currentText() == "TCP")
      {
        tcpserver.reset();
        tcpserver.reset(new ModbusTcpServer( this, serialport.get(), tcpport ));
      }
      else if (cb_protocol->currentText() == "UDP")
      {
        tcpserver.reset();
        tcpserver.reset(new ModbusUdpServer( this, serialport.get(), tcpport ));
      }
  }
}


//==============================================================================
//
//==============================================================================
void MainWindow::console_update()
{
  QString str = Console::takeMessage();
  if( !str.isEmpty() ) te->insertPlainText( str );
  QScrollBar *sb = te->verticalScrollBar();
  if( ! sb->isSliderDown() )	sb->setValue( sb->maximum() );
}

//==============================================================================
//
//==============================================================================
void MainWindow::closeEvent ( QCloseEvent * event )
{
  Q_UNUSED( event );

  Settings settings;
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("mainwindow", saveState() );
}
