#include <QtGui>
#include <QtXml>

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>

#include "dialogs.h"
#include "main.h"
#include "mbmasterxml.h"
#include "serialport.h"
#include "mbtcp.h"

//##############################################################################
/// Окно выбора порта и скорости
//##############################################################################
InitDialog::InitDialog( QWidget *parent)
  : QDialog( parent = 0 )
{
  setupUi( this );
  setWindowTitle("MKStudio");

  QSettings settings( QSETTINGS_PARAM );

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
  le_tcp_server -> setText( settings.value("tcpserver").toString() );
  //----------------------------------------------------------------------
  setFocus();
}

//==============================================================================
/// Переключение ComboBox'а порта
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
/// Окно выбора порта и скорости -- нажатие OK
//==============================================================================
void InitDialog::accept()
{
  QString str = cb_portname->itemData( cb_portname->currentIndex() ).toString();
  QString str2 = le_tcp_server->text().simplified();

  QSettings settings( QSETTINGS_PARAM );
  settings.setValue( "portname",  str );
  settings.setValue( "portspeed", cb_portspeed->currentText() );
  settings.setValue( "tcpserver", str2 );

  if( str == "===TCP===" )
  { str = le_tcp_server->text();
    port = new MbTcpPort;
  } else
  { port = new SerialPort;
  }

  if( !str.isEmpty() )
  {  port->setName( str );
     port->setSpeed( cb_portspeed->currentText().toInt() );
     port->setAnswerTimeout( sb_timeout->value() );
  }

  done( QDialog::Accepted );
}

//##############################################################################
/// Диалог "Привязка параметров"
//##############################################################################
AssignDialog::AssignDialog( QWidget *parent, const QString &assign_arg,
                           const QString &format_arg,  const QString &ss_arg,
                           const QStringList &sl_arg )
  : QDialog( parent ), assign( assign_arg ), format( format_arg ), ss( ss_arg )
{
  setupUi( this );
  setWindowTitle("Привязка");
  QRegExp rx("^(\\d+)/(\\d+)/(\\d+)$");
  rx.indexIn( assign );
  le1->setText( rx.cap(1) );
  le2->setText( rx.cap(2) );
  le3->setText( rx.cap(3) );  le1->setFocus();

  if( format.contains("unsigned") ) cb_format   -> setCurrentIndex(1);
  if( format.contains("hex") )      cb_format   -> setCurrentIndex(2);
  if( format.contains("bin") )      cb_format   -> setCurrentIndex(3);

  le_ss->setText(ss);
  QCompleter *completer = new QCompleter(sl_arg, this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  le_ss->setCompleter(completer);

  cb_fill_row    -> addItems( QStringList() << "модуль" << "слот" << "номер" );
  cb_fill_column -> addItems( QStringList() << "модуль" << "слот" << "номер" );
  cb_fill_row    -> setCurrentIndex(2);
  cb_fill_column -> setCurrentIndex(2);
  cb_fill_row    -> setEnabled( false );
  cb_fill_column -> setEnabled( false );

  connect( cbox_fill_row,    SIGNAL( toggled(bool) ),
           cb_fill_row,        SLOT( setEnabled(bool) ) );
  connect( cbox_fill_column, SIGNAL( toggled(bool) ),
           cb_fill_column,     SLOT( setEnabled(bool) ) );

}

//==============================================================================
/// Диалог "Привязка параметров" -- нажатие ОК
//==============================================================================
void AssignDialog::accept()
{
  if(    cbox_fill_row->isChecked()
      && cbox_fill_column->isChecked()
      && (cb_fill_row->currentIndex() == cb_fill_column->currentIndex()) )
  {
    QMessageBox::warning( this, app_header,
         "Заполнение ячеек задано неверно. Для строк и столбцов заданы одинаковые параметры." );
    return;
  }

  bool m_ok, s_ok, n_ok;
  int im = le1->text().toInt(&m_ok);
  int is = le2->text().toInt(&s_ok);
  int in = le3->text().toInt(&n_ok);

  if( !m_ok || ( im < 1 ) )
  {  QMessageBox::warning( this, app_header,
         "Поле \"Модуль\" должно содержать номер модуля (целое положительное число)." );
     return;
  }
  if( !s_ok || ( is < 1 ) )
  {  QMessageBox::warning( this, app_header,
         "Поле \"Слот\" должно содержать номер слота (целое положительное число)." );
     return;
  }
  if( !n_ok || ( in < 1 ) )
  {  QMessageBox::warning( this, app_header,
         "Поле \"Номер\" должно содержать целое положительное число." );
     return;
  }

  assign = QString().sprintf("%d/%d/%d",im,is,in);


  switch( cb_format->currentIndex() )
  { case(0): format = "dec";      break;
    case(1): format = "unsigned"; break;
    case(2): format = "hex";     break;
    case(3): format = "bin";     break;
  }

  ss = le_ss->text();

  done( QDialog::Accepted );
}

//##############################################################################
/// Диалог "Настройки"
//##############################################################################
SettingsDialog::SettingsDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( "Настройки MKStudio" );
}

//##############################################################################
/// Диалог "Редактирование текста" (используется при редактировании
/// содержимого заголовка таблицы)
//##############################################################################
TextEditDialog::TextEditDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( app_header );
}

//##############################################################################
/// Диалог "Настройки"
//##############################################################################
UnitedSlots::UnitedSlots( QWidget *parent )
  : QWidget( parent )
{
  int i;
  QTableWidgetItem *titem;
  QColor color;

  setupUi( this );
  setWindowTitle( app_header );

  setWindowFlags( Qt::Tool );
  setAttribute( Qt::WA_DeleteOnClose,    true );

  tw->verticalHeader()->setDefaultSectionSize( font().pointSize()+11 );
  tw->setColumnCount(2);
  tw->setHorizontalHeaderLabels( QStringList() << "Цвет" << "Слот" );
  tw->horizontalHeader()->resizeSection(  0 , 50 );
  tw->horizontalHeader()->setStretchLastSection( true );
  tw->horizontalHeader()->setClickable( false );
  tw->verticalHeader()->hide();
  tw->setRowCount(curves_num);

  plot->setCanvasBackground(QColor("linen"));
  plot->enableAxis(QwtPlot::yRight, false);
  plot->enableAxis(QwtPlot::yLeft,  true );

  for(i=0;i<curves_num;i++)
  { color = QColor( (Qt::GlobalColor)(i+8) );
    titem = new QTableWidgetItem;
    titem->setFlags(0);
    titem->setBackgroundColor( color );
    tw->setItem(i,0,titem);

    curves[i] = new QwtPlotCurve("");
    curves[i]->setPen(QPen( color ,2));
    curves[i]->setStyle( QwtPlotCurve::Steps );
    curves[i]->attach(plot);
  }

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  grid->setMajPen(QPen(Qt::black, 1, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 1,  Qt::DotLine));
  grid->attach(plot);

  startTimer( 1000 );
}

//==============================================================================
///
//==============================================================================
void UnitedSlots::timerEvent ( QTimerEvent * event )
{
  Q_UNUSED( event );

  int i,j,k;
  bool ok;
  QTableWidgetItem *titem;
  QVector<double> plot_data_x;
  QVector<double> plot_data_y;

  for(i=0;i<curves_num;i++)
  { titem = tw->item(i,1);
    if( titem == 0 ) continue;
    j = titem->text().toInt(&ok);
    if( !ok ) continue;
    if( j < 1 ) continue;

    MMSlot ss = mbmaster->getSlot( 1, j-1 );

    int n = ss.data.size()+1;
    if( n==1 ) continue;
    plot_data_x.resize( n );
    plot_data_y.resize( n );

    for( k=1; k<n; k++ )
    { plot_data_y[k] = ss.data[k-1].toDouble();
      plot_data_x[k] = k;
    }
    plot_data_x[0] = 0;
    plot_data_y[0] = plot_data_y[1];
    curves[i]->setData( plot_data_x.constData(), plot_data_y.constData(),n );
  }
  plot->replot();
}
