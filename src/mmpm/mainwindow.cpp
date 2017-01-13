#include "mainwindow.h"

#include "modbus.h"
#include "main.h"

#include "serialport.h"
#include "console.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QStatusBar>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QSettings>

//==============================================================================
// Главное окно
//==============================================================================
MainWindow::MainWindow()
{
  setupUi( this );
  setWindowTitle( tr("Менеджер микропрограмм модулей ПТК УМИКОН") );
  te->setLineWrapMode( QPlainTextEdit::NoWrap );
  pb8->hide();
  pb11->hide();

  Console::setMessageTypes(   Console::Error
                            | Console::Warning
                            | Console::Information
                            | Console::Debug  );
  flag=0;
  flag2=0;
  flag3=0;
  flag4=0;

  //------- геометрия окна по центру экрана  -----------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  rw.setSize( QSize( 800, 600 ) );
  if( rw.height() > ( rs.height() - 70 ) )
  { rw.setHeight(  rs.height() - 50 );
  }
  if( rw.width() > ( rs.width() - 50 ) )
  { rw.setWidth(  rs.width() - 70 );
  }
  rw.moveCenter( rs.center() );
  setGeometry( rw );

  //------- строка статуса -----------------
  statusbar = new QStatusBar();
  status_requests = new QLabel;
  status_answers  = new QLabel;
  status_errors   = new QLabel;
  statusbar->addPermanentWidget( status_requests );
  statusbar->addPermanentWidget( status_answers  );
  statusbar->addPermanentWidget( status_errors   );
  setStatusBar( statusbar );

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(10);

  pb_value=0;
  flag=0;
}

//==============================================================================
// Главное окно -- установка адреса модуля
//==============================================================================
void MainWindow::set_node()
{
  bool ok;
  BYTE ret;

  if( secret_mode )
  { ret = QInputDialog::getInt( this, tr("MMpM"),
                 tr("Адрес модуля:"), thread1->node, 1, 127, 1,&ok);
    if( !ok )
    { thread1->mode=0;
      return;
    }
    thread1->node = ret;
    Console::Print( Console::Information, tr("Адрес модуля: %1\n").arg(thread1->node));
  } else
  { thread1->node = 127;
  }
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Прочитать"
//==============================================================================
void MainWindow::on_pb1_clicked()
{
  if( thread1->isRunning() ) return;
  thread1->mode=1;
  set_node();
  thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Записать"
//==============================================================================
void MainWindow::on_pb2_clicked()
{
  if( thread1->isRunning() ) return;
  thread1->mode=2;
  set_node();
  thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Опции"
//==============================================================================
void MainWindow::on_pb3_clicked()
{
  OptDialog dialog(this);
  dialog.exec();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Сброс"
//==============================================================================
void MainWindow::on_pb4_clicked()
{
  if( thread1->isRunning() ) return;
  thread1->mode=3;
  set_node();
  thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Информация"
//==============================================================================
void MainWindow::on_pb5_clicked()
{
  if( thread1->isRunning() ) return;
  thread1->mode=4;
  set_node();
  thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Проверить"
//==============================================================================
void MainWindow::on_pb6_clicked()
{
  if( thread1->isRunning() ) return;
  thread1->mode=5;
  set_node();
  thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Помощь"
//==============================================================================
void MainWindow::on_pb7_clicked()
{
  //HelpDialog *dialog = new HelpDialog( this );
  helpdialog->show();
  helpdialog->raise();
  helpdialog->activateWindow();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "СТЕРЕТЬ"
//==============================================================================
void MainWindow::on_pb8_clicked()
{
  if( thread1->isRunning() ) return;
  thread1->mode=6;
  set_node();
  thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Очистить"
//==============================================================================
void MainWindow::on_pb10_clicked()
{
  te->clear();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Запись загрузчика"
//==============================================================================
void MainWindow::on_pb11_clicked()
{
  if( QMessageBox::warning( this,"MMpM",
                    "<big><b>ВНИМАНИЕ!</b>"
                    "<p>Запись загрузчика является потенциально опасной операцией! "
                    "Вы должны четко представлять, что делаете. В случае записи некорректного или "
                    "не соответствующего типу модуля загрузчика, а также при отключении питания во время "
                    "перезаписи, восстановление модуля может быть осуществленно <b>только у изготовителя!</b></p>"
                    "<p>Продолжить?</p></big>",
                QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel )
      != QMessageBox::Yes ) return;

  if( thread1->isRunning() ) return;
  thread1->mode=7;
  set_node();
  thread1->start();

}

//==============================================================================
//
//==============================================================================
void MainWindow::closeEvent( QCloseEvent* )
{
}

//==============================================================================
//
//==============================================================================
static WORD irnd( WORD value )
{ //                       8#31105    8#15415   8#77777
  return ((( (DWORD)value * 0x3245 ) + 0x1B0D) & 0x7FFF );
}


//==============================================================================
//
//==============================================================================
static DWORD passw2answer( DWORD pass )
{
  const DWORD pow10[10] = { 1000000000,
                             100000000,
                              10000000,
                               1000000,
                                100000,
                                 10000,
                                  1000,
                                   100,
                                    10,
                                     1 };
  int i;
  DWORD sum;

  sum=0;
  for( i=0; i<10; i++ )
  { while( pass >= pow10[i] )
    { sum++;
      pass -= pow10[i];
    }
  }
  return sum;
}

//==============================================================================
// Главное окно -- таймер 100 ms
//==============================================================================
void MainWindow::update()
{
  bool ok;

  status_requests -> setText( QString(" Запросы: %1 ").arg( modbus -> req_counter ) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg( modbus -> ans_counter ) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg( modbus -> err_counter ) );

  progressBar->setValue( pb_value );

  if( thread1->mode )
  { pb1->setEnabled( false );
    pb2->setEnabled( false );
    pb4->setEnabled( false );
    pb5->setEnabled( false );
    pb6->setEnabled( false );
    pb8->setEnabled( false );
    pb11->setEnabled( false );
  } else
  { pb1->setEnabled( true );
    pb2->setEnabled( true );
    pb4->setEnabled( true );
    pb5->setEnabled( true );
    pb6->setEnabled( true );
    pb8->setEnabled( true );
    pb11->setEnabled( true );
    pb_value=0;
  }

  if( flag )
  { timer->stop();
      if(   secret_mode2 == 1 )
      { if( secret_mode  == 1 ) { temp_str2.setNum( irnd(         temp_str2.toULong() ) ); }
        else                    { temp_str2.setNum( passw2answer( temp_str2.toULong() ) ); }
      }
      else                    { temp_str2.clear(); }

    temp_str = QInputDialog::getText(this, tr("MMpM"),
                  temp_str, QLineEdit::Normal, temp_str2, &ok);
    if( !ok ) temp_str.clear();
    timer->start();
    flag=0;
  }

  if( flag2 == 1 )
  { QMessageBox::StandardButton button;
    timer->stop();
    button = QMessageBox::warning( this,"MMpM",
                    "<p>Файл микропрограммы подготовлен в сторонней программе и не соответствует формату MMpM !"
                    "Тем не менее, его можно попробовать безопасно записать в модуль.</p>"
                    "<p>Продолжить?</p>",
                    QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel );
    timer->start();
    flag2 = 0;
    if( button == QMessageBox::Yes )
    { flag2 = 2;
    } else
    { Console::Print(Console::Information, "Отмена\n");
    }
  }

  if( flag3 == 1 )
  { timer->stop();
    //static QString filename;
    temp_str3 = QFileDialog::getOpenFileName(
                    0,
                    "Загрузить файл", temp_str3 ,
                    "*.bin"  );
    //if( !temp_str3.isEmpty() ) filename = temp_str3;
  timer->start();
    flag3 = 0;
  }

  if( flag4 == 1 )
  { timer->stop();

   temp_str3 = QFileDialog::getSaveFileName(
                    0,
                    tr("Записать файл"), temp_str3,
                    tr("*.bin")  );
  timer->start();
    flag4 = 0;
  }

  modbus -> subnode = le->text().toInt();

  //////////////////////

  QString console_data = Console::takeMessage();
  if( !console_data.isEmpty() )
  { te->insertPlainText( console_data );
  }

  if( mainwindow->cb_autoscroll->isChecked() )
  {
    QScrollBar *sb = te->verticalScrollBar();
    if( ! sb->isSliderDown() )  sb->setValue( sb->maximum() );
  }
}

//==============================================================================
const char InitDialog::ini_port_name[]  = "ComPortName";
const char InitDialog::ini_port_speed[] = "ComPortSpeed";
const char InitDialog::ini_port_host[]  = "ModbusTcpHost";
const char InitDialog::ini_port_timeout[]   = "ComPortTimeout";
const char InitDialog::ini_port_max_packet_size[] = "MaxPacketSize";
//==============================================================================
// Начальный диалог
//==============================================================================
InitDialog::InitDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle("MMpM - Настройка связи");

  Settings settings;

  QString str,str2;
  QStringList sl;
  int i;
  //----------------------------------------------------------------------
  cb_portname->clear();
  QStringList ports = SerialPort::queryComPorts();
  foreach( str, ports )
  { str2=str;
    str2.replace(';',"        ( ");
    cb_portname->addItem( str2+" )", str.section(';',0,0) );
  }
  cb_portname->addItem( "Modbus TCP", "===TCP===" );
  i = cb_portname->findData( settings.value( ini_port_name ).toString() );
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
  i = cb_portspeed->findText( settings.value( ini_port_speed ,"115200").toString() );
  if( i >= 0 ) cb_portspeed->setCurrentIndex(i);
  //----------------------------------------------------------------------
  le_tcp_server->setText( settings.value( ini_port_host,"localhost" ).toString() );
  //----------------------------------------------------------------------
  sb_timeout->setValue( settings.value( ini_port_timeout, 200 ).toInt() );
  //----------------------------------------------------------------------
  sb_max_packet_size->setValue( settings.value( ini_port_max_packet_size, 128 ).toInt() );
  //----------------------------------------------------------------------
  setFocus();
}

//==============================================================================
// Переключение ComboBox'а порта
//==============================================================================
void InitDialog::on_cb_portname_currentIndexChanged(int)
{
  QString str = cb_portname->itemData( cb_portname->currentIndex() ).toString();
  bool b = ( str == "===TCP===" );

  cb_portspeed->setHidden(   b );
  l_speed->setHidden(        b );
  l_server->setHidden(      !b );
  le_tcp_server->setHidden( !b );
}

//==============================================================================
// Начальный диалог -- нажатие OK
//==============================================================================
void InitDialog::accept()
{
  QString str = cb_portname->itemData(cb_portname->currentIndex()).toString();

  if( str == "===TCP===" )
  {
    modbus->setupTcpHost( le_tcp_server->text() );
  } else if( modbus->setupPort( str, cb_portspeed->currentText().toInt()) == 0 )
  { QMessageBox::critical(this, "MMpM",
                              tr("Ошибка открытия порта!"));
   reject();
   return;
  }
  modbus->setupTimeOut( sb_timeout->value() );
  thread1->setupMaxPacketSize( sb_max_packet_size->value() );

  Settings settings;
  settings.setValue( ini_port_name,   str                );
  settings.setValue( ini_port_speed,  cb_portspeed->currentText() );
  settings.setValue( ini_port_host,   le_tcp_server->text() );
  settings.setValue( ini_port_timeout,   sb_timeout->value() );
  settings.setValue( ini_port_max_packet_size,   sb_max_packet_size->value() );

  done(1);
}

//==============================================================================
// Начальный диалог -- нажатие OK
//==============================================================================
void InitDialog::on_pb_help_clicked()
{
  helpdialog->exec();
  helpdialog->activateWindow();
}

//==============================================================================
// Диалог настроек
//==============================================================================
OptDialog::OptDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  setWindowTitle( tr("Опции"));

  cb_modbus_packets->setChecked( Console::messageTypes() & Console::ModbusPacket );

  sb_delay->setValue( thread1->erase_delay );

  if( !secret_mode2 ) cb1->hide();
  if( !secret_mode2 ) cb2->hide();

  if( !mainwindow->pb8->isHidden() )  cb1->setChecked( true );
  if( !mainwindow->pb11->isHidden() ) cb2->setChecked( true );
  setFocus();
}

//==============================================================================
// Диалог настроек -- нажатие OK
//==============================================================================
void OptDialog::accept()
{
  Console::setMessageTypes( Console::ModbusPacket, cb_modbus_packets->isChecked() );

  thread1->erase_delay = sb_delay->value();

  if( cb1->isChecked() && secret_mode2 ) { mainwindow->pb8->show(); }
  else                                   { mainwindow->pb8->hide(); }
  if( cb2->isChecked() && secret_mode2 ) { mainwindow->pb11->show(); }
  else                                   { mainwindow->pb11->hide(); }
  done(1);
}

//==============================================================================
// Диалог помощи
//==============================================================================
HelpDialog::HelpDialog(QWidget*)
{
  setupUi( this );
  setWindowTitle("Помощь MMpM версия 2.6 от 20091020");
  resize(700,400);

  QFont f = font();
  f.setPointSize( 9 );
  textBrowser->setFont( f );
  textBrowser->setSource( QUrl::fromLocalFile(":/html/help/help.html" ) );
}

