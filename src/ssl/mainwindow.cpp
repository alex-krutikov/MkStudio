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

  //Console::Print( Console::Debug, "thread id="+QString::number( (int)currentThreadId() ) +"\n" );

  server = new Server;

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
