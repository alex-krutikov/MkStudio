#include <QtGui>
#include <QtXml>

#include "plot.h"
#include "mbmaster.h"

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

//##############################################################################
//
//##############################################################################
Plot::Plot( QWidget *parent, MBMaster *mbmaster, const QString &config )
  : QWidget( parent )
{
  int i;
  static QRegExp rx("^(\\d+)/(\\d+)/(\\d+)$");

  setupUi( this );
  setWindowTitle("—амописец");

  setWindowFlags( Qt::Tool );
  setAttribute( Qt::WA_DeleteOnClose,    true );
  move( QCursor::pos() - QPoint(20,20) );
  startTimer(100);

  //--------------------------------------------
  rx.indexIn( config );
  module_index = rx.cap(1).toInt();
  slot_index   = rx.cap(2).toInt();
  value_index  = rx.cap(3).toInt()-1;
  //--------------------------------------------
  this->mbmaster = mbmaster;
  firstrun_flag = true;
  pause_flag    = false;
  avr_a=0;
  avr_counter=0;
  points_counter = 0;
  memset( x_data, 0, sizeof( x_data ) );
  memset( y_data, 0, sizeof( y_data ) );
  for(i=0; i<y_hist_data_len;i++) x_hist_data[i]=i;
  //--------------------------------------------
  plot->setCanvasBackground(QColor("linen"));
  plot->enableAxis(QwtPlot::yRight);
  plot->enableAxis(QwtPlot::yLeft, false );
  plot->setAxisScale(QwtPlot::xBottom,20,0);

  QwtPlotCurve *plot_data = new QwtPlotCurve("Curve 1");
  plot_data->setPen(QPen(Qt::darkBlue,2));
  plot_data->setYAxis(QwtPlot::yRight);
  plot_data->setRawData( x_data, y_data, y_data_len );
  plot_data->attach(plot);
  //--------------------------------------------
  hist->setCanvasBackground(QColor("linen"));
  hist->enableAxis(QwtPlot::xTop,      false );
  hist->enableAxis(QwtPlot::xBottom,   false );
  hist->enableAxis(QwtPlot::yRight,    false );
  hist->enableAxis(QwtPlot::yLeft,     false );

  QwtPlotCurve *hist_data = new QwtPlotCurve("Curve 2");
  hist_data->setPen( Qt::NoPen );
  hist_data->setStyle( QwtPlotCurve::Steps );
  hist_data->setBrush( QColor( "lightslategrey" ) );
  hist_data->setRawData( x_hist_data, y_hist_data, y_hist_data_len );
  hist_data->attach(hist);
  //---------

  QWidget *w = new QWidget( plot );
  QHBoxLayout *layout = new QHBoxLayout;

  QToolButton *pb = new QToolButton;
  pb->setText("ќчистить");
  pb->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  connect( pb, SIGNAL( clicked() ), this, SLOT( pb_clear_clicked() ) );

  pb_pause = new QToolButton;
  pb_pause->setText("ѕауза");
  pb_pause->setToolTip("ѕауза");
  pb->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  pb_pause->setIcon( QIcon(":/icons/res/media_pause.png" ) );
  connect( pb_pause, SIGNAL( clicked() ), this, SLOT( pb_pause_clicked() ) );

  pb_export = new QToolButton;
  pb_export->setText("Ёкспорт");
  pb_export->setToolTip("Ёкспорт данных в буфер обмена");
  pb_export->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  connect( pb_export, SIGNAL( clicked() ), this, SLOT( pb_export_clicked() ) );

  layout->addWidget( pb );
  layout->addWidget( pb_export );
  layout->addWidget( pb_pause );
  layout->setSpacing(4);
  layout->setContentsMargins(0,0,0,0);
  w->setLayout( layout );
  w->move(5,5);
}

//==============================================================================
//
//==============================================================================
double Plot::noise() const
{
  return le_std->text().toDouble();
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
  if( pause_flag )
  { pause_flag = false;
    pb_pause->setText("ѕауза");
    pb_pause->setToolTip("ѕауза");
    pb_pause->setIcon( QIcon(":/icons/res/media_pause.png" ) );
  } else
  { pause_flag = true;
    pb_pause->setText("ѕуск");
    pb_pause->setToolTip("ѕуск");
    pb_pause->setIcon( QIcon(":/icons/res/media_play.png" ) );
  }
}

//==============================================================================
//
//==============================================================================
void Plot::pb_export_clicked()
{
  QString str;
  int i,j,k;
  double scale_x = 20;
  double x_value;

  switch( cb_speed->currentIndex() )
  { case(0): // 20 сек
      scale_x = 20;
      break;
    case(1): // 1 мин
      scale_x = 60;
      break;
    case(2): // 5 мин
      scale_x = 300;
      break;
    case(3): // 20 мин
      scale_x = 1200;
      break;
    case(4): // 1 час
      scale_x = 3600;
      break;
  }

  i = y_data_len - 1;
  j = y_data_len - points_counter;
  k = 0;
  while( i >= (y_data_len - points_counter) )
  { x_value = k*(scale_x/y_data_len);
    str += QString::number( x_value ) + "\t";
    str += QString::number( y_data[j] );
    str += "\n";
    i--;
    j++;
    k++;
  }

  QApplication::clipboard()->setText( str );
  QMessageBox::information( this, "—амописец", "ƒанные скопированы в буфер обмена." );
}

//==============================================================================
//
//==============================================================================
void Plot::on_cb_speed_currentIndexChanged( int index )
{
  double a = 20;
  switch(index)
  { case(0): a=20; break; // 20 сек
    case(1): a=60; break; // 1 мин
    case(2): a=5;  break; // 5 мин
    case(3): a=20; break; // 20 мин
    case(4): a=60; break; // 1 час
  }
  plot->setAxisScale(QwtPlot::xBottom,a,0);
  pb_clear_clicked();
}

//==============================================================================
// 100 ms таймер
//==============================================================================
void Plot::timerEvent ( QTimerEvent * event )
{
  Q_UNUSED( event );

  int i,j;
  double a;
  double scale_x;

  if( pause_flag && (!firstrun_flag) ) return;

  MMValue value = mbmaster->getSlotValue( module_index, slot_index, value_index );
  if ( value.status() != MMValue::Ok ) return;

  a = value.toDouble();

  scale_x=0;

  if(firstrun_flag)
  { avr_counter=1;
    avr_a=a;
  } else
  { avr_a += a;
    avr_counter++;

    switch( cb_speed->currentIndex() )
    { case(0): // 20 сек
        scale_x = 20;
        break;
      case(1): // 1 мин
        if( avr_counter < 3 ) return;
        scale_x = 60;
        break;
      case(2): // 5 мин
        if( avr_counter < 15 ) return;
        scale_x = 5;
        break;
      case(3): // 20 мин
        if( avr_counter < 60 ) return;
        scale_x = 20;
        break;
      case(4): // 1 час
        if( avr_counter < 180 ) return;
        scale_x = 60;
        break;
    }
  }
  a = avr_a / avr_counter;
  avr_a = 0;
  avr_counter=0;

  // сдвиг данных
  if( firstrun_flag )
  { for( i=0;i<y_data_len;i++ ) y_data[i]=a;
    firstrun_flag = false;
  } else
  { for( i=1;i<y_data_len;i++ ) y_data[i-1]=y_data[i];
    y_data[ y_data_len - 1 ] = a;
  }
  // максимум, минимум и шкала по X
  y_min=y_max=y_data[0];
  for( i=0;i<y_data_len;i++ )
  { if( y_data[i] > y_max ) y_max = y_data[i];
    if( y_data[i] < y_min ) y_min = y_data[i];
    x_data[i]=((double)(y_data_len-i)/y_data_len)*scale_x;
  }
  // обрезание еще не полученных данных
  if( points_counter < y_data_len ) points_counter++;
  for(i=0; i<(y_data_len-points_counter); i++ )
  { x_data[i]=x_data[y_data_len-points_counter];
  }
  // расчет среднего и дисперсии
  y_mean=0;y_std=0;
  for( i=y_data_len-points_counter;i<y_data_len;i++ )
  { y_mean += y_data[i];
    y_std  += y_data[i]*y_data[i];
  }
  y_mean /= points_counter;
  y_std  /= points_counter;
  y_std  -= y_mean*y_mean;
  y_std = sqrt( y_std );

  // расчет гистограммы
  a = (y_max-y_min)/(y_hist_data_len-1);
  memset( y_hist_data, 0 , sizeof( y_hist_data ) );
  if(a)
  { for( i=y_data_len-points_counter;i<y_data_len;i++ )
    { j = (int)((y_data[i] - y_min) / a)+1;
      if( j < 0 ) j=0;
      if( j > (y_hist_data_len-1) ) j=(y_hist_data_len-1);
      y_hist_data[j]+=1;
    }
  }

  // обновление графика
  plot->replot();
  hist->replot();
  le_mean          -> setText( QString::number( y_mean ) );
  le_diff          -> setText( QString::number( y_max-y_min ) );
  le_std           -> setText( QString::number( y_std ) );
  if( ( y_mean != 0 ) && (y_max != y_min ) )
  { le_noise_percent -> setText( QString::number( fabs((y_max-y_min) / y_mean )*100 ) );
    le_noise_db      -> setText( QString::number( -20*log10( fabs( (y_max-y_min) / y_mean ) ),'f' ,1 ) );
  } else
  { le_noise_percent->clear();
    le_noise_db->clear();
  }
}

//==============================================================================
//
//==============================================================================
QSize Plot::sizeHint() const
{
  return QSize(500,250);
}
