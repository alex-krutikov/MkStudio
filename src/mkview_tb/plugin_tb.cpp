#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "mbmasterxml.h"

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


#if defined( Q_OS_WIN32 )
  #define QSETTINGS_PARAM (qApp->applicationDirPath()+"/mkview_tb.ini"),QSettings::IniFormat
#elif defined( Q_OS_UNIX )
  #define QSETTINGS_PARAM "mkview_tb"
#else
  #error Wrong OS
#endif

MBMasterXML   *mbmaster;
SerialPort *port;

int modules_n;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Термошкаф";
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
   d.setWindowTitle(tr("Термошкаф"));
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
    QMessageBox::information(0,"Термошкаф",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
 }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, 1, 0, "MB100" );
  mbmaster->add_slot( 1, 1, 0x002E, 4, MBDataType::Floats  ); // значения входов
  mbmaster->add_slot( 1, 2, 0x0202, 1, MBDataType::Floats  ); // уставка ПИД2
  mbmaster->add_slot( 1, 3, 0x000A, 8, MBDataType::Bits    ); // Выходы
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
  int i;
  state=0;
  state_names << "Останов" << "" << "Нагрев" << "" << "Прогрев"
              << "" << "Охлаждение" << "" << "" <<"Процесс закончен";
  ust_set = 0;
  cycle_counter = 0;
  time_dev = 1;
  ftime=0;
  t=0;
  cc=0;

  setupUi( this );
  setWindowTitle("Термошкаф");
 //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  //rw.setSize( sizeHint() );
  rw.setSize( QSize( 900,700 ) );
  if( rw.height() > ( rs.height() - 70 ) )
  { rw.setHeight(  rs.height() - 50 );
  }
  if( rw.width() > ( rs.width() - 50 ) )
  { rw.setWidth(  rs.width() - 70 );
  }
  rw.moveCenter( rs.center() );
  setGeometry( rw );
  //--------------------------------------------
  QSettings settings( QSETTINGS_PARAM );

  le_par_warmspeed->setText( settings.value("warmspeed",20 ).toString() );
  le_par_tempmax->setText( settings.value("tempmax",80 ).toString() );
  le_par_progrevtime->setText( settings.value("progrevtime", 4 ).toString() );
  le_par_tempmin->setText( settings.value("tempmin", 40 ).toString() );
  le_par_cycles->setText( settings.value("cycles", 1 ).toString() );
  le_par_warmtimemax->setText( settings.value("warmtimemax", 6 ).toString() );
  le_par_cooltimemax->setText( settings.value("cooltimemax", 6 ).toString() );
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

  //--------------------------------------------
  memset( buff_ustavka, 0, sizeof(buff_ustavka) );
  memset( buff_t1, 0, sizeof(buff_t1) );
  memset( buff_t2, 0, sizeof(buff_t1) );
  for( i=0; i<buff_len; i++ ) buff_x[i] = ((double)i/buff_len)*(buff_len/3600.0);
  plot->setAxisScale(QwtPlot::xBottom,buff_x[buff_len-1],0 );
  buff_index=0;
  //--------------------------------------------

  plot->setCanvasBackground(QColor("linen"));
  plot->enableAxis(QwtPlot::yRight);
  plot->enableAxis(QwtPlot::yLeft, false );
  plot->setAxisScale(QwtPlot::yLeft,0,110);
  plot->setAxisScale(QwtPlot::yRight,0,110);
  plot->setAxisTitle(QwtPlot::xBottom, "Время (часов назад)");
  plot->setAxisTitle(QwtPlot::yRight, "Температура");
  plot->plotLayout()->setAlignCanvasToScales(true);

  plot_curve_ustavka = new QwtPlotCurve("Уставка");
  plot_curve_ustavka->setPen(QPen(Qt::darkBlue,2));
  plot_curve_ustavka->attach(plot);

  plot_curve_t1 = new QwtPlotCurve("Температура верх");
  plot_curve_t1->setPen(QPen(Qt::cyan,2));
  plot_curve_t1->setYAxis(QwtPlot::yRight);
  plot_curve_t1->attach(plot);

  plot_curve_t2 = new QwtPlotCurve("Температура низ");
  plot_curve_t2->setPen(QPen(Qt::red,2));
  plot_curve_t2->setYAxis(QwtPlot::yRight);
  plot_curve_t2->attach(plot);

  plot->insertLegend(new QwtLegend, QwtPlot::BottomLegend);

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  grid->setMajPen(QPen(Qt::black, 1, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 1, Qt::DotLine));
  grid->attach(plot);
  //--------------------------------------------

  pb_start -> setEnabled( true );
  pb_stop  -> setEnabled( false );
  pb_skip  -> setEnabled( false );
  //--------------------------------------------

  QToolButton *pb = new QToolButton( plot );
  pb->setText("Очистить");
  pb->move(5,5);
  connect( pb, SIGNAL( clicked() ), this, SLOT( pb_clear_clicked() ) );

  startTimer( 1000 );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  double f;
  int    i;
  double cycles      = le_par_cycles->text().toInt();

  //------------------------------------
  le_ind_state                  -> setText( state_names.value( state ) );
  if( ftime )
  { le_ind_state_time           -> setText( QTime().addSecs((int)ftime).toString("hh:mm:ss") );
    le_ind_state_remaining_time -> setText( QTime().addSecs((int)(ftime-t)).toString("hh:mm:ss") );
  } else
  { le_ind_state_time           -> clear();
    le_ind_state_remaining_time -> clear();
  }
  if( cycle_counter )
  { le_ind_cycle                -> setText( QString("%1 из %2").arg(cycle_counter).arg(cycles) );
  } else
  { le_ind_cycle->clear();
  }
  //------------------------------------

  MMValue value;
  value = mbmaster->getSlotValue( 1, 1, 0 );
  if( value.status() == MMValue::Ok )
  { le_ind_tcj->setText( QString::number( value.toDouble(), 'g', 4 ) );
  } else
  { le_ind_tcj->setText( "  ***" );
  }
  value = mbmaster->getSlotValue( 1, 1, 1 );
  if( value.status() == MMValue::Ok )
  { le_ind_t1->setText( QString::number( value.toDouble(), 'g', 4 ) );
  } else
  { le_ind_t1->setText( "  ***" );
  }
  value = mbmaster->getSlotValue( 1, 1, 2 );
  if( value.status() == MMValue::Ok )
  { le_ind_t2->setText( QString::number( value.toDouble(), 'g', 4 ) );
  } else
  { le_ind_t2->setText( "  ***" );
  }
  value = mbmaster->getSlotValue( 1, 2, 0 );
  if( value.status() == MMValue::Ok )
  { le_ind_ust->setText( QString::number( value.toDouble(), 'g', 4 ) );
  } else
  { le_ind_ust->setText( "  ***" );
  }
  value = mbmaster->getSlotValue( 1, 3, 2 );
  if( value.status() == MMValue::Ok )
  { static int flag;
    if( value.toInt() && (flag!=1) )
    { le_ind_nagrevatel->setText( "ВКЛ" );
      le_ind_nagrevatel->setStyleSheet("background: lightpink");
      flag=1;
    }
    if( (value.toInt()==0) && (flag!=2) )
    { le_ind_nagrevatel->setText( "ОТКЛ" );
      le_ind_nagrevatel->setStyleSheet("background: lightgrey");
      flag=2;
    }
  } else
  { le_ind_nagrevatel->setText( "  ***" );
  }
  mbmaster->setSlotValue( 1,2,0, ust_set );
  //------------------------------------

  t += 1.0;
  if( ( state == 0 ) || ( state == 9 ) )
  {  ust_set=0;
     ftime=0;
     t=0;
     cycle_counter=0;
  }

  switch( state )
  { case( 0 ): // останов
      ust_set=0;
      break;
    case( 1 ): // инициализация нагрева
      ust_set = le_ind_t2->text().toDouble();
      ftime =   ( le_par_tempmax->text().toDouble() - ust_set )
              / ( le_par_warmspeed->text().toDouble()/3600);
      t = 0;
      state++;
      break;
    case( 2 ): // нагрев
      f = le_par_warmspeed->text().toDouble();
      f = f / 3600;
      ust_set += f;
      if(      ( ust_set >= le_par_tempmax->text().toDouble() )
            || ( t > ( le_par_warmtimemax->text().toDouble()*3600 ) )  )
      { state++;
      }
      break;
    case( 3 ): // инициализация прогрева
      t = 0;
      ftime = le_par_progrevtime->text().toDouble()*3600;
      state++;
      break;
    case( 4 ): // прогрев
      ftime   = le_par_progrevtime->text().toDouble()*3600;
      ust_set = le_par_tempmax->text().toDouble();
      if( t > ftime )
      { state++;
      }
      break;
    case( 5 ): // инициализация остывания
      ust_set=0;
      t=0;
      ftime   = le_par_cooltimemax->text().toDouble()*3600;
      state++;
      break;
    case( 6 ): // остывание
      ust_set=0;
      ftime   = le_par_cooltimemax->text().toDouble()*3600;
      if(    ( t > ftime )
          || ( le_ind_t2->text().toDouble() < le_par_tempmin->text().toDouble() ) )
      { state++;
      }
      break;
    case( 7 ): // новый цикл
      cycle_counter++;
      if( cycle_counter > le_par_cycles->text().toInt() )
      { state++;
        break;
      }
      state=1;
      break;
    case( 8 ): // инициализация финиш
      ust_set=0;
      pb_start -> setEnabled( true );
      pb_stop  -> setEnabled( false );
      pb_skip  -> setEnabled( false );
      state++;
      break;
    case( 9 ): // финиш
      ust_set=0;
      break;
  }

  //------------------------------------
  cc++;
  if( cc >= time_dev )
  {
    cc=0;
    for( i=buff_len-2; i>=0; i-- )
    { buff_ustavka[i+1] = buff_ustavka[i];
      buff_t1[i+1] = buff_t1[i];
      buff_t2[i+1] = buff_t2[i];
    }
    buff_ustavka[0] = le_ind_ust->text().toDouble();
    buff_t1[0]      = le_ind_t1->text().toDouble();
    buff_t2[0]      = le_ind_t2->text().toDouble();
    buff_index++;
    if( buff_index >= buff_len )
    { buff_index = buff_len / 2;
      time_dev <<= 1;
      for(i=0; i< buff_index ; i++ )
      { buff_ustavka[i] = ( buff_ustavka[2*i] + buff_ustavka[2*i+1] )/2;
        buff_t1[i] = ( buff_t1[2*i] + buff_t1[2*i+1] )/2;
        buff_t2[i] = ( buff_t2[2*i] + buff_t2[2*i+1] )/2;
      }
      for(i=0; i< buff_len ; i++ )
      { buff_x[i] = buff_x[i]*2;
      }
      plot->setAxisScale(QwtPlot::xBottom,buff_x[buff_len-1],0 );
    }
    plot_curve_ustavka->setRawData( buff_x, buff_ustavka, buff_index );
    plot_curve_t1->setRawData( buff_x, buff_t1, buff_index );
    plot_curve_t2->setRawData( buff_x, buff_t2, buff_index );
    plot->replot();
  }

  //------------------------------------
  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster->full_time()) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter()) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter()) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter()) );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_start_clicked()
{
  state = 1;
  cycle_counter=1;
  pb_start -> setEnabled( false );
  pb_stop  -> setEnabled( true );
  pb_skip  -> setEnabled( true );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_stop_clicked()
{
  state = 0;
  cycle_counter=0;
  ust_set=0;
  pb_start -> setEnabled( true );
  pb_stop  -> setEnabled( false );
  pb_skip  -> setEnabled( false );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_skip_clicked()
{
  switch( state )
  { case( 2 ):
    case( 4 ):
    case( 6 ):
      state++;
      break;
  }
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::pb_clear_clicked()
{
  int i;
  memset( buff_ustavka, 0, sizeof(buff_ustavka) );
  memset( buff_t1, 0, sizeof(buff_t1) );
  memset( buff_t2, 0, sizeof(buff_t1) );
  for( i=0; i<buff_len; i++ ) buff_x[i] = ((double)i/buff_len)*(buff_len/3600.0);
  plot->setAxisScale(QwtPlot::xBottom,buff_x[buff_len-1],0 );
  buff_index=0;
  time_dev = 1;
  cc=0;
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::closeEvent( QCloseEvent *event )
{
  if( ( state == 0 ) || ( state == 9 ) )
  {
    QSettings settings( QSETTINGS_PARAM );
    settings.setValue( "warmspeed",   le_par_warmspeed->text() );
    settings.setValue( "tempmax",     le_par_tempmax->text() );
    settings.setValue( "progrevtime", le_par_progrevtime->text() );
    settings.setValue( "tempmin",     le_par_tempmin->text() );
    settings.setValue( "cycles",      le_par_cycles->text() );
    settings.setValue( "warmtimemax", le_par_warmtimemax->text() );
    settings.setValue( "cooltimemax", le_par_cooltimemax->text() );
    return;
  }
  QMessageBox::information( this, "Термошкаф","Выход при запущенном процессе невозможен." );
  event->ignore();
}

