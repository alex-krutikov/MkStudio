#include <QtGui>
#include <QtNetwork>

#include "mainwindow.h"
#include "main.h"
#include "server.h"

#include "serialport.h"
#include "crc.h"
#include "mbcommon.h"
#include "console.h"

//#######################################################################################
// главное окно
//#######################################################################################
MainWindow::MainWindow()
{
  setupUi( this );
  setWindowTitle( app_header );

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

  connect( cb_portname,  SIGNAL( currentIndexChanged(int) ), this, SLOT( settingsChanged() ));
  connect( cb_portspeed, SIGNAL( currentIndexChanged(int) ), this, SLOT( settingsChanged() ));

  te->setMaximumBlockCount( 10000 );

  serialport = new SerialPort;

  te->setFont( QFont("Courier",8 ) );
  te->setWordWrapMode( QTextOption::NoWrap );

  settingsChanged();

  te->setFocus();

  //----------------------------------------------------------------------
  tcpserver = new ModbusTcpServer( this, serialport );

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

  settings.setValue("portname",   portname  );
  settings.setValue("portspeed",  portspeed );

  serialport -> setName( portname );
  serialport -> setSpeed( portspeed );
  serialport -> setAnswerTimeout( 1000 );
  serialport -> open();
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
  QSettings settings( QSETTINGS_PARAM );
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("mainwindow", saveState() );
}
