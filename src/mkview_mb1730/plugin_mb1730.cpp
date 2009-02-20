#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "mbmasterxml.h"
#include "mktable.h"
#include "plot.h"
#include "crc.h"

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>

MBMasterXML   *mbmaster;
SerialPort *port;

int modules_n;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Сервисный режим MB1730";
}

void MKViewPlugin::helpWindow( QWidget *parent )
{
   QDialog d( parent );

   QTextBrowser *tb = new QTextBrowser;
   QVBoxLayout *layout = new QVBoxLayout;
   layout->addWidget(tb);
   layout->setContentsMargins(0,0,0,0);
   d.setLayout(layout);

   tb->setFrameShape( QFrame::NoFrame );
   tb->setHtml( "<p>Здесь будет краткое описание модуля.</p> " );
   d.setWindowTitle("Сервисный режим MB1730");
   d.resize(300,300);
   d.exec();
}

void MKViewPlugin::mainWindow( QWidget *parent, const QString &portname, int portspeed, int node, int subnode )
{
  Q_UNUSED( parent );
  Q_UNUSED( node );
  Q_UNUSED( subnode );

  mbmaster = new MBMasterXML;
  port     = new SerialPort;

  port->setName( portname );
  port->setSpeed( portspeed );
  if( !port->open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,"Сервисный режим MB1730",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
 }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, node, 0, "MB1730" );
  mbmaster->add_slot( 1, 1, 0x0652,  5, MBDataType::Dwords );  // Сервисный режим
  mbmaster->add_slot( 1, 2, 0x0032,  2, MBDataType::Floats );  // Значения входов (мВ)
  mbmaster->add_slot( 1, 3, 0x01CE,  2, MBDataType::Dwords );  // Коды АЦП
  mbmaster->add_slot( 1,11, 0x067A, 68, MBDataType::Bytes  );  // Окно для записи калибровки
  mbmaster->polling_start();

  MainWindow mainwindow;
  mainwindow.show();
  qApp->exec();

  delete mbmaster;
  delete port;
}

Q_EXPORT_PLUGIN2(EchoInterface, MKViewPlugin);

//###############################################################################################
//
//###############################################################################################
MainWindow::MainWindow( QWidget *parent  )
  : QMainWindow( parent )
{
  QFont f = font();
  f.setPixelSize( 16 );
  setFont(f);

  setupUi( this );
  setWindowTitle("Сервисный режим MB1730");
 //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  rw.setSize( sizeHint() );
  if( rw.height() > ( rs.height() - 70 ) )
  { rw.setHeight(  rs.height() - 50 );
  }
  if( rw.width() > ( rs.width() - 50 ) )
  { rw.setWidth(  rs.width() - 70 );
  }
  rw.moveCenter( rs.center() );
  setGeometry( rw );

  tabWidget->hide();
  pb_adc_write->setEnabled( false );

  //--------------------------------------------
  close_flag = false;
  //--------------------------------------------
  QButtonGroup *bg = new QButtonGroup(this);
  bg->addButton( pb_disp_all );
  bg->addButton( pb_disp_off );
  bg->addButton( pb_disp_55  );
  bg->addButton( pb_disp_AA  );
  pb_disp_all->setChecked(true);
  //--------------------------------------------
  QStringList sl;

  sl << "Напряжение (мВ)"
     << "Разброс (мВ)"
     << "Код АЦП";

  //--------------------------------------------

  statusbar = new QStatusBar();
  full_time       = new QLabel;
  status_requests = new QLabel;
  status_answers  = new QLabel;
  status_errors   = new QLabel;
  statusbar->addPermanentWidget( full_time       );
  statusbar->addPermanentWidget( status_requests );
  statusbar->addPermanentWidget( status_answers  );
  statusbar->addPermanentWidget( status_errors   );
  setStatusBar( statusbar );

  startTimer( 50 );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  static int timer;
  int i;
  bool ok,bb;
  unsigned char b;
  static int code_avr[60];
  double ff;

  MMSlot slot = mbmaster->getSlot( 1,1 );

  if( slot.data[0].isValid() && ( slot.data[0].toInt() == (int)0xAA55BB66 ) )
  { tabWidget->show();
  } else
  { tabWidget->hide();
    if( close_flag ) close();
  }

//------------------------------------
  int a=0;
  bb = cb_comm->isChecked();
  label_comm_post -> setEnabled( bb );
  cbox_comm       -> setEnabled( bb );
  if( bb ) a |= ( (cbox_comm->currentIndex()+1) << 8 );
  if( pb_do1->isChecked() ) a |= 1;
  if( pb_do2->isChecked() ) a |= 2;
  if( pb_do3->isChecked() ) a |= 4;
  if( pb_do4->isChecked() ) a |= 8;
  mbmaster->setSlotValue( 1,1,1,a );

//------------------------------------
  a=0;
  if( pb_disp_all -> isChecked() ) a=0;
  if( pb_disp_off -> isChecked() ) a=1;
  if( pb_disp_55  -> isChecked() ) a=2;
  if( pb_disp_AA  -> isChecked() ) a=3;
  mbmaster->setSlotValue( 1,1,2,a );
//------------------------------------
  union
  { int i;
    float f;
  } u;
  u.i = slot.data[4].toInt();
  le_i2c_temp->setText( QString::number( u.f ) );
//------------------------------------
  mbmaster->setSlotValue( 1,1,3,sb_ao->value() );
//------------------------------------
  MMSlot slot2 = mbmaster->getSlot( 1,2 );
  if( slot2.data[0].isValid() ) { le_ai1->setText( slot2.data[0].toString() ); }
  else                          { le_ai1->setText( "***" ); }
  if( slot2.data[1].isValid() ) { le_ai2->setText( slot2.data[1].toString() ); }
  else                          { le_ai2->setText( "***" ); }
//------------------------------------
  MMSlot slot3 = mbmaster->getSlot( 1,3 );
  if( slot3.data[0].isValid() ) { le_ai1_raw->setText( slot3.data[0].toString() ); }
  else                          { le_ai1_raw->setText( "***" ); }
  if( slot3.data[1].isValid() ) { le_ai2_raw->setText( slot3.data[1].toString() ); }
  else                          { le_ai2_raw->setText( "***" ); }
//------------------------------------
  le_ai1_1->setText( le_ai1->text() );
  le_ai2_1->setText( le_ai2->text() );
//------------------------------------
  switch( cbox_adc_calib_channel->currentIndex() )
  { case(0): // канал 1
      le_current_code->setText( slot3.data[0].toString() );
      break;
    case(1): // канал 2
      le_current_code->setText( slot3.data[1].toString() );
      break;
  }
  for(i=59;i>0;i--)  code_avr[i]=code_avr[i-1];
  code_avr[0]=le_current_code->text().toInt();
  ff=0;
  for(i=0;i<60;i++)  ff+=code_avr[i];
  ff /= 60;
  ff += 0.5;
  le_current_code_avr->setText( QString::number( (int)ff ) );
  //--------------------------------------------------------

  timer++;
  switch( write_automat_state )
  {
    case(1):
      mbmaster->setSlotValue( 1, 11, 64, 0 );
      mbmaster->setSlotValue( 1, 11, 65, 0 );
      mbmaster->setSlotValue( 1, 11, 66, 0 );
      mbmaster->setSlotValue( 1, 11, 67, 0 );
      for(i=0; i<64; i++ )
      { mbmaster->setSlotValue( 1, 11, i, eeprom.buff[i] );
      }
      timer=0;
      write_automat_state++;
    case(2):
      if( timer > 40 ) // 2 сек
      { write_automat_state=0;
        QMessageBox::critical( this, "Калибровка АЦП", "Ошибка записи в буфер модуля");
        pb_adc_write->setEnabled( true );
      }
      ok = true;
      for(i=0; i<64; i++ )
      { b = mbmaster->getSlotValue( 1, 11, i ).toInt();
        if( b != eeprom.buff[i] )
        { ok = false;
        } break;
      }
      if( ok ) write_automat_state++;
      break;
    case(3):
      mbmaster->setSlotValue( 1, 11, 64, 0xBB );
      mbmaster->setSlotValue( 1, 11, 65, 0x55 );
      mbmaster->setSlotValue( 1, 11, 66, 0xAA );
      mbmaster->setSlotValue( 1, 11, 67, 0x55 );
      timer=0;
      write_automat_state++;
      break;
    case(4):
      if( timer > 40 ) // 2 сек
      { write_automat_state=0;
        QMessageBox::critical( this, "Калибровка АЦП", "Ошибка записи в EEPROM");
        pb_adc_write->setEnabled( true );
      }
      if(  mbmaster->getSlotValue( 1, 11, 64 ).toInt() != 0x11 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 65 ).toInt() != 0x11 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 66 ).toInt() != 0x11 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 67 ).toInt() != 0x11 ) break;
      mbmaster->setSlotValue( 1, 11, 64, 0 );
      mbmaster->setSlotValue( 1, 11, 65, 0 );
      mbmaster->setSlotValue( 1, 11, 66, 0 );
      mbmaster->setSlotValue( 1, 11, 67, 0 );
      timer=0;
      write_automat_state++;
      break;
    case(5):
      if( timer > 40 ) // 2 сек
      { write_automat_state=0;
        QMessageBox::critical( this, "Калибровка АЦП", "Ошибка записи в EEPROM");
        pb_adc_write->setEnabled( true );
      }
      if(  mbmaster->getSlotValue( 1, 11, 64 ).toInt() != 0 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 65 ).toInt() != 0 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 66 ).toInt() != 0 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 67 ).toInt() != 0 ) break;
      write_automat_state=0;
      QMessageBox::information( this, "Калибровка АЦП", "Калибровка АЦП успешно прошита.");
      pb_adc_write->setEnabled( true );
      break;
  }


  //------------------------------------
  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster->full_time()) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter()) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter()) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter()) );
}

//==============================================================================
//
//==============================================================================
void MainWindow::calc()
{
  double a,b,p1_mv,p2_mv,p1_code,p2_code;
  bool ok1,ok2,ok3,ok4;
  p1_mv   = le_hp_value->text().toDouble( &ok1 );
  p2_mv   = le_lp_value->text().toDouble( &ok2 );
  p1_code = le_hp_code->text().toDouble(  &ok3 );
  p2_code = le_lp_code->text().toDouble(  &ok4 );

  if( !ok1 )
  { QMessageBox::critical( this, "Калибровка АЦП",
               "Значение напряжения калибровки в верхней точке не задано." );
    goto error;
  }
  if( !ok2 )
  { QMessageBox::critical( this, "Калибровка АЦП",
               "Значение напряжения калибровки в нижней точке не задано." );
    goto error;
  }

  if( !( ok1 && ok2 && ok3 && ok4 ) ) goto error;

  a = ( p1_mv - p2_mv ) / ( p1_code - p2_code );
  b = p1_mv - a*p1_code;

  le_A->setText( QString::number( a ) );
  le_B->setText( QString::number( b ) );
  pb_adc_write->setEnabled( true );
  return;
error:
  le_A->clear();
  le_B->clear();
  pb_adc_write->setEnabled( false );
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_pb_adc_write_clicked()
{
  pb_adc_write->setEnabled( false );

  memset( eeprom.buff, 0, sizeof( eeprom.buff ));

  eeprom.st.signature = 0xA5B5C5D5;
  eeprom.st.version   = 1;
  eeprom.st.a         = le_A->text().toDouble();
  eeprom.st.b         = le_B->text().toDouble();
  eeprom.st.crc16     = CRC::CRC16( (const char*)eeprom.buff, sizeof(eeprom.buff)-2);

  write_automat_state = 1;
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_ao_0_clicked()
{ sb_ao->setValue(0);
}
//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_ao_4095_clicked()
{ sb_ao->setValue(4095);
}
//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_ao_2048_clicked()
{ sb_ao->setValue(2048);
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_sm_enable_clicked()
{
  mbmaster->setSlotValue(1,1,0,(int)0xAA55BB66  );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_sm_disable_clicked()
{
  mbmaster->setSlotValue(1,1,0,0  );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_lp_trap_clicked()
{
  le_lp_code->setText( le_current_code_avr->text() );
  calc();
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_hp_trap_clicked()
{
  le_hp_code->setText( le_current_code_avr->text() );
  calc();
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_adc_clear_clicked()
{
  le_lp_code->clear();
  le_hp_code->clear();
  calc();
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_font_plus_clicked()
{
  QFont f = font();
  int i = f.pixelSize();
  if( i < 25 ) i++;
  f.setPixelSize( i );
  setFont(f);
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_font_minus_clicked()
{
  QFont f = font();
  int i = f.pixelSize();
  if( i > 10 ) i--;
  f.setPixelSize( i );
  setFont(f);
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_ai1_graph_clicked()
{
  Plot *plot = new Plot(this, mbmaster, "1/2/1" );
  plot->setWindowTitle("Аналоговый вход. Канал 1");
  plot->show();
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_ai2_graph_clicked()
{
  Plot *plot = new Plot(this, mbmaster, "1/2/2" );
  plot->setWindowTitle("Аналоговый вход. Канал 2");
  plot->show();
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::closeEvent( QCloseEvent *event )
{
  Q_UNUSED( event );

  if( !close_flag )
  { close_flag = true;
    mbmaster->setSlotValue(1,1,0,0  );
    event->ignore();
  }
}
