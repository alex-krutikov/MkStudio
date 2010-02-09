#include <QtGui>
#include <QDomDocument>

#include "slotwidget.h"
#include "mbmasterxml.h"
#include "console.h"
#include "ui_slotwidget.h"
#include "ui_slotdialog.h"

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>

//###################################################################
///���� � �������������� ����� (������������ �� �������� ������� �� ����)
//###################################################################
SlotWidget::SlotWidget( QWidget *parent, MBMasterXML *mm, int module, int slot )
  : QMainWindow( parent ), ui( new Ui::SlotWidget )
{
  ui->setupUi(this);
  ui->stackedWidget->setCurrentIndex(0);
  setWindowFlags( Qt::Tool );
  setAttribute( Qt::WA_DeleteOnClose, true );
  resize( 800,400 );

  module_n = module;
  slot_n   = slot;
  this->mm = mm;
  kx=ky=1;
  ax=0;

  const MMSlot slot2 = mm->getSlot( module_n, slot_n );
  // ���� "�������"
  QAction *act = new QAction( this );
  act->setText("�������");
  ui->menubar->addAction( act );
  connect( act, SIGNAL( triggered() ), SLOT( export_data() ) );

  // �������������� �������� �� ������
  QRegExp rx("^(.+)=(.+)$");
  foreach( QString s, slot2.attributes.split(';') )
  { if( rx.indexIn( s ) == 0 )
    { settings[ rx.cap(1) ] = rx.cap(2);
    }
  }
  //������ �����
  pb_pause = new QToolButton( ui->plot );
  pb_pause->move(5,5);
  pb_pause->setText("�����");
  pb_pause->setToolTip("�����");
  pb_pause->setCursor( QCursor( Qt::ArrowCursor ) );
  pb_pause->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  pb_pause->setIcon( QIcon(":/icons/res/media_pause.png" ) );
  connect( pb_pause, SIGNAL( clicked() ), this, SLOT( pb_pause_clicked() ) );

  pause_flag = false;

  // ��������� ����
  connect( ui->action_view_table, SIGNAL( activated() ), this, SLOT( view_changed() ) );
  connect( ui->action_view_plot1, SIGNAL( activated() ), this, SLOT( view_changed() ) );
  connect( ui->action_view_plot2, SIGNAL( activated() ), this, SLOT( view_changed() ) );

  connect(ui->action_stat_show, SIGNAL( toggled(bool) ), ui->statGroupBox, SLOT( setVisible(bool) ) );
  connect( ui->chb_line_trans_use, SIGNAL( toggled(bool)), this, SLOT ( slot_line_trans_used(bool)) );

  connect( ui->action_format_bin,      SIGNAL( activated() ), this, SLOT( format_changed() ) );
  connect( ui->action_format_dec,      SIGNAL( activated() ), this, SLOT( format_changed() ) );
  connect( ui->action_format_unsigned, SIGNAL( activated() ), this, SLOT( format_changed() ) );
  connect( ui->action_format_hex,      SIGNAL( activated() ), this, SLOT( format_changed() ) );
  connect( ui->action_format_float,    SIGNAL( activated() ), this, SLOT( format_changed() ) );

  // �������� ����
  QString str;
  str =  "C���: "+slot2.desc;
  str += " [ " + QString::number(slot2.module.n) + "." +QString::number(slot2.n);
  str += " | " + slot2.datatype.toName() + " | " + QString().sprintf("0x%X",slot2.addr);
  str += " | " + QString::number(slot2.len);
  str += " | " + slot2.module.name + " | ";
  str += "���� " + QString::number(slot2.module.node);
  if( slot2.module.subnode ) str += "."+QString::number(slot2.module.subnode);
	str += " ]" + slot2.module.desc ;
  setWindowTitle( str );

  // ��������� �������� ����
  switch( slot2.datatype.id() )
  { case( MBDataType::Floats ):
      ui->action_format_bin      -> setEnabled( false );
      ui->action_format_dec      -> setEnabled( false );
      ui->action_format_unsigned -> setEnabled( false );
      ui->action_format_hex      -> setEnabled( false );
      ui->action_format_float    -> setEnabled( true  );
      ui->action_format_float    -> setChecked( true );
      break;
    case( MBDataType::Bits ):
      ui->action_format_bin      -> setEnabled( false  );
      ui->action_format_dec      -> setEnabled( true   );
      ui->action_format_unsigned -> setEnabled( false  );
      ui->action_format_hex      -> setEnabled( false  );
      ui->action_format_float    -> setEnabled( false  );
      ui->action_format_dec      -> setChecked( true );
      break;
    default:
      ui->action_format_float    -> setEnabled( false  );
      ui->action_format_dec      -> setChecked( true );
      break;
  }

  // ������������ �������
  configure_table();

  // ������������ ��������
  ui->plot->setCanvasBackground(QColor("linen"));
  ui->plot->enableAxis(QwtPlot::yRight, false);
  ui->plot->enableAxis(QwtPlot::yLeft,  true );

  plot_curve = new QwtPlotCurve("");
  plot_curve->setPen(QPen(Qt::darkBlue,2));
  plot_curve->setStyle( QwtPlotCurve::Steps );
  plot_curve->attach(ui->plot);

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  grid->setMajPen(QPen(Qt::black, 1, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 1,  Qt::DotLine));
  grid->attach(ui->plot);

  ui->plot2->setCanvasBackground(QColor("linen"));
  ui->plot2->enableAxis(QwtPlot::yRight, false);
  ui->plot2->enableAxis(QwtPlot::yLeft,  true );

  plot2_curve = new QwtPlotCurve("");
  plot2_curve->setPen(QPen(Qt::darkBlue,2));
  plot2_curve->setStyle( QwtPlotCurve::Steps );
  plot2_curve->attach(ui->plot2);

  QwtPlotGrid *grid2 = new QwtPlotGrid;
  grid2->enableXMin(true);
  grid2->enableYMin(true);
  grid2->setMajPen(QPen(Qt::black, 1, Qt::DotLine));
  grid2->setMinPen(QPen(Qt::gray, 1,  Qt::DotLine));
  grid2->attach(ui->plot2);
  //��������� ����������
  for(int i=0; i<hist_plot_data_y_len;i++) hist_plot_data_x[i]=i;

  ui->hist->setCanvasBackground(QColor("linen"));
  ui->hist->enableAxis(QwtPlot::xTop,      false );
  ui->hist->enableAxis(QwtPlot::xBottom,   false );
  ui->hist->enableAxis(QwtPlot::yRight,    false );
  ui->hist->enableAxis(QwtPlot::yLeft,     false );

  QwtPlotCurve *hist_data = new QwtPlotCurve("Curve 2");
  hist_data->setPen( Qt::NoPen );
  hist_data->setStyle( QwtPlotCurve::Steps );
  hist_data->setBrush( QColor( "lightslategrey" ) );
  hist_data->setRawData( hist_plot_data_x, hist_plot_data_y, hist_plot_data_y_len );
  hist_data->attach(ui->hist);
  // ���������� ��������
  QMap<QString,QString> settings2 = settings;

  str = settings2["view"];
  if( str == "table"       ) ui->action_view_table->activate(QAction::Trigger);
  if( str == "plot"        ) ui->action_view_plot1->activate(QAction::Trigger);
  if( str == "scaled_plot" ) ui->action_view_plot2->activate(QAction::Trigger);
  str = settings2["format"];
  if( str == "dec"       ) ui->action_format_dec      -> activate(QAction::Trigger);
  if( str == "bin"       ) ui->action_format_bin      -> activate(QAction::Trigger);
  if( str == "unsigned"  ) ui->action_format_unsigned -> activate(QAction::Trigger);
  if( str == "hex"       ) ui->action_format_hex      -> activate(QAction::Trigger);
  if( str == "float"     ) ui->action_format_float    -> activate(QAction::Trigger);
  applay_settings();

  // ������ ����������� ��������
  startTimer( 1000 );
}

//===================================================================
//
//===================================================================
SlotWidget::~SlotWidget()
{
  delete ui;
}

//===================================================================
/// ��������� ���� "���"
//===================================================================
void SlotWidget::view_changed()
{
  QAction *act = (QAction*)sender();
  ui->action_view_table->setChecked( act == ui->action_view_table );
  ui->action_view_plot1->setChecked( act == ui->action_view_plot1 );
  ui->action_view_plot2->setChecked( act == ui->action_view_plot2 );

  if(        act == ui->action_view_table )
  { ui->stackedWidget->setCurrentIndex(0);
  } else if( act == ui->action_view_plot1 )
  { ui->stackedWidget->setCurrentIndex(1);
      pb_pause->setParent( ui->plot->canvas() );
      pb_pause->show();
  } else if( act == ui->action_view_plot2 )
  { ui->stackedWidget->setCurrentIndex(2);
      pb_pause->setParent( ui->plot2->canvas() );
      pb_pause->show();
  }
  save_settings();
}

//==============================================================================
/// ��������� ���� "������"
//==============================================================================
void SlotWidget::format_changed()
{
  QAction *act = (QAction*)sender();
  ui->action_format_bin      -> setChecked( act == ui->action_format_bin      );
  ui->action_format_dec      -> setChecked( act == ui->action_format_dec      );
  ui->action_format_unsigned -> setChecked( act == ui->action_format_unsigned );
  ui->action_format_hex      -> setChecked( act == ui->action_format_hex      );
  ui->action_format_float    -> setChecked( act == ui->action_format_float    );

  configure_table();
  save_settings();
}

//==============================================================================
/// ������� ������ � ����� ������
//==============================================================================
void SlotWidget::export_data()
{
  QString str;
  int i,j,n,m;
  QwtPlotCurve *curve = 0;

  switch( ui->stackedWidget->currentIndex() )
  { case(0): // ������� �������
      n = ui->table->rowCount();
      m = ui->table->columnCount();
      for(i=0; i<n; i++ )
      { for( j=0; j<m; j++ )
        { QTableWidgetItem *titem = ui->table->item(i,j);
          if( !titem ) continue;
          if( !( titem->flags() &  Qt::ItemIsEditable ) ) continue;
          str += titem->text() + "\n";
        }
      }
      str.chop(1);
      break;
   case(1): // ������
     curve = plot_curve;
     break;
   case(2): // ���������������� ������
     curve = plot2_curve;
     break;
  }

  if( curve )
  { // ������� ���������� ��������
    const QwtData &d = curve->data();
    n = d.size();
    for(i=1; i<n; i++ )
    { str += QString::number( d.x(i-1),'g',14 ) + "\t";
      str += QString::number( d.y(i  ),'g',14 ) + "\n";
    }
    str.chop(1);
  }

  QApplication::clipboard()->setText( str );
  QMessageBox::information( this, "����", "������ ����������� � ����� ������." );
}
//==============================================================================
//
//==============================================================================
void SlotWidget::pb_pause_clicked()
{
  if( pause_flag )
  { pause_flag = false;
    pb_pause->setText("�����");
    pb_pause->setToolTip("�����");
    pb_pause->setIcon( QIcon(":/icons/res/media_pause.png" ) );
  } else
  { pause_flag = true;
    pb_pause->setText("����");
    pb_pause->setToolTip("����");
    pb_pause->setIcon( QIcon(":/icons/res/media_play.png" ) );
  }
}
//==============================================================================
/// ���������� � ������ (��������) ������� �������� ����
//==============================================================================
void SlotWidget::save_settings()
{
  // ������� ������ ����������� �������
  if(      ui->action_format_dec->isChecked() )       settings["format"] = "dec";
  else if( ui->action_format_unsigned->isChecked() )  settings["format"] = "unsigned";
  else if( ui->action_format_hex->isChecked() )       settings["format"] = "hex";
  else if( ui->action_format_bin->isChecked() )       settings["format"] = "bin";
  else if( ui->action_format_float->isChecked() )     settings["format"] = "float";

  // ������� ���
  switch( ui->stackedWidget->currentIndex() )
  { case(0): settings["view"] = "table";         break;
    case(1): settings["view"] = "plot";          break;
    case(2): settings["view"] = "scaled_plot";   break;

  }

  // �������� ���� �������� � ������
  QString str;
  foreach( QString key, settings.uniqueKeys() )
  { str += key + "=" + settings[key].remove(';') + ";";
  }
  if( str.endsWith(";") ) str.chop(1);

  // ����� �������
  emit attributes_saved( module_n, slot_n, str );
}

//==============================================================================
/// ����� ������� ��������� ��������������� �������
//==============================================================================
void SlotWidget::on_action_set_scale_triggered()
{
  SlotDialog dialog(0,&settings );
  if( dialog.exec() != QDialog::Accepted ) return;

  applay_settings();
  ui->action_view_plot2->activate(QAction::Trigger);
  save_settings();
}

//==============================================================================
/// ���������� �������� ��������������� � ������� � �������
//==============================================================================
void SlotWidget::applay_settings()
{
  bool   ok;
  double a,b,c,d;
  int ia,ib,ic;
  QString str;
  QRegExp rx1("^(.+)/(.+)$");
  QRegExp rx2("^(.+)/(.+)/(.+)$");
  QRegExp rx3("^(.+)/(.+)/(.+)/(.+)$");

  // ��������� �������
  configure_table();

  //-------- ��������� ����������������� ������� ---------

  scale_mode_x = scale_mode_y = None;
  kx=ky=1;
  ax=0;
  plot2_unsigned = false;

  // �������� ����
  ui->plot2->setAxisTitle(QwtPlot::xBottom, settings["xname"]);
  ui->plot2->setAxisTitle(QwtPlot::yLeft,   settings["yname"]);

  // ������� - �����������
  if( settings.contains("xscale_coeff") )
  { a = settings["xscale_coeff"].toDouble( &ok );
    if( ok )
    { scale_mode_x = Coeff;
      kx = a;
    }
  }
  if( settings.contains("yscale_coeff") )
  { a = settings["yscale_coeff"].toDouble( &ok );
    if( ok )
    { scale_mode_y = Coeff;
      ky = a;
    }
  }

  // ������� - ��������
  str = settings["xscale_interval"];
  if( rx1.indexIn( str ) == 0 )
  { a = rx1.cap(1).toDouble( &ok );
    if( !ok )  goto lab1;
    b = rx1.cap(2).toDouble( &ok );
    if( !ok )  goto lab1;
    scale_mode_x = Interval;
    ax = a;
    bx = b;
  }
lab1:;
  // ������� - ���� ������
  str = settings["xscale_base"];
  if( rx2.indexIn( str ) == 0 )
  { ia = rx2.cap(1).toInt( &ok );
    if( !ok )  goto lab2;
    ib = rx2.cap(2).toInt( &ok );
    if( !ok )  goto lab2;
    ic = rx2.cap(3).toInt( &ok );
    if( !ok )  goto lab2;
    scale_mode_x = Base;
    scale_base_x.m = ia;
    scale_base_x.s = ib;
    scale_base_x.n = ic;
  }
lab2:;
  str = settings["yscale_base"];
  if( rx2.indexIn( str ) == 0 )
  { ia = rx2.cap(1).toInt( &ok );
    if( !ok )  goto lab3;
    ib = rx2.cap(2).toInt( &ok );
    if( !ok )  goto lab3;
    ic = rx2.cap(3).toInt( &ok );
    if( !ok )  goto lab3;
    scale_mode_y = Base;
    scale_base_y.m = ia;
    scale_base_y.s = ib;
    scale_base_y.n = ic;
  }
lab3:;
  plot2_unsigned = ( settings["yunsigned"] == "1" );

 str = settings["eff_bits_ref"];
  if( rx1.indexIn( str ) == 0 )
  { ia = rx1.cap(1).toInt( &ok );
    if( !ok )  ia=0;
    ib = rx1.cap(2).toInt( &ok );
    if( !ok )  ib=0;
    eff_bits_ref.column = ia;
    eff_bits_ref.row    = ib;
  }
  else
  {
    eff_bits_ref.column = 1;
    eff_bits_ref.row    = 1;
  }
  use_line_trans = ( settings["use_line_trans"] == "1" );

  ui->chb_line_trans_use->setChecked( use_line_trans );

  str = settings["line_trans_coords"];
  if( rx3.indexIn( str ) == 0 )
  { a = rx3.cap(1).toDouble( &ok );
    if( !ok )  a=0.;
    b = rx3.cap(2).toDouble( &ok );
    if( !ok )  b=0.;
    c = rx3.cap(3).toDouble( &ok );
    if( !ok )  c=0.;
    d = rx3.cap(4).toDouble( &ok );
    if( !ok )  d=0.;
    line_trans_x1 = a;
    line_trans_y1 = b;
    line_trans_x2 = c;
    line_trans_y2 = d;
  }
}
//==============================================================================
/// ��������� �������
//==============================================================================
void SlotWidget::configure_table()
{
  int i,j,k;
  char *cf;

  if(!mm) return;

  const MMSlot slot = mm->getSlot( module_n, slot_n );

  // ������ �����
  cf = (char*)"";
  if( ui->action_format_dec->isChecked() )         cf = (char*)"";
  if( ui->action_format_unsigned->isChecked() )    cf = (char*)"unsigned";
  if( ui->action_format_hex->isChecked() )         cf = (char*)"hex";
  if( ui->action_format_bin->isChecked() )         cf = (char*)"bin";
  if( ui->action_format_float->isChecked() )       cf = (char*)"float";

  // ���������� XML ��������� � �������
  QDomDocument doc;
  QDomElement tag_table = doc.createElement("Table");
  tag_table.setAttribute("ColumnCount", 10 );
  tag_table.setAttribute("RowCount",    ((slot.len-1)/10)+1 );
  tag_table.setAttribute("Labels",      "0;1;2;3;4;5;6;7;8;9" );
  tag_table.setAttribute("LabelsSize",  "70;70;70;70;70;70;70;70;70;70" );
  doc.appendChild(tag_table);
  i=0;
  while(1)
  { for(j=0; j<10; j++)
    { k = 10*i+j;
      if( k >= slot.len ) goto exit;
      QDomElement tag_item = doc.createElement("Item");
      tag_item.setAttribute("Row",    QString::number(i) );
      tag_item.setAttribute("Column", QString::number(j) );
      tag_item.setAttribute("Assign", QString("%1/%2/%3").arg(slot.module.n).arg(slot.n).arg(k+1) );
      tag_item.setAttribute("Format", cf );
      tag_table.appendChild(tag_item);
    }
    i++;
  }
exit:
  // ��������� �������
  ui->table->setMode( MKTable::Edit );
  ui->table->setMBMaster( mm );
  ui->table->loadConfiguration( doc );
  for(i=0; i<ui->table->rowCount(); i++ )
  { ui->table->setVerticalHeaderItem( i, new QTableWidgetItem( QString::number(10*i) ) );
  }
  ui->table->verticalHeader()->show();
  ui->table->setMode( MKTable::Polling );
}

//==============================================================================
/// ������ ��� ���������� ��������
//==============================================================================
void SlotWidget::timerEvent( QTimerEvent *event)
{
  Q_UNUSED( event );

  if( !mm ) return;
  if( pause_flag ) return;

  //----------- ���������� ������ --------------------

  int i,mask;
  bool b_unsigned =    ui->action_format_unsigned->isChecked()
                    || ui->action_format_hex->isChecked()
                    || ui->action_format_bin->isChecked();


  MMSlot ss = mm->getSlot( module_n, slot_n );

  switch( ss.datatype.id() )
  { case( MBDataType::Bytes ): mask = 0xFF;       break;
    case( MBDataType::Words ): mask = 0xFFFF;     break;
    default:                   mask = 0xFFFFFFFF; break;
  }

  if( b_unsigned && ( ss.datatype.id() == MBDataType::Floats ) ) b_unsigned=false;

  int n = ss.data.size()+1;
  plot_data_x.resize( n );
  plot_data_y.resize( n );
  plot_data_y_trans.resize( n );
  for( i=1; i<n; i++ )
  { plot_data_y[i] =   b_unsigned
                     ? (double)( (unsigned int)ss.data[i-1].toInt() & mask )
                     :  ss.data[i-1].toDouble();
    plot_data_x[i] = i;
  }

  //------------  ����� ������� ---------------

  plot_data_x[0] = 0;
  plot_data_y[0] = plot_data_y[1];
  if( use_line_trans )
  {
    //------------  �������� �������������� ������� ---------------
    for(i=1;i<n; i++)
    {
      plot_data_y_trans[i]=line_trans_y1+((line_trans_y2-line_trans_y1)/
                   (line_trans_x2-line_trans_x1))*(plot_data_y[i]-line_trans_x1);
    }

    plot_data_y_trans[0] = plot_data_y_trans[1];
    plot_curve->setData( plot_data_x.constData(), plot_data_y_trans.constData(),n );
  }
  else
  {
    plot_curve->setData( plot_data_x.constData(), plot_data_y.constData(),n );
  }
  ui->plot->replot();
  if( ui->action_view_plot1->isChecked() ) calc_statistic();

  //------------ ����� ����������������� ������� -------------

  for( i=1; i<n; i++ )
  { plot_data_y[i] =   plot2_unsigned
                     ? (double)( (unsigned int)ss.data[i-1].toInt() & mask )
                     :  ss.data[i-1].toDouble();
    plot_data_x[i] = i;
  }

  // ���������� ��� X
  switch( scale_mode_x )
  { case( None ):
      kx = 1.0;
      break;
    case( Coeff ):
      break;
    case( Interval ):
      kx = (bx-ax)/(n-1);
      break;
    case( Base ):
     MMValue v = mm->getSlotValue( scale_base_x.m, scale_base_x.s, scale_base_x.n-1 );
     if( v.isValid() )
     { kx = v.toDouble();
       if( kx == 0 ) kx=1;
       ax = 0;
     }
     break;
  }
  // ���������� ��� Y
  switch( scale_mode_y )
  { case( None ):
      ky = 1.0;
      break;
    case( Coeff ):
      break;
    case( Interval ):
      break;
    case( Base ):
     MMValue v = mm->getSlotValue( scale_base_y.m, scale_base_y.s, scale_base_y.n-1 );
     if( v.isValid() )
     { ky = v.toDouble();
       if( ky == 0 ) ky=1;
     }
     break;
  }

  // ��������������� � �����
  for( i=0; i<n; i++ )
  { plot_data_x[i] *= kx;
    plot_data_x[i] += ax;
    plot_data_y[i] *= ky;
  }

  //------------  �������� �������������� ����������������� �������-------------

  if( use_line_trans )
  {
    for(i=1;i<n; i++)
    {
      plot_data_y_trans[i]=line_trans_y1+((line_trans_y2-line_trans_y1)/
                   (line_trans_x2-line_trans_x1))*(plot_data_y[i]-line_trans_x1);
    }

    plot2_curve->setData( plot_data_x.constData(), plot_data_y_trans.constData(),n );
  }
  else
  {
    plot2_curve->setData( plot_data_x.constData(), plot_data_y.constData(),n );
  }
  ui->plot2->replot();
  if( ui->action_view_plot2->isChecked() ) calc_statistic();
}
//==============================================================================
// ������ ����������
//==============================================================================
void SlotWidget::calc_statistic()
{
  y_mean=0;y_std=0;
  y_min=y_max=plot_data_y.value(0);
  double y_offset;
  foreach( double y, plot_data_y )
  {
    if( y > y_max ) y_max = y;
    if( y < y_min ) y_min = y;
  }
  if( y_min < 0. ) y_offset = fabs( y_min );
  else y_offset = 0.;

  y_max += y_offset;
  y_min += y_offset;

  foreach( double y, plot_data_y )
  {
    y_mean += y+y_offset;
    y_std  += (y+y_offset)*(y+y_offset);
  }

  y_mean /= plot_data_y.size();
  y_std  /= plot_data_y.size();
  y_std  -= y_mean*y_mean;
  y_std   = sqrt( y_std );

  if( !use_line_trans)
  {
    ui->le_mean        -> setText( QString::number( y_mean-y_offset ) );
    ui->le_diff        -> setText( QString::number( y_max-y_min ) );
  }

  ui->le_std           -> setText( QString::number( y_std ) );

  if( ( y_mean != 0 ) && (y_max != y_min ) )
  {
       emit signalGetEffBitsRef( eff_bits_ref.column-1, eff_bits_ref.row-1 );
       if( eb_ref != 0.0 )
       {
         ui->le_noise_percent -> setText( QString::number( fabs((y_max-y_min) / eb_ref )*100 ) );
         ui->le_noise_db -> setText( QString::number( (6.020599913*(log(eb_ref/(y_max-y_min))/log(2.0))+1.76),'f',1));
         //-20*log10( fabs( (y_max-y_min) / y_mean ) ),'f' ,1 ) );old variant
         ui->le_eff_bits -> setText( QString::number( log(eb_ref/(y_max-y_min))/log(2.0),'f' ,1 ) );
         //( -20*log10( fabs( (y_max-y_min) / y_mean ) )) / 6.0205999,'f' ,1 ) ); ������ ������ ���. ��������
       }
       else
       {
         ui->le_noise_percent ->setText( "---" );
         ui->le_noise_db      ->setText( "---" );
         ui->le_eff_bits      ->setText( "---" );
       }
  } else
  { ui->le_noise_percent->clear();
    ui->le_noise_db->clear();
    ui->le_eff_bits->clear();
  }

  // ������ �����������
  double a;
  int i,j;
  a = (y_max-y_min)/(hist_plot_data_y_len-1);
  memset( hist_plot_data_y, 0 , sizeof( hist_plot_data_y ) );
  if(a)
  { for(i=0;i<plot_data_y.size();i++ )
    { j = (int)((plot_data_y[i] - y_min) / a)+1;
      if( j < 0 ) j=0;
      if( j > (hist_plot_data_y_len-1) ) j=(hist_plot_data_y_len-1);
      hist_plot_data_y[j]+=1;
    }

    static const int hist_sqr_len=8;
    int hist_sqr[hist_sqr_len];
    int hist_sqr_sum=0;
    int ind,sqr_sum_part,diff;
    for(i=0;i<8;i++)
    {
      hist_sqr[i]   = (int)(( hist_plot_data_y[i+1] - hist_plot_data_y[i] ) / 2);
      hist_sqr_sum += hist_sqr[i];
    }

    diff = hist_sqr_sum;
    ind = 1;
    for(i=1;i<8;i++)
    {
      sqr_sum_part = 0;
      for(j=0;j<=i;j++)
      {
        sqr_sum_part += hist_sqr[j];
      }
      if(abs(sqr_sum_part - (hist_sqr_sum-sqr_sum_part)) < diff )
      {
        diff = abs(sqr_sum_part - (hist_sqr_sum-sqr_sum_part));
        ind  = i;
      }
    }
    ui->le_eff_bits->setText( QString::number( ind ));

    if( ind < 8 )
    {
      for(i=ind;i<=8;i++)
      {
        for(j=15;j>0;j--)
        {
          hist_plot_data_y[j] = hist_plot_data_y[j-1];
        }
        hist_plot_data_y[0] = 0;
      }
    }
    else if( ind > 8 )
    {
       for(i=8;i<ind;i++)
      {
        for(j=0;j<15;j++)
        {
          hist_plot_data_y[j] = hist_plot_data_y[j+1];
        }
        hist_plot_data_y[15] = 0;
      }
    }
  }
  ui->hist->replot();

  //���� ������������ �������� ��������������
  if( use_line_trans )
  {
    y_mean=0;
    y_min=y_max=plot_data_y_trans.value(0);
    double y_offset_trans;
    foreach( double y, plot_data_y_trans )
    {
      if( y > y_max ) y_max = y;
      if( y < y_min ) y_min = y;
    }
    if( y_min < 0. ) y_offset_trans = fabs( y_min );
    else y_offset_trans = 0.;

    y_max += y_offset_trans;
    y_min += y_offset_trans;

    foreach( double y, plot_data_y_trans )
    {
      y_mean += y+y_offset_trans;
    }

    y_mean /= plot_data_y_trans.size();

    ui->le_mean -> setText( QString::number( y_mean-y_offset_trans ) );
    ui->le_diff -> setText( QString::number( y_max-y_min ) );
  }
}
//==============================================================================
//
//==============================================================================
void SlotWidget::slotSendEffBitsRef( double ref )
{
  eb_ref = ref;
}
//==============================================================================
//
//==============================================================================
void SlotWidget::slot_line_trans_used( bool us )
{
  use_line_trans = us;
  settings.remove( "use_line_trans" );
  if( us ) settings["use_line_trans"] = "1";
}
//###################################################################
/// ������ � ��������������� �������
//###################################################################
SlotDialog::SlotDialog( QWidget *parent, QMap<QString,QString> *settings )
  : QDialog( parent ), ui( new Ui::SlotDialog )
{
  ui->setupUi( this );
  setWindowTitle("��������������� �������");

  this->settings = settings;

  ui->xscale_int_a       -> setValidator( new QDoubleValidator( this ) );
  ui->xscale_int_b       -> setValidator( new QDoubleValidator( this ) );
  ui->xscale_coeff_value -> setValidator( new QDoubleValidator( this ) );
  ui->yscale_coeff_value -> setValidator( new QDoubleValidator( this ) );

  ui->le_x1              -> setValidator( new QDoubleValidator( this ) );
  ui->le_x2              -> setValidator( new QDoubleValidator( this ) );
  ui->le_y1              -> setValidator( new QDoubleValidator( this ) );
  ui->le_y2              -> setValidator( new QDoubleValidator( this ) );


  connect( ui->xscale_none,    SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->yscale_none,    SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->xscale_coeff,   SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->yscale_coeff,   SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->xscale_int,     SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->xscale_base,    SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->yscale_base,    SIGNAL( toggled(bool) ), this, SLOT( radio_buttons() ) );
  connect( ui->chb_line_trans, SIGNAL( toggled(bool)) , this, SLOT( radio_buttons() ) );

  ui->xscale_none->setChecked( true );
  ui->yscale_none->setChecked( true );

  // ������ ��������
  if( !settings ) return;
  const QMap<QString,QString> &csettings = *settings;

  QRegExp rx1("^(.+)/(.+)$");
  QRegExp rx2("^(.+)/(.+)/(.+)$");
  QRegExp rx3("^(.+)/(.+)/(.+)/(.+)$");
  QString str;

  // ��������
  ui->xname->setText( csettings["xname"] );
  ui->yname->setText( csettings["yname"] );

  // ������� - �����������
  if( csettings.contains("xscale_coeff") )
  { ui->xscale_coeff->setChecked( true );
    ui->xscale_coeff_value->setText( csettings["xscale_coeff"] );
  }
  if( csettings.contains("yscale_coeff") )
  { ui->yscale_coeff->setChecked( true );
    ui->yscale_coeff_value->setText( csettings["yscale_coeff"] );
  }

  // ������� - ��������
  str = csettings["xscale_interval"];
  if( rx1.indexIn( str ) == 0 )
  { ui->xscale_int->setChecked( true );
    ui->xscale_int_a->setText( rx1.cap(1) );
    ui->xscale_int_b->setText( rx1.cap(2) );
  }

  // ������� - ����
  str = csettings["xscale_base"];
  if( rx2.indexIn( str ) == 0 )
  { ui->xscale_base->setChecked( true );
    ui->xscale_base_m->setValue( rx2.cap(1).toInt() );
    ui->xscale_base_s->setValue( rx2.cap(2).toInt() );
    ui->xscale_base_n->setValue( rx2.cap(3).toInt() );
  }
  str = csettings["yscale_base"];
  if( rx2.indexIn( str ) == 0 )
  { ui->yscale_base->setChecked( true );
    ui->yscale_base_m->setValue( rx2.cap(1).toInt() );
    ui->yscale_base_s->setValue( rx2.cap(2).toInt() );
    ui->yscale_base_n->setValue( rx2.cap(3).toInt() );
  }
  ui->yunsigned->setChecked( csettings["yunsigned"] == "1" );

  //�������� �������� �������
  str = csettings["eff_bits_ref"];
  if( rx1.indexIn( str ) == 0 )
  {
    ui->eff_bits_ref_column->setValue( rx1.cap(1).toInt() );
    ui->eff_bits_ref_row   ->setValue( rx1.cap(2).toInt() );
  }

  ui->chb_line_trans->setChecked( csettings["use_line_trans"] == "1" );

  str = csettings["line_trans_coords"];
  if( rx3.indexIn( str ) == 0 )
  {
    ui->le_x1->setText( rx3.cap(1) );
    ui->le_y1->setText( rx3.cap(2) );
    ui->le_x2->setText( rx3.cap(3) );
    ui->le_y2->setText( rx3.cap(4) );
  }
}

//==============================================================================
// Destructor
//==============================================================================
SlotDialog::~SlotDialog()
{
  delete ui;
}

//==============================================================================
// ��������� ���������/���������� ��������� ������� �������
//==============================================================================
void SlotDialog::radio_buttons()
{
  ui->xscale_coeff_value -> setEnabled( ui->xscale_coeff->isChecked()   );
  ui->xscale_int_a       -> setEnabled( ui->xscale_int->isChecked()     );
  ui->xscale_int_b       -> setEnabled( ui->xscale_int->isChecked()     );
  ui->xscale_base_m      -> setEnabled( ui->xscale_base->isChecked()    );
  ui->xscale_base_s      -> setEnabled( ui->xscale_base->isChecked()    );
  ui->xscale_base_n      -> setEnabled( ui->xscale_base->isChecked()    );
  ui->yscale_coeff_value -> setEnabled( ui->yscale_coeff->isChecked()   );
  ui->yscale_base_m      -> setEnabled( ui->yscale_base->isChecked()    );
  ui->yscale_base_s      -> setEnabled( ui->yscale_base->isChecked()    );
  ui->yscale_base_n      -> setEnabled( ui->yscale_base->isChecked()    );
  ui->le_x1              -> setEnabled( ui->chb_line_trans->isChecked() );
  ui->le_x2              -> setEnabled( ui->chb_line_trans->isChecked() );
  ui->le_y1              -> setEnabled( ui->chb_line_trans->isChecked() );
  ui->le_y2              -> setEnabled( ui->chb_line_trans->isChecked() );
}


//==============================================================================
// ������� ������ OK
//==============================================================================
void SlotDialog::accept()
{
  if( !settings ) return;
  QMap<QString,QString> &isettings = *settings;

  QString str;

  isettings.remove("xname");
  isettings.remove("yname");
  isettings.remove("xscale_coeff");
  isettings.remove("yscale_coeff");
  isettings.remove("xscale_interval");
  isettings.remove("xscale_base");
  isettings.remove("yscale_base");
  isettings.remove("yunsigned");
  isettings.remove("eff_bits_ref");
  isettings.remove("use_line_trans");
  isettings.remove("line_trans_coords");


  // �������� ����
  str = ui->xname->text();  if( !str.isEmpty() ) isettings["xname"] = str;
  str = ui->yname->text();  if( !str.isEmpty() ) isettings["yname"] = str;

  // ������� - �����������
  if( ui->xscale_coeff->isChecked() )
  { isettings["xscale_coeff"] = ui->xscale_coeff_value->text();
  }
  if( ui->yscale_coeff->isChecked() )
  { isettings["yscale_coeff"] = ui->yscale_coeff_value->text();
  }
  // ������� - ��������
  if( ui->xscale_int->isChecked() )
  { isettings["xscale_interval"] = ui->xscale_int_a->text()+"/"+ui->xscale_int_b->text();
  }
  // ������� - ����
  if( ui->xscale_base->isChecked() )
  { isettings["xscale_base"] = QString().sprintf("%d/%d/%d", ui->xscale_base_m->value(),
                                                             ui->xscale_base_s->value(),
                                                             ui->xscale_base_n->value() );
  }
  if( ui->yscale_base->isChecked() )
  { isettings["yscale_base"] = QString().sprintf("%d/%d/%d", ui->yscale_base_m->value(),
                                                             ui->yscale_base_s->value(),
                                                             ui->yscale_base_n->value() );
  }
  // ����������� �����
  if( ui->yunsigned->isChecked() )
  { isettings["yunsigned"] = "1";
  }

  isettings["eff_bits_ref"] = QString().sprintf("%d/%d", ui->eff_bits_ref_column->value(),
                                                            ui->eff_bits_ref_row->value());

  if( ui->chb_line_trans->isChecked() )
  { isettings["use_line_trans"] = "1";
  }

  isettings["line_trans_coords"] = ui->le_x1->text()+"/"+ui->le_y1->text()+"/"+
                                   ui->le_x2->text()+"/"+ui->le_y2->text();

  done( QDialog::Accepted );
}
