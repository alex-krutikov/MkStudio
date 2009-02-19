#include <QtGui>

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>

#include "plugin.h"

#include "mbmaster.h"
#include "serialport.h"
#include "crc.h"

MBMaster   *mbmaster;
SerialPort *port;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Калибровка mAI103 (AI020)";
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
   tb->setHtml( "<p>Калибровка мезонина АЦП mAI103 при помощи модуля AI020.</p> " );
   d.setWindowTitle(tr("Описание"));
   d.resize(300,300);
   d.exec();
}

void MKViewPlugin::mainWindow( QWidget *parent, const QString &portname, int portspeed, int node, int subnode )
{
  Q_UNUSED( parent );
  Q_UNUSED( node );
  Q_UNUSED( subnode );

  mbmaster = new MBMaster;
  port     = new SerialPort;

  port->setName( portname );
  port->setSpeed( portspeed );
  if( !port->open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,"Калибровка mAI103 при помощи AI020",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
  }
  mbmaster->setTransport( port );

/*
<MBConfig>
  <Module Node="1.1" Desc="" N="1" Name="AI020" >
    <Slot Desc="Настройки"                      Operation="readwrite" Length="4"   Type="bytes"   N="1"  Addr="2"   />
    <Slot Desc="Значения аналоговых входов"     Operation="readonly"  Length="17"  Type="floats"  N="3"  Addr="DA"  />
    <Slot Desc="Коды АЦП"                       Operation="readonly"  Length="17"  Type="dwords"  N="10" Addr="9BE" />
    <Slot Desc="Калибровка АЦП"                 Operation="readwrite" Length="68"  Type="bytes"   N="11" Addr="A0E" />
  </Module>
</MBConfig>
*/

  mbmaster->add_module( 1, 1, 1, "DI020" );
  mbmaster->add_slot( 1, 1,  0x0002, 4,  MBDataType::Bytes  ); // Настройки
  mbmaster->add_slot( 1, 3,  0x00DA, 17, MBDataType::Floats ); // Значения аналоговых входов
  mbmaster->add_slot( 1, 10, 0x09BE, 17, MBDataType::Dwords ); // Коды АЦП
  mbmaster->add_slot( 1, 11, 0x0A0E, 68, MBDataType::Bytes  ); // Калибровка АЦП
  mbmaster->polling_start();


  MainWindow mainwindow;
  mainwindow.show();
  qApp->exec();
}

Q_EXPORT_PLUGIN2(EchoInterface, MKViewPlugin);

//########################################################################################

//==============================================================================
// Главное окно
//==============================================================================
MainWindow::MainWindow()
{
  setupUi( this );

  app_header = "Калибровка mAI103 при помощи AI020";

  setWindowTitle( app_header );
  int i;

  //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  rw.setSize( QSize(750,550 ));
  if( rw.height() > ( rs.height() - 70 ) )
  { rw.setHeight(  rs.height() - 50 );
  }
  if( rw.width() > ( rs.width() - 50 ) )
  { rw.setWidth(  rs.width() - 70 );
  }
  rw.moveCenter( rs.center() );
  setGeometry( rw );
  //--------------------------------------------


  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(group_update()));
  timer->start(50);

  QStringList sl;
  sl << "Напряжение (мВ)"
     << "Разброс (мВ)"
     << "Код АЦП";

  tw_inputs->verticalHeader()->setDefaultSectionSize(font().pointSize()+16);
  tw_inputs->setSelectionMode( QAbstractItemView::NoSelection );
  tw_inputs->setColumnCount(3);
  tw_inputs->setRowCount(17);
  tw_inputs->setHorizontalHeaderLabels( sl );
  tw_inputs->horizontalHeader()->resizeSection(  0 , 100 );
  tw_inputs->horizontalHeader()->resizeSection(  1 , 120 );
  QTableWidgetItem titem;
  titem.setTextAlignment( Qt::AlignRight|Qt::AlignVCenter );
  for( i=0; i< tw_inputs->rowCount(); i++ )
  {
    tw_inputs->setItem( i,0, new QTableWidgetItem(titem) );
    tw_inputs->setItem( i,1, new QTableWidgetItem(titem) );
  }

  //---------------------------------------------------------------------------
  for( i=0; i<plot_length; i++ )
  { plot_x[i] = i;
    plot_y[i] = 0;
  }
  for( i=0; i<codes_length; i++ )
  { codes_array[i] = 0;
  }

  plot->setCanvasBackground(QColor("linen"));
  plot_data = new QwtPlotCurve("Data");
  plot_data->setPen(QPen(Qt::darkBlue,2));
  plot_data->setYAxis(QwtPlot::yRight);
  plot_data->setRawData( plot_x, plot_y, plot_length );
  plot_data->attach(plot);

  plot->enableAxis(QwtPlot::yRight);
  plot->enableAxis(QwtPlot::yLeft, false );
  plot->enableAxis(QwtPlot::xBottom, false );
  //---------------------------------------------------------------------------
  first_run = true;
  write_automat_state = 0;
}

//==============================================================================
//
//==============================================================================
void MainWindow::group_update()
{
  int i,a;
  int channel;
  bool ok;
  unsigned char b;
  static int timer;

  //le_mb_req->setText( QString::number( mbmaster.request_counter ) );
  //le_mb_ans->setText( QString::number( mbmaster.answer_counter  ) );
  //le_mb_err->setText( QString::number( mbmaster.error_counter   ) );

  MMSlot ss1 = mbmaster->getSlot( 1, 3  );
  MMSlot ss2 = mbmaster->getSlot( 1, 10 );

  for( i = 0; i < tw_inputs->rowCount(); i++ )
  {
    if( i >= ss1.data.count() ) break;
    if( i >= ss2.data.count() ) break;
    tw_inputs->item(i,0)->setText( QString::number( ss1.data[i].toDouble(), 'g', 8 ) );
    tw_inputs->item(i,1)->setText( ss2.data[i].toString() );
  }

  //-------------------------------------------------------
  //channel = le_adc_input_n->text().toInt();
  channel = 1;
  if( ( channel >= 1 ) && ( channel <= 16 ) )
  {  mbmaster->setSlotValue( 1, 1, 1, channel+200 );
  } else
  {  mbmaster->setSlotValue( 1, 1, 1, 0 );
  }
  //-------------------------------------------------------
  double dmin,dmax;
  dmin=dmax=plot_y[0];
  for( i=0; i<(plot_length-1); i++ )
  {  plot_y[i]=plot_y[i+1];
     if( plot_y[i] > dmax ) dmax = plot_y[i];
     if( plot_y[i] < dmin ) dmin = plot_y[i];
  }
  if(!(( channel >=1 ) && ( channel <=17 ) )) channel=1;
  plot_y[plot_length-1] = tw_inputs->item(0,channel-1)->text().toDouble();
  plot->replot();
  //-------------------------------------------------------
  for( i=0; i<(codes_length-1); i++ )
  {  codes_array[i]=codes_array[i+1];
  }
  codes_array[codes_length-1] = tw_inputs->item(channel-1,1)->text().toInt();
  //codes_array[codes_length-1] = 1000;
  a = 0;
  for( i=0; i<(codes_length); i++ )
  { a += codes_array[i];
  }
  le_current_code->setText( QString::number( (int)((double)a/codes_length+0.5) ) );
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
        QMessageBox::critical( this, app_header, "Ошибка записи в буфер модуля");
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
        QMessageBox::critical( this, app_header, "Ошибка записи в EEPROM");
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
        QMessageBox::critical( this, app_header, "Ошибка записи в EEPROM");
        pb_adc_write->setEnabled( true );
      }
      if(  mbmaster->getSlotValue( 1, 11, 64 ).toInt() != 0 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 65 ).toInt() != 0 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 66 ).toInt() != 0 ) break;
      if(  mbmaster->getSlotValue( 1, 11, 67 ).toInt() != 0 ) break;
      write_automat_state=0;
      QMessageBox::information( this, app_header, "Калибровка АЦП успешно прошита.");
      pb_adc_write->setEnabled( true );
      break;
  }
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_pb_adc_clear_clicked()
{
  le_A->clear();
  le_B->clear();
  le_p1_code->clear();
  le_p2_code->clear();
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

//==============================================================================
//
//==============================================================================
void MainWindow::on_pb_zahvat1_clicked()
{
  le_p1_code->setText( le_current_code->text() );
  if( !le_p2_code->text().isEmpty() )
  { calc();
  }
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_pb_zahvat2_clicked()
{
  le_p2_code->setText( le_current_code->text() );
  if( !le_p1_code->text().isEmpty() )
  { calc();
  }
}

//==============================================================================
//
//==============================================================================
void MainWindow::calc()
{
  double a,b,p1_mv,p2_mv,p1_code,p2_code;
  p1_mv   = le_p1_mv->text().toDouble();
  p2_mv   = le_p2_mv->text().toDouble();
  p1_code = le_p1_code->text().toDouble();
  p2_code = le_p2_code->text().toDouble();

  a = ( p1_mv - p2_mv ) / ( p1_code - p2_code );
  b = p1_mv - a*p1_code;

  le_A->setText( QString::number( a ) );
  le_B->setText( QString::number( b ) );
}
