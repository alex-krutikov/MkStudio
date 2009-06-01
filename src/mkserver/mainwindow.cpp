#include <QtGui>
#include <QtNetwork>

#include "mainwindow.h"
#include "main.h"
#include "server.h"

#include "serialport.h"
#include "crc.h"
#include "mbcommon.h"
#include "console.h"

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

    ans.resize( 300 );

    int addr = ( (unsigned char)req[3] << 8 ) | (unsigned char)req[4];
    int len  = (unsigned char)req[5];
    int ans_len = 0;
    int i;

    ans[0] = req[0];
    ans[1] = req[1];
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
        ans_len = len + 8;
        break;
      case( 0x03 ): // and
        ans[5] = len;
        for(i=0; i<len; i++) base[addr+i] &= req[6+i];
        ans_len = len + 8;
        break;
      case( 0x05 ): // or
        ans[5] = len;
        for(i=0; i<len; i++) base[addr+i] |= req[6+i];
        ans_len = len + 8;
        break;
      case( 0x07 ): // xor
        ans[5] = len;
        for(i=0; i<len; i++) base[addr+i] ^= req[6+i];
        ans_len = len + 8;
      break;
    }

    if( ans_len > 2)
    { ans.resize( ans_len - 2 );
      CRC::appendCRC16( ans );
    }

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
  tcpserver = 0;
  serialport = 0;

  QSettings settings( QSETTINGS_PARAM );
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(600, 400)).toSize();
  restoreState( settings.value("mainwindow").toByteArray() );
  resize(size);
  move(pos);

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
  cb_portname->addItem( "Ёмул€тор", "__EMULATOR__" );
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
 delete serialport;
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
  QSettings settings( QSETTINGS_PARAM );

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

  if( !serialport || sender() == cb_portname )
  { delete tcpserver;
    tcpserver=0;
    delete serialport;
    if( portname == "__EMULATOR__" )
    { serialport = new SerialPortEmulator;
    } else
    { serialport = new SerialPort;
    }
  }

  serialport -> setName( portname );
  serialport -> setSpeed( portspeed );
  serialport -> setAnswerTimeout( timeout );
  serialport -> open();

  Console::setMessageTypes( Console::ModbusPacket, show_mb_packets );

  if( !tcpserver || sender() == le_tcp_port )
  { delete tcpserver;
    tcpserver = new ModbusTcpServer( this, serialport, tcpport );
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

  QSettings settings( QSETTINGS_PARAM );
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("mainwindow", saveState() );
}
