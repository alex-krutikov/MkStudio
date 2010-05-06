#include <QtGui>
#include <QtXml>

#include "ui_plot.h"
#include "plot.h"
#include "mktable.h"
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
#include <qwt_plot_zoomer.h>
#include <qwt_math.h>

//##############################################################################
//
//##############################################################################

static QList<QString> openFilesList;

Plot::Plot( QString title, QList<QTableWidgetItem *> mkItemList,bool min_flag, QWidget *parent, MBMasterXML *mbmaster )
  : QWidget( parent ), ui( new Ui::Plot )
{ QT_TRY{
  Q_INIT_RESOURCE( mklib );
  int i;
  static QRegExp rx("^(\\d+)/(\\d+)/(\\d+)$");

  ui->setupUi( this );
  titleStr = title;
  setWindowTitle(titleStr);

  setWindowFlags( Qt::Window | Qt::WindowMinimizeButtonHint );
  setAttribute( Qt::WA_DeleteOnClose, true );
  move( QCursor::pos() - QPoint(20,20) );

  mktableItemList = mkItemList;

  QStringList config;
  foreach( QTableWidgetItem * twp, mktableItemList )
  { config.append( twp->data(MKTable::AssignRole).toString() );
  }

  plots_count = config.count();

  if( plots_count == 1 )
  { ui->te_plotlist ->setVisible( false );
    ui->cb_use_match->setEnabled( false );
  }

  ui->cb_plot_stat->addItem( "синий" );
  if( plots_count > 1 ) ui->cb_plot_stat->addItem( "зеленый" );
  if( plots_count > 2 ) ui->cb_plot_stat->addItem( "красный" );
  if( plots_count > 3 ) ui->cb_plot_stat->addItem( "черный"  );

  module_index = new int[ plots_count ];
  slot_index   = new int[ plots_count ];
  value_index  = new int[ plots_count ];
  //--------------------------------------------
  for( i=0; i < plots_count; i++ )
  {
  rx.indexIn( config[i] );
  module_index[i] = rx.cap(1).toInt();
  slot_index[i]   = rx.cap(2).toInt();
  value_index[i]  = rx.cap(3).toInt()-1;
  }
  //--------------------------------------------
  this->mbmaster  = mbmaster;
  firstrun_flag   = true;
  pause_flag      = false;
  file_write_flag = false;
  minimize_flag   = min_flag;
  a_value = new double[ plots_count ];
  avr_a   = new double[ plots_count ];
  avr_counter        = 0;
  points_counter     = 0;
  file_write_counter = 0;

  zoomer = new QwtPlotZoomer( ui->plot->canvas() );
  zoomer->setSelectionFlags( QwtPicker::DragSelection | QwtPicker::CornerToCorner );
  zoomer->setTrackerMode( QwtPicker::AlwaysOff );

  picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yRight,
                             QwtPicker::NoSelection,
                             QwtPlotPicker::NoRubberBand,
                             QwtPicker::AlwaysOn,
                             ui->plot->canvas());
  picker->setTrackerPen(QColor(Qt::darkMagenta));

  ui->le_input_signal_range->setValidator( new QDoubleValidator( this ) );
  ui->pb_file_write->setEnabled( false );
  eb_ref = 0.0;
  y_data_len = 200;

  x_data  = new double[ y_data_len ];
  y_data1 = new double[ y_data_len ];
  y_data_stat = y_data1;

  if( plots_count > 1 ) y_data2 = new double[y_data_len];
  if( plots_count > 2 ) y_data3 = new double[y_data_len];
  if( plots_count > 3 ) y_data4 = new double[y_data_len];

  for( i=0; i < y_data_len; i++ )
  { x_data[i]  = 0.0;
    y_data1[i] = 0.0;
    if( plots_count > 1 ) y_data2[i] = 0.0;
    if( plots_count > 2 ) y_data3[i] = 0.0;
    if( plots_count > 3 ) y_data4[i] = 0.0;
  }

  for( i=0; i < y_hist_data_len; i++ ) x_hist_data[i] = i;
  //--------------------------------------------
  ui->plot->setCanvasBackground(QColor("linen"));
  ui->plot->enableAxis(QwtPlot::yRight);
  ui->plot->enableAxis(QwtPlot::yLeft, false );
  ui->plot->setAxisScale(QwtPlot::xBottom,20,0);

  plot_data1 = new QwtPlotCurve("Curve 1");
  plot_data1->setPen(QPen(Qt::blue,2));
  plot_data1->setYAxis(QwtPlot::yRight);
  plot_data1->setRawData( x_data, y_data1, y_data_len );
  plot_data1->attach( ui->plot );

  if( plots_count > 1 )
  {
  plot_data2 = new QwtPlotCurve("Curve 2");
  plot_data2->setPen(QPen(Qt::green,2));
  plot_data2->setYAxis(QwtPlot::yRight);
  plot_data2->setRawData( x_data, y_data2, y_data_len );
  plot_data2->attach( ui->plot );
  }
  if( plots_count > 2 )
  {
  plot_data3 = new QwtPlotCurve("Curve 3");
  plot_data3->setPen(QPen(Qt::red,2));
  plot_data3->setYAxis(QwtPlot::yRight);
  plot_data3->setRawData( x_data, y_data3, y_data_len );
  plot_data3->attach( ui->plot );
  }
  if( plots_count > 3 )
  {
  plot_data4 = new QwtPlotCurve("Curve 4");
  plot_data4->setPen(QPen(Qt::black,2));
  plot_data4->setYAxis(QwtPlot::yRight);
  plot_data4->setRawData( x_data, y_data4, y_data_len );
  plot_data4->attach( ui->plot );
  }
  //--------------------------------------------
  ui->hist->setCanvasBackground(QColor("linen"));
  ui->hist->enableAxis(QwtPlot::xTop,    false );
  ui->hist->enableAxis(QwtPlot::xBottom, false );
  ui->hist->enableAxis(QwtPlot::yRight,  false );
  ui->hist->enableAxis(QwtPlot::yLeft,   false );

  QwtPlotCurve *hist_data = new QwtPlotCurve("Curve H");
  hist_data->setPen( Qt::NoPen );
  hist_data->setStyle( QwtPlotCurve::Steps );
  hist_data->setBrush( QColor( "lightslategrey" ) );
  hist_data->setRawData( x_hist_data, y_hist_data, y_hist_data_len );
  hist_data->attach( ui->hist );
  //---------

  QWidget *w = new QWidget( ui->plot );
  QHBoxLayout *layout = new QHBoxLayout;

  pb_clear = new QToolButton;
  pb_clear->setText("Очистить");
  pb_clear->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  connect( pb_clear, SIGNAL( clicked() ), this, SLOT( pb_clear_clicked() ) );

  pb_export = new QToolButton;
  pb_export->setText("Экспорт");
  pb_export->setToolTip("Экспорт данных в буфер обмена");
  pb_export->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  connect( pb_export, SIGNAL( clicked() ), this, SLOT( pb_export_clicked() ) );

  pb_pause = new QToolButton;
  pb_pause->setText("Пауза");
  pb_pause->setToolTip("Пауза");
  pb_pause->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  pb_pause->setIcon( QIcon(":/icons/res/media_pause.png" ) );
  connect( pb_pause, SIGNAL( clicked() ), this, SLOT( pb_pause_clicked() ) );

  pb_minimize = new QToolButton;
  pb_minimize->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  connect( pb_minimize, SIGNAL( clicked() ), this, SLOT( pb_minimize_clicked()) );
  updateMinimizeButtonState( minimize_flag );

  connect( ui->cb_plot_stat, SIGNAL(currentIndexChanged(int)) ,
           this,             SLOT(change_current_plot(int)));

  connect( ui->pb_set_file,   SIGNAL(clicked()), this, SLOT(set_file_for_write()   ));
  connect( ui->pb_file_write, SIGNAL(clicked()), this, SLOT(file_write_start_stop()));

  change_current_plot( 0 );

  layout->addWidget( pb_clear    );
  layout->addSpacing(30);
  layout->addWidget( pb_pause    );
  layout->addWidget( pb_minimize );
  layout->addSpacing(30);
  layout->addWidget( pb_export   );
  layout->setSpacing(5);
  layout->setContentsMargins(0,0,0,0);
  w->setLayout( layout );
  w->move(5,5);

  timerInterval = 100;
  if( load_recorder_params() )
  { params_change();
    timerInterval = ui->sb_tact->value();
  }

  if( timerId ) killTimer( timerId );
  timerId = startTimer(timerInterval);

  if( !timerId ) QMessageBox::critical(this, "самописец","Не удалось запустить самописец.");
}
 QT_CATCH(...){}
}

//==============================================================================
//
//==============================================================================
Plot::~Plot()
{
  delete ui;
}
//==============================================================================
//
//==============================================================================
void Plot::closeEvent( QCloseEvent *event )
{ QT_TRY{
  Q_UNUSED( event );

  if ( windowState() != Qt::WindowMinimized ) emit signalPlotClose();

  delete[] a_value;
  delete[] x_data;
  delete[] y_data1;
  delete plot_data1;
  if( plots_count > 1 )
  { delete[] y_data2;
    delete plot_data2;
  }
  if( plots_count > 2 )
  { delete[] y_data3;
    delete plot_data3;
  }
  if( plots_count > 3 )
  { delete[] y_data4;
    delete plot_data4;
  }
  delete[] module_index;
  delete[] slot_index;
  delete[] value_index;
  delete[] avr_a;

  points_counter = 0;
  pause_flag = false;

  file_write_flag = false;
  file_stream.flush();
  if( file.isOpen() ) file.close();
  if( !file.fileName().isEmpty() ) openFilesList.removeOne( file.fileName() );
  save_recorder_params();
 }
 QT_CATCH(...){}
}
//==============================================================================
//
//==============================================================================
void Plot::changeEvent( QEvent *event )
{
  if ( event->type() == QEvent::WindowStateChange )
  {
    if( windowState() == Qt::WindowMinimized )
    { emit signalPlotClose();
    }
    if( windowState() == Qt::WindowNoState )
    { emit signalPlotOpen();
      if( minimize_flag ) signalMinimize( true );
    }
  }
}
//==============================================================================
//
//==============================================================================
double Plot::noise() const
{
  return ui->le_std->text().toDouble();
}
//==============================================================================
//
//==============================================================================
void Plot::setPlotList( QString list )
{
  ui->te_plotlist->setText( list );
}

//==============================================================================
//
//==============================================================================
void Plot::pb_clear_clicked()
{
  firstrun_flag  = true;
  points_counter = 0;
}

//==============================================================================
//
//==============================================================================
void Plot::pb_pause_clicked()
{
  ui->cb_speed->setEnabled( pause_flag );
  ui->sb_tact->setEnabled(  pause_flag );
  if( pause_flag )
  { pause_flag = false;
    pb_pause->setText("Пауза");
    pb_pause->setToolTip("Пауза");
    pb_pause->setIcon( QIcon(":/icons/res/media_pause.png" ) );
  } else
  { pause_flag = true;
    pb_pause->setText("Пуск");
    pb_pause->setToolTip("Пуск");
    pb_pause->setIcon( QIcon(":/icons/res/media_play.png" ) );
  }
}
//==============================================================================
//
//==============================================================================
void Plot::pb_minimize_clicked()
{
  emit signalMinimize( !minimize_flag );
}
//==============================================================================
//
//==============================================================================
void Plot::updateMinimizeButtonState( bool state )
{
  minimize_flag = state;
  if( minimize_flag )
  { pb_minimize->setText("Развернуть");
    pb_minimize->setToolTip("Развернуть таблицу");
    pb_minimize->setIcon( QIcon(":/icons/res/window_max.png") );
  } else
  { pb_minimize->setText("Свернуть");
    pb_minimize->setToolTip("Свернуть таблицу");
    pb_minimize->setIcon( QIcon(":/icons/res/window_min.png") );
  }
}
//==============================================================================
//
//==============================================================================
void Plot::pb_export_clicked()
{ QT_TRY{
  QString str;
  int i,j,k;
  double scale_x = 20;
  double x_value;

  switch( ui->cb_speed->currentIndex() )
  { case(0): // 5 сек
      scale_x = 5;
      break;
    case(1): // 20 сек
      scale_x = 20;
      break;
    case(2): // 1 мин
      scale_x = 60;
      break;
    case(3): // 5 мин
      scale_x = 300;
      break;
    case(4): // 20 мин
      scale_x = 1200;
      break;
    case(5): // 1 час
      scale_x = 3600;
      break;
  }
  i = y_data_len - 1;
  j = y_data_len - points_counter;
  k = i-j;
  while( i >= (y_data_len - points_counter) )
  { x_value = k*(scale_x/y_data_len);
    str += QString::number(  x_value );
    str += "\t" + QString::number( y_data1[j],'f',8 );
    if( plots_count > 1 ) str += "\t" + QString::number( y_data2[j],'f',8 );
    if( plots_count > 2 ) str += "\t" + QString::number( y_data3[j],'f',8 );
    if( plots_count > 3 ) str += "\t" + QString::number( y_data4[j],'f',8 );
    str += "\n";
    i--;
    j++;
    k--;
  }
  QApplication::clipboard()->setText( str );
  QMessageBox::information( this, "Самописец", "Данные скопированы в буфер обмена." );
}
 QT_CATCH(...){}
}
//==============================================================================
//
//==============================================================================
void Plot::on_cb_speed_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  params_change();
}
//==============================================================================
//
//==============================================================================
void Plot::on_sb_tact_valueChanged( int value )
{ params_change();
  if( timerId ) killTimer( timerId );
  timerId = startTimer( value );
  timerInterval = value;
  if( !timerId ) QMessageBox::critical(this, "самописец","Не удалось запустить самописец.");
}
//==============================================================================
//
//==============================================================================
void Plot::on_sb_plot_pen_w_valueChanged( int width )
{
   plot_data1->setPen( QPen( Qt::blue, width ) );
   if( plots_count > 1 )
   { plot_data2->setPen( QPen( Qt::green, width ) );
   }
   if( plots_count > 2 )
   { plot_data3->setPen( QPen( Qt::red, width ) );
   }
   if( plots_count > 3 )
   { plot_data4->setPen( QPen( Qt::black, width ) );
   }
   ui->plot->replot();
}
//==============================================================================
//
//==============================================================================
void Plot::change_current_plot( int index )
{ QT_TRY{
  QString strColor;
  QPalette palette;

  switch( index )
  {
    case 0: strColor = "blue" ;break;
    case 1: strColor = "green";break;
    case 2: strColor = "red"  ;break;
    case 3: strColor = "black";break;
   default: strColor = "blue";
  }

  palette.setColor( QPalette::Text, QColor( strColor ) );
  ui->le_mean->          setPalette( palette );
  ui->le_std->           setPalette( palette );
  ui->le_noise_percent-> setPalette( palette );
  ui->le_noise_db->      setPalette( palette );
  ui->le_eff_bits->      setPalette( palette );
  ui->le_diff->          setPalette( palette );

  ui->le_mean          ->clear();
  ui->le_std           ->clear();
  ui->le_noise_percent ->clear();
  ui->le_noise_db      ->clear();
  ui->le_eff_bits      ->clear();
  ui->le_diff          ->clear();

  switch(ui->cb_plot_stat->currentIndex())
      { case 0: y_data_stat = y_data1;break;
        case 1: y_data_stat = y_data2;break;
        case 2: y_data_stat = y_data3;break;
        case 3: y_data_stat = y_data4;break;
       default: y_data_stat = y_data1;break;
      }
  if( ( plots_count != 0 ) && ( pause_flag == true ) )
  { calc_statistic();
  }
 }
 QT_CATCH(...){}
}
//==============================================================================
//
//==============================================================================
void Plot::on_le_input_signal_range_textChanged( QString signal )
{
  bool ok;
  eb_ref = signal.toDouble( &ok );
  if( !ok ) eb_ref = 0.0;
  if( ( plots_count != 0 ) && ( pause_flag == true ) )
  { calc_statistic();
  }
}

//==============================================================================
//
//==============================================================================
void Plot::params_change()
{ QT_TRY{
  int sc_x = 20000;
  switch( ui->cb_speed->currentIndex() )
  { case(0): // 5 сек
      sc_x = 5000;
      break;
    case(1): // 20 сек
      sc_x = 20000;
      break;
    case(2): // 1 мин
      sc_x = 60000;
      break;
    case(3): // 5 мин
      sc_x = 300000;
      break;
    case(4): // 20 мин
      sc_x = 1200000;
      break;
    case(5): // 1 час
      sc_x = 3600000;
      break;
  }

    int tact = ui->sb_tact->value();
    y_data_len = (int)( sc_x / tact );

   double a;
   switch( ui->cb_speed->currentIndex() )
    { case(0): a=5;  break; // 5 сек
      case(1): a=20; break; // 20 сек
      case(2): a=60; break; // 1 мин
      case(3): a=5;  break; // 5 мин
      case(4): a=20; break; // 20 мин
      case(5): a=60; break; // 1 час
      default: a=20;
    }
  ui->plot->setAxisScale(QwtPlot::xBottom,a,0);

  delete[] x_data;
  delete[] y_data1;
  if( plots_count > 1 ) delete[] y_data2;
  if( plots_count > 2 ) delete[] y_data3;
  if( plots_count > 3 ) delete[] y_data4;

  x_data  = new double[ y_data_len ];
  y_data1 = new double[ y_data_len ];

  plot_data1 ->setRawData( x_data, y_data1,  y_data_len );

  if( plots_count > 1 )
  { y_data2 = new double[ y_data_len ];
    plot_data2->setRawData( x_data, y_data2, y_data_len );
  }
  if( plots_count > 2 )
  { y_data3 = new double[ y_data_len ];
    plot_data3->setRawData( x_data, y_data3, y_data_len );
  }
  if( plots_count > 3 )
  { y_data4 = new double[ y_data_len ];
    plot_data4->setRawData( x_data, y_data4, y_data_len );
  }
  switch(ui->cb_plot_stat->currentIndex())
  { case 0: y_data_stat = y_data1;break;
    case 1: y_data_stat = y_data2;break;
    case 2: y_data_stat = y_data3;break;
    case 3: y_data_stat = y_data4;break;
   default: y_data_stat = y_data1;break;
  }
  pb_clear_clicked();
}
 QT_CATCH(...){}
}
//==============================================================================
//
//==============================================================================
void Plot::calc_statistic()
{ QT_TRY{
  double scale_x = 20.0;
  int i,j;
  switch( ui->cb_speed->currentIndex() )
  { case(0): scale_x = 5.0 ;break;
    case(1): scale_x = 20.0;break;
    case(2): scale_x = 60.0;break;
    case(3): scale_x = 5.0 ;break;
    case(4): scale_x = 20.0;break;
    case(5): scale_x = 60.0;break;
   }
  // максимум, минимум, шкала по X
  y_min=y_max=y_data_stat[0];
  for( i=0; i < y_data_len; i++ )
  { if( y_data_stat[i] > y_max ) y_max = y_data_stat[i];
    if( y_data_stat[i] < y_min ) y_min = y_data_stat[i];
    x_data[i]=((double)(y_data_len-i)/y_data_len)*scale_x;
  }
  for(i=0; i<(y_data_len-points_counter); i++ )
  { x_data[i]=x_data[y_data_len-points_counter];
  }
  // расчет среднего и дисперсии
  y_mean=0;y_std=0;
  double y_offset;

  if( y_min < 0. ) y_offset = fabs( y_min );
  else y_offset = 0.;

  y_max += y_offset;
  y_min += y_offset;

  for( i = (y_data_len-points_counter); i < y_data_len; i++ )
  { y_mean += y_data_stat[i]+y_offset;
    y_std  += (y_data_stat[i]+y_offset)*(y_data_stat[i]+y_offset);
  }
  y_mean /= points_counter;
  y_std  /= points_counter;
  y_std  -= y_mean*y_mean;
  y_std = sqrt( y_std );

  ui->le_mean -> setText( QString::number( y_mean ) );
  ui->le_diff -> setText( QString::number( y_max-y_min ) );
  ui->le_std  -> setText( QString::number( y_std ) );

  if( ( y_mean != 0 ) && (y_max != y_min ) )
  { if( eb_ref != 0.0 )
    { ui->le_noise_percent -> setText( QString::number( fabs((y_max-y_min) / eb_ref )*100 ) );
      ui->le_noise_db -> setText( QString::number( (6.020599913*(log(eb_ref/(y_max-y_min))/log(2.0))+1.76),'f',1));
      ui->le_eff_bits -> setText( QString::number( log(eb_ref/(y_max-y_min))/log(2.0),'f' ,1 ) );
    }else
    { ui->le_noise_percent ->setText( "---" );
      ui->le_noise_db      ->setText( "---" );
      ui->le_eff_bits      ->setText( "---" );
    }
  }else
  { ui->le_noise_percent->clear();
    ui->le_noise_db->clear();
    ui->le_eff_bits->clear();
  }

  y_max -= y_offset;
  y_min -= y_offset;
  // расчет гистограммы
  double a_hist = (y_max-y_min)/(y_hist_data_len-1);
  memset( y_hist_data, 0 , sizeof( y_hist_data ) );
  if( a_value[ui->cb_plot_stat->currentIndex()] )
  { for( i=y_data_len-points_counter;i<y_data_len;i++ )
    { j = (int)((y_data_stat[i] - y_min) / a_hist)+1;
      if( j < 0 ) j=0;
      if( j > (y_hist_data_len-1) ) j=(y_hist_data_len-1);
      y_hist_data[j]+=1;
    }
  }
  ui->hist->replot();
}
 QT_CATCH(...){}
}

//==============================================================================
//
//==============================================================================
void Plot::timerEvent( QTimerEvent *event )
{ QT_TRY{
  Q_UNUSED( event );
  if( pause_flag ) return;

  int i;
  MMValue value;
  for( i=0; i < plots_count; i++ )
  { value = mbmaster->getSlotValue( module_index[i], slot_index[i], value_index[i] );
    if ( value.status() != MMValue::Ok ) return;
    a_value[i] = value.toDouble();
  }

  int zoomScale = 20;
  switch( ui->cb_speed->currentIndex() )
  { case 0: zoomScale = 5;  break; // 5 сек
    case 1: zoomScale = 20; break; // 20 сек
    case 2: zoomScale = 60; break; // 1 мин
    case 3: zoomScale = 5;  break; // 5 мин
    case 4: zoomScale = 20; break; // 20 мин
    case 5: zoomScale = 60; break; // 1 час
  }
  zoomer->setZoomBase( QwtDoubleRect( 0, 0, zoomScale, 1000 ) );

  time_append +=  timerInterval;
  if( file_write_flag )
  { file_stream << time_append;
    for( i=0; i<plots_count; i++ )
    { file_stream << "\t" << QString::number( a_value[i], 'f', 8 );
    }
    file_stream << "\r\n";
    file_write_counter++;
    if( file_write_counter >  100 )
      { file_stream.flush();
        file_write_counter = 0;
      }
  }
  if( firstrun_flag )
  { avr_counter=1;
    for( i=0; i < plots_count; i++ ) avr_a[i]  = a_value[i];
  } else
  { for( i=0; i < plots_count; i++ ) avr_a[i] += a_value[i];
    avr_counter++;
  }

  for( i=0; i < plots_count; i++ )
  { a_value[i] = avr_a[i] / avr_counter;
    avr_a[i]   = 0.0;
  }
   avr_counter = 0;
   // сдвиг данных
   if( firstrun_flag )
   { for( i=0; i < y_data_len; i++ )
     { y_data1[i] = a_value[0];
       if( plots_count > 1 ) y_data2[i] = a_value[1];
       if( plots_count > 2 ) y_data3[i] = a_value[2];
       if( plots_count > 3 ) y_data4[i] = a_value[3];
     }
     firstrun_flag = false;
   } else
   { for( i=1; i < y_data_len; i++ )
     { y_data1[i-1] = y_data1[i];
       if( plots_count > 1 ) y_data2[i-1] = y_data2[i];
       if( plots_count > 2 ) y_data3[i-1] = y_data3[i];
       if( plots_count > 3 ) y_data4[i-1] = y_data4[i];
     }
     y_data1[ y_data_len - 1 ] = a_value[0];
     if( plots_count > 1 ) y_data2[ y_data_len - 1 ] = a_value[1];
     if( plots_count > 2 ) y_data3[ y_data_len - 1 ] = a_value[2];
     if( plots_count > 3 ) y_data4[ y_data_len - 1 ] = a_value[3];
   }

   // обрезание еще не полученных данных
   if( points_counter < y_data_len ) points_counter++;

   // рассчет статистики
   calc_statistic();

   // масштабирование
   double koeff = 1.0;
   double y_mn, y_mx;
   int plot_index;
   plot_index = ui->cb_plot_stat->currentIndex();
   if( plots_count > 1 && ui->cb_use_match->isChecked() )
   {
     y_min1 = y_max1 = y_data1[0];
     y_min2 = y_max2 = y_data2[0];

     for( i=y_data_len-points_counter;i<y_data_len;i++ )
     { if( y_min1 > y_data1[i] ) y_min1 = y_data1[i];
       if( y_min2 > y_data2[i] ) y_min2 = y_data2[i];
       if( y_max1 < y_data1[i] ) y_max1 = y_data1[i];
       if( y_max2 < y_data2[i] ) y_max2 = y_data2[i];
     }

     if( plots_count > 2 )
     { y_min3 = y_max3 = y_data3[0];

       for( i=y_data_len-points_counter;i<y_data_len;i++ )
       { if( y_min3 > y_data3[i] ) y_min3 = y_data3[i];
         if( y_max3 < y_data3[i] ) y_max3 = y_data3[i];
       }
     }
     if( plots_count > 3 )
     { y_min4 = y_max4 = y_data4[0];

       for( i=y_data_len-points_counter;i<y_data_len;i++ )
       { if( y_min4 > y_data4[i] ) y_min4 = y_data4[i];
         if( y_max4 < y_data4[i] ) y_max4 = y_data4[i];
       }
     }

       switch( plot_index )
       { case 0:
         { y_mx = y_max1;
           y_mn = y_min1;
           break;
         }
         case 1:
         { y_mx = y_max2;
           y_mn = y_min2;
           break;
         }
         case 2:
         { y_mx = y_max3;
           y_mn = y_min3;
           break;
         }
         case 3:
         { y_mx = y_max4;
           y_mn = y_min4;
           break;
         }
         default:
         { y_mx = 1.0;
           y_mn = 0.0;
         }
       }
     koeff = y_mx - y_mn;
     }

     if( ui->cb_use_match->isChecked() && ( koeff!=0 ) )
     {
     if( plot_index!=0 )
     { if(y_min1!=y_max1)
       { for(i=0;i<y_data_len;i++)
         { y_data1[i] = (koeff*(y_data1[i]-y_min1)/(y_max1-y_min1))+y_mn;
         }
       } else
       { for(i=0;i<y_data_len;i++)
         { y_data1[i] = y_mn+((y_mx-y_mn)/2);
         }
       }

     }
     if( plot_index!=1 )
     { if(y_min2!=y_max2)
       { for(i=0;i<y_data_len;i++)
         { y_data2[i] = koeff*(y_data2[i]-y_min2)/(y_max2-y_min2)+y_mn;
         }
       } else
       { for(i=0;i<y_data_len;i++)
         { y_data2[i] = y_mn+((y_mx-y_mn)/2);
         }
       }
     }
     if( (plots_count>2) && (plot_index!=2) )
     { if( y_min3!=y_max3 )
       { for(i=0;i<y_data_len;i++)
         { y_data3[i] = (koeff*(y_data3[i]-y_min3)/(y_max3-y_min3))+y_mn;
         }
       } else
       { for(i=0;i<y_data_len;i++)
         { y_data3[i] = y_mn+((y_mx-y_mn)/2);
         }
       }
     }
     if( (plots_count>3) && (plot_index!=3) )
     { if( y_min4!=y_max4 )
       { for(i=0;i<y_data_len;i++)
         { y_data4[i] = (koeff*(y_data4[i]-y_min4)/(y_max4-y_min4))+y_mn;
         }
       }else
       { for(i=0;i<y_data_len;i++)
         { y_data4[i] = y_mn+((y_mx-y_mn)/2);
         }
       }
     }
   }
  // обновление графика
   ui->plot->replot();

   // возврат в исходное состояние из масштабированного графика
   if( (plots_count>1) && (koeff!=0) && ui->cb_use_match->isChecked() )
   { if( plot_index != 0 )
     { if( y_min1!=y_max1 )
       { for( i=0; i<y_data_len; i++ )
         { y_data1[i] = ((y_data1[i]-y_mn)*(y_max1-y_min1)/koeff)+y_min1;
         }
       } else
       { for( i=0; i<y_data_len; i++ )
         { y_data1[i] = y_min1;
         }
       }
     }
     if( plot_index != 1 )
     { if( y_min2!=y_max2 )
       { for( i=0; i<y_data_len; i++ )
         { y_data2[i] = ((y_data2[i]-y_mn)*(y_max2-y_min2)/koeff)+y_min2;
         }
       } else
       { for( i=0; i<y_data_len; i++ )
         { y_data2[i] = y_min2;
         }
       }
     }

     if( (plots_count>2) && (plot_index!=2) )
     { if( y_min3!=y_max3 )
       { for( i=0; i<y_data_len; i++ )
         { y_data3[i] = ((y_data3[i]-y_mn)*(y_max3-y_min3)/koeff)+y_min3;
         }
       } else
       { for( i=0; i<y_data_len; i++ )
         { y_data3[i] = y_min3;
         }
       }
     }
     if( (plots_count>3) && (plot_index!=3) )
     { if( y_min4!=y_max4 )
       { for( i=0; i<y_data_len; i++ )
         { y_data4[i] = ((y_data4[i]-y_mn)*(y_max4-y_min4)/koeff)+y_min4;
         }
       } else
       { for( i=0; i<y_data_len; i++ )
         { y_data4[i] = y_min4;
         }
       }
     }
   }
}
 QT_CATCH(...){}
}
   //==============================================================================
//
//==============================================================================
bool Plot::load_recorder_params()
{ QT_TRY{
  QStringList allParams, separatedParams;
  QString currentParams;

  foreach( QTableWidgetItem * twi, mktableItemList )
  { currentParams = twi->data(MKTable::RecorderParamsRole).toString();
    if( !currentParams.isEmpty() ) allParams.append( currentParams );
  }
  if( allParams.count() != plots_count ) return false;
  allParams.removeDuplicates();
  if((plots_count > 1) && (allParams.count() > 1)) return false;

  currentParams = allParams.first();
  separatedParams = currentParams.split( ";", QString::SkipEmptyParts );

  if( separatedParams.count() != 6 ) return false;

  bool ok;
  int integ;
  double dobl;

  //настройка параметров
  currentParams = separatedParams.at(0);
  integ = currentParams.toInt( &ok );
  if( ok ) ui->sb_tact->setValue( integ );

  currentParams = separatedParams.at(1);
  integ = currentParams.toInt( &ok );
  if( ok ) ui->cb_speed->setCurrentIndex( integ );

  currentParams = separatedParams.at(2);
  integ = currentParams.toInt( &ok );
  if( ok ) ui->sb_plot_pen_w->setValue( integ );

  currentParams = separatedParams.at(3);
  integ = currentParams.toInt( &ok );
  if( integ >= plots_count ) integ = plots_count - 1;
  if( ok ) ui->cb_plot_stat->setCurrentIndex( integ );

  currentParams = separatedParams.at(4);
  dobl = currentParams.toDouble( &ok );
  if( ok ) ui->le_input_signal_range->setText( QString::number( dobl,'f', 8 ) );

  if((separatedParams.at(5)=="1") && (plots_count>1)) ui->cb_use_match->setChecked( true );

  return true;
}
 QT_CATCH(...){ return false; }
}
//==============================================================================
//
//==============================================================================
bool Plot::save_recorder_params()
{ QT_TRY{
  QString params;
  //формирование строки параметров
  params += QString::number( ui->sb_tact->value() ) + ";";
  params += QString::number( ui->cb_speed->currentIndex() ) + ";";
  params += QString::number( ui->sb_plot_pen_w->value() ) + ";";
  params += QString::number( ui->cb_plot_stat->currentIndex() ) + ";";
  params += ui->le_input_signal_range->text() + ";";

  if( ui->cb_use_match->isChecked() ) params += "1";
  else params += "0";
  //сохранение параметров
  foreach( QTableWidgetItem * twi, mktableItemList )
  { twi->setData( MKTable::RecorderParamsRole, params );
  }
  return true;
}
 QT_CATCH(...){}
}
//==============================================================================
//
//==============================================================================
void Plot::set_file_for_write()
{ QString str = QFileDialog::getSaveFileName( this, "Файл", "", "*.*", 0,
                                              QFileDialog::DontConfirmOverwrite );
  if( str.isEmpty() ) return;

  if( openFilesList.contains( str ) )
  { QMessageBox::warning( this, "Ошибка","Файл занят другим самописцем. Выберите другой файл." );
    return;
  } else
  { openFilesList.append( str );
  }

  if( !ui->pb_file_write->isEnabled() ) ui->pb_file_write->setEnabled( true );

  if( !file.fileName().isEmpty() ) openFilesList.removeOne( file.fileName() );

  if( file.isOpen() ) file.close();

  file.setFileName( str );
  setWindowTitle( titleStr + " Запись: " + str );
}
//==============================================================================
//
//==============================================================================
void Plot::file_write_start_stop()
{ if( file.isOpen() )
  { file_stream.flush();
    file.close();
  }
  if( file_write_flag )
  { file_write_flag = false;
    ui->pb_file_write->setText( "Пуск" );
    ui->pb_set_file->setEnabled( true );
  } else
  {
    QMessageBox modeBox( this );
    modeBox.setWindowTitle( "Открытие файла" );
    modeBox.setText( "Выберите режим открытия файла." );
    modeBox.setIcon( QMessageBox::Question );
    QPushButton *yesB = modeBox.addButton( "Дозапись",  QMessageBox::YesRole );
    modeBox.addButton( "Перезапись", QMessageBox::NoRole );
    QPushButton *rejB = modeBox.addButton( "Отмена", QMessageBox::RejectRole );
    modeBox.setDefaultButton( yesB );
    modeBox.setEscapeButton( rejB );

    bool open_mode = true;
    switch( modeBox.exec() )
    { case 0: open_mode = true; break;
      case 1: open_mode = false;break;
      case 2: return;
     default: return;
    }

    if( !file.open( QIODevice::WriteOnly | ( open_mode ? QIODevice::Append : QIODevice::Truncate ) ) )
    { QMessageBox::critical( this, "Ошибка",
       "Не удалось открыть файл для записи. Возможно, он не указан или у вас нет на это прав.");
      return;
    }
    file.setPermissions( QFile::WriteOwner );
    file_stream.setDevice( &file );
    if( file_stream.status() != QTextStream::Ok )
    { QMessageBox::critical( this, "Ошибка", "Не удалось открыть файл для записи.");
      return;
    }
    ui->pb_set_file->setEnabled( false );
    ui->pb_file_write->setText( "Стоп" );
    time_append = 0;
    file_write_flag = true;
  }
}

//==============================================================================
//
//==============================================================================
QSize Plot::sizeHint() const
{
  return QSize(500,250);
}
