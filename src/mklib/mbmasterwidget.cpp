#include <QtGui>
#include "mbmasterwidget.h"

#include "mbcommon.h"
#include "slotwidget.h"

#include "ui_mbmasterwidget.h"
#include "ui_mbmslotwidget.h"
#include "ui_mbmslotexportdialog.h"

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>

//##############################################################################
//
//##############################################################################
MBMasterWidget::MBMasterWidget( QWidget *parent )
  : QWidget( parent ), ui( new Ui::MBMasterWidget )
{
  ui->setupUi( this );

  mbmaster = 0;
  mbmodel = new MBMasterWidgetTableModel( this );

  ui->tw->setSelectionBehavior( QAbstractItemView::SelectRows );
  ui->tw->setSelectionMode( QAbstractItemView::NoSelection );
  ui->tw->verticalHeader()->setDefaultSectionSize( ui->tw->font().pointSize()+11 );
  ui->tw->verticalHeader()->hide();
  ui->tw->setModel( mbmodel );
  ui->tw->horizontalHeader()->setStretchLastSection( true );
  ui->tw->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
  ui->tw->horizontalHeader()->setClickable( false );
}

//==============================================================================
//
//==============================================================================
MBMasterWidget::~MBMasterWidget()
{
  delete ui;
}

//==============================================================================
//
//==============================================================================
void MBMasterWidget::load_config( QDomDocument &doc )
{
  if( mbmaster ) mbmaster->load_configuration( doc );
  mbmodel->update_slots( mbmaster );
}

//==============================================================================
//
//==============================================================================
void MBMasterWidget::load_config( QByteArray &xml )
{
  if( mbmaster ) mbmaster->load_configuration( xml );
  mbmodel->update_slots( mbmaster );
}

//==============================================================================
//
//==============================================================================
void MBMasterWidget::clear_config()
{
  if( mbmaster ) mbmaster->clear_configuration();
  mbmodel->update_slots( mbmaster );
  delete_slotwidgets();
}

//==============================================================================
// запустить опрос
//==============================================================================
void MBMasterWidget::polling_start()
{
  if( mbmaster ) mbmaster->polling_start();
}
//==============================================================================
// остановить опрос
//==============================================================================
void MBMasterWidget::polling_stop()
{
  if( mbmaster ) mbmaster->polling_stop();
}

//==============================================================================
//
//==============================================================================
void MBMasterWidget::setMBMaster( MBMaster *mm )
{
  mbmaster = mm;
  connect( mm, SIGNAL( stateChanged() ), SLOT( stateUpdate() ),Qt::QueuedConnection);
}

//==============================================================================
//
//==============================================================================
void  MBMasterWidget::stateUpdate()
{
  int i,j;
  int n = mbmodel->table.count();
  int m = mbmaster->mmslots.count();
  for( i=0; i<n; i++ )
  { j = mbmodel->table[i].mm_index;
    if( j < 0   ) continue;
    if( j >= m  ) continue;
    mbmodel->setData( mbmodel->index(i,0),mbmaster->mmslots[j].status, Qt::UserRole );
  }
}

//==============================================================================
//
//==============================================================================
void MBMasterWidget::on_tw_doubleClicked ( const QModelIndex & index)
{
	int row = index.row();
	if( row >= mbmodel->table.count() ) return;
	const MTMSlot slot = mbmodel->table[row];

  SlotWidget *p = new SlotWidget( this, mbmaster, slot.module.n, slot.n );
  connect( p,    SIGNAL( attributes_saved(int,int,QString) ),
           this, SIGNAL( attributes_saved(int,int,QString) ) );
  slotwidgets.add( p );
  p->show();
  p->activateWindow();

/*
  // старый вариант окна слота

	int row = index.row();
	if( row >= mbmodel->table.count() ) return;

  MBMasterSlotWidget *p = new MBMasterSlotWidget( this,
                                        mbmaster, mbmodel->table[row] );
  slotwidgets.add( p );
  p->show();
  p->activateWindow();

*/
}

//==============================================================================
//
//==============================================================================
void MBMasterWidget::delete_slotwidgets()
{
  slotwidgets.clear();
}

//##############################################################################
//
//#############################################################################
MBMasterWidgetTableModel::MBMasterWidgetTableModel( QObject *parent )
  : QAbstractTableModel( parent )
{
}

//==============================================================================
//
//==============================================================================
QVariant MBMasterWidgetTableModel::data ( const QModelIndex & index, int role ) const
{
  static QColor color_gray("grey");
  static QColor color_lightgray("lightgray");
  static QColor color_lightgreen("lightgreen");
  static QColor color_pink("pink");
  //static QColor color_yellow("darkgreen");
  static QColor color_yellow("lightgray");

  QString str;
  int column = index.column();
  int row    = index.row();

  if( row >= table.count() ) return QVariant();

  if( role == Qt::TextAlignmentRole)
  { switch( column )
    { case(0):
        if( table[row].flag == 1  )
          return QVariant(Qt::AlignVCenter|Qt::AlignHCenter); // модуль
    	case(1):
    	case(2):
    	  return QVariant(Qt::AlignVCenter|Qt::AlignRight);
    	case(3):
    	case(4):
    	  return QVariant(Qt::AlignVCenter|Qt::AlignLeft);

   	}
 	}

  switch( table[row].flag )
  { case( 0 ): // обычный слот
      switch( role )
      { case( Qt::DisplayRole ):
         switch( column )
         { case( 0 ):  return table[row].n;    // N
           case( 1 ):  return table[row].addr; // Адрес
           case( 2 ):  return table[row].len;  // Кол-во
           case( 3 ):  return table[row].datatype.toName(); // Тип
           case( 4 ):  return table[row].desc; // Описание
          }
          break;
        case( Qt::BackgroundRole ):
          if( column == 0 )
          { switch( table[row].status )
            { case( MMSlot::NotInit ):  return color_lightgray;
              case( MMSlot::Ok ):       return color_lightgreen;
              case( MMSlot::Bad ):      return color_pink;
              case( MMSlot::Skiped ):   return color_yellow;
            }
          }
          break;
      }
      break;
    case( 1 ): // модуль
      switch( role )
      { case( Qt::DisplayRole ):
         switch( column )
         {
           case( 0 ):  return table[row].n;    // N
           case( 1 ):  return table[row].addr; // Адрес
           case( 4 ):
             str.clear();
             if( (!table[row].name.isEmpty()) && (!table[row].desc.isEmpty()) )
               str = " | ";
             return "Модуль "+ table[row].name + str + table[row].desc; // Описание
        }
        break;
        case( Qt::BackgroundRole ):
          return color_gray;
        case( Qt::FontRole ):
          QFont myfont;
          myfont.setBold( true );
          return myfont;

        break;
      }
      break;
  }

  return QVariant();
}

//==============================================================================
//
//==============================================================================
bool MBMasterWidgetTableModel::setData ( const QModelIndex & index,
                   const QVariant & value, int role )
{
  int row    = index.row();
  int column = index.column();

  if(( column==0 ) && (role = Qt::UserRole))
  { table[row].status = value.toInt();
    emit dataChanged( index,index );
    return true;
  }
  return false;
}

//==============================================================================
//
//==============================================================================
Qt::ItemFlags MBMasterWidgetTableModel::flags ( const QModelIndex & index ) const
{
  Q_UNUSED( index );
  return Qt::ItemIsEnabled;
}

//==============================================================================
//
//==============================================================================
QVariant MBMasterWidgetTableModel::headerData ( int section, Qt::Orientation orientation,
                                   int role  ) const
{
  if( orientation == Qt::Vertical ) return QVariant();

  const int len = 5;
  const static char* names[len] =
     { "N", "Адрес", "Кол-во", "Тип", "Описание" };
  const static int sizes[len] =
     { 30,    50,      50,      80,     80       };

  if( section < len )
  { switch( role )
    { case(Qt::DisplayRole):  return names[section];
      case(Qt::SizeHintRole): return QSize(sizes[section],20);
    }
  }
  return QVariant();
}

//==============================================================================
//
//==============================================================================
void MBMasterWidgetTableModel::update_slots( MBMaster *mm )
{
  QMutexLocker locker(&mm->mutex);

  int i;
  int len = mm->mmslots.count();

  table.clear();

  MTMSlot ss;
  MMModule module;
  MMModule module_old;
  module_old.n = -1;

  for( i=0; i<len; i++ )
  { module = mm->mmslots[i].module;
    if( module.n != module_old.n )
    { ss.flag = 1;
      ss.n    = module.n;
      ss.mm_index=-1;
      ss.addr = QString::number( module.node );
      if( module.subnode )
          ss.addr += "."+QString::number( module.subnode );
      ss.name = module.name;
      ss.desc = module.desc;
      ss.len = 0;
      ss.datatype = MBDataType::Unknown;
      module_old = module;
      table << ss;
    }
    ss.flag = 0;
    ss.status   = MMSlot::NotInit;
    ss.module   = module;
    ss.n        = mm->mmslots[i].n;
    ss.mm_index = i;
    ss.addr     = QString::number( mm->mmslots[i].addr,16 ).toUpper();
    ss.len      = mm->mmslots[i].len;
    ss.datatype = mm->mmslots[i].datatype;
    ss.desc     = mm->mmslots[i].desc;
    table << ss;
  }
  reset();

}

//==============================================================================
//
//==============================================================================
void MBMasterWidgetTableModel::refresh()
{
  emit dataChanged( index(0,0), index(table.count()-1,0) );
}

//##############################################################################
//
//##############################################################################
QMap<QString,QString> MBMasterSlotWidget::class_settings;
MBMasterSlotWidget::MBMasterSlotWidget( QWidget *parent, MBMaster *mm_arg,
                                                MTMSlot slot_arg )
 :  QMainWindow( parent ), ui( new Ui::MBMasterSlotWidget ),
    slot(slot_arg), mm( mm_arg )
{
  QString str;

  ui->setupUi( this );
  setWindowFlags( Qt::Tool );
  setAttribute( Qt::WA_DeleteOnClose, true );
  resize( 800,400 );

  QAction *act_export = ui->menubar->addAction("Экспорт");
  connect( act_export, SIGNAL( activated() ), this, SLOT( action_export_activated() ) );


  str =  "Cлот: "+slot.desc;
  str += " [ " + QString::number(slot.module.n) + "." +QString::number(slot.n);
  str += " | " + slot.datatype.toName() + " | " + slot.addr;
  str += " | " + QString::number(slot.len);
  str += " | " + slot.module.name + " | ";
  str += "адрес " + QString::number(slot.module.node);
  if( slot.module.subnode ) str += "."+QString::number(slot.module.subnode);
	str += " ]" + slot.module.desc ;
  setWindowTitle( str );

  if( slot.len <= 0 ) return;

  configure_table();
  //-----------
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
  //--------------
  ui->splitter->setStretchFactor(0,10);
  ui->splitter->setStretchFactor(1,10);
  ui->plot->hide();
  //--------------
  uncheck_all_formats();
  if( slot.datatype.id() == MBDataType::Floats )
  { ui->action_format_dec          -> setVisible( false );
    ui->action_format_unsigned     -> setVisible( false );
    ui->action_format_hex          -> setVisible( false );
    ui->action_format_float        -> setVisible( true  );
    ui->action_format_bin          -> setVisible( false );
    ui->action_format_text_cp1251  -> setVisible( false );
    ui->action_format_text_866     -> setVisible( false );
    ui->action_format_text_KOI8_R  -> setVisible( false );
    ui->action_format_float        -> setChecked( true  );
  } else
  { ui->action_format_dec          -> setChecked( true  );
    ui->action_format_float        -> setVisible( false );
  }

  ui->action_format_text_cp1251    -> setVisible( false );
  ui->action_format_text_866       -> setVisible( false );
  ui->action_format_text_KOI8_R    -> setVisible( false );
  //--------------

  str = class_settings.value( QString("%1/%2;table").arg(slot.module.n).arg(slot.n) );
  if( str.contains("hidden") ) ui->action_view_table->setChecked( false );
  if( str.contains("shown") )  ui->action_view_table->setChecked( true );
  str = class_settings.value( QString("%1/%2;plot").arg(slot.module.n).arg(slot.n) );
  if( str.contains("hidden") ) ui->action_view_plot->setChecked( false );
  if( str.contains("shown") )  ui->action_view_plot->setChecked( true );

  //--------------
  startTimer( 1000 );
}

//==============================================================================
//
//==============================================================================
MBMasterSlotWidget::~MBMasterSlotWidget()
{
  delete ui;
}

//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_pb_format_clicked()
{
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_dec_activated()
{
  uncheck_all_formats();
  ui->action_format_dec->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_hex_activated()
{
  uncheck_all_formats();
  ui->action_format_hex->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_bin_activated()
{
  uncheck_all_formats();
  ui->action_format_bin->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_float_activated()
{
  uncheck_all_formats();
  ui->action_format_float->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_unsigned_activated()
{
  uncheck_all_formats();
  ui->action_format_unsigned->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_text_cp1251_activated()
{
  uncheck_all_formats();
  ui->action_format_text_cp1251->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_text_866_activated()
{
  uncheck_all_formats();
  ui->action_format_text_866->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_text_KOI8_R_activated()
{
  uncheck_all_formats();
  ui->action_format_text_KOI8_R->setChecked( true );
  configure_table();
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_format_options_activated()
{
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_view_table_toggled( bool b)
{
  if( !b )
  { ui->tw->setFocus();
    ui->tw->clearSelection();
  }
  ui->tw->setShown( b );
  class_settings.insert( QString("%1/%2;table").arg(slot.module.n).arg(slot.n),
                   b ? "shown":"hidden" );
}
//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::on_action_view_plot_toggled(bool b)
{
  ui->plot->setShown( b );
  class_settings.insert( QString("%1/%2;plot").arg(slot.module.n).arg(slot.n),
                   b ? "shown":"hidden" );
}

//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::uncheck_all_formats()
{
  ui->action_format_dec          -> setChecked( false );
  ui->action_format_unsigned     -> setChecked( false );
  ui->action_format_hex          -> setChecked( false );
  ui->action_format_float        -> setChecked( false );
  ui->action_format_bin          -> setChecked( false );
  ui->action_format_text_cp1251  -> setChecked( false );
  ui->action_format_text_866     -> setChecked( false );
  ui->action_format_text_KOI8_R  -> setChecked( false );
}

//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::configure_table()
{
  int i,j,k;
  char *cf;

  cf = (char*)"";
  if( ui->action_format_dec->isChecked() )         cf = (char*)"";
  if( ui->action_format_unsigned->isChecked() )    cf = (char*)"unsigned";
  if( ui->action_format_hex->isChecked() )         cf = (char*)"hex";
  if( ui->action_format_bin->isChecked() )         cf = (char*)"bin";
  if( ui->action_format_float->isChecked() )       cf = (char*)"float";
  if( ui->action_format_text_cp1251->isChecked() ) cf = (char*)"text_cp1251";
  if( ui->action_format_text_866->isChecked() )    cf = (char*)"text_866";
  if( ui->action_format_text_KOI8_R->isChecked() ) cf = (char*)"text_koi8r";


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
  ui->tw->setMode( MKTable::Edit );
  ui->tw->setMBMaster( mm );
  ui->tw->loadConfiguration( doc );
  for(i=0; i<ui->tw->rowCount(); i++ )
  { ui->tw->setVerticalHeaderItem( i, new QTableWidgetItem( QString::number(10*i) ) );
  }
  ui->tw->verticalHeader()->show();
  ui->tw->setMode( MKTable::Polling );
}

//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::timerEvent( QTimerEvent *event)
{
  Q_UNUSED( event );

  if( ui->plot->isHidden() ) return;

  int i,mask;
  bool b_unsigned =    ui->action_format_unsigned->isChecked()
                    || ui->action_format_hex->isChecked()
                    || ui->action_format_bin->isChecked();


  MMSlot ss = mm->getSlot( slot.module.n, slot.n );

  switch( ss.datatype.id() )
  { case( MBDataType::Bytes ): mask = 0xFF;       break;
    case( MBDataType::Words ): mask = 0xFFFF;     break;
    default:                   mask = 0xFFFFFFFF; break;
  }

  if( b_unsigned && ( ss.datatype.id() == MBDataType::Floats ) ) b_unsigned=false;

  int n = ss.data.size()+1;
  plot_data_x.resize( n );
  plot_data_y.resize( n );
  for( i=1; i<n; i++ )
  { plot_data_y[i] =   b_unsigned
                     ? (double)( (unsigned int)ss.data[i-1].toInt() & mask )
                     :  ss.data[i-1].toDouble();
    plot_data_x[i] = i;
  }
  plot_data_x[0] = 0;
  plot_data_y[0] = plot_data_y[1];
  plot_curve->setData( plot_data_x.constData(), plot_data_y.constData(),n );
  ui->plot->replot();
}

//==============================================================================
//
//==============================================================================
void MBMasterSlotWidget::action_export_activated()
{
  MBMasterSlotExportDialog dialog;
  dialog.exec();

  //QString str;
  //QApplication::clipboard()->setText( str );
  //QMessageBox::information( this, "Слот", "Данные скопированы в буфер обмена." );
}

//##############################################################################
//
//##############################################################################
MBMasterSlotExportDialog::MBMasterSlotExportDialog( QWidget *parent )
  : QDialog( parent ), ui( new Ui::MBMasterSlotExportDialog )
{
  ui->setupUi(this);
  setWindowTitle( "Экспорт" );
}

//==============================================================================
//
//==============================================================================
MBMasterSlotExportDialog::~MBMasterSlotExportDialog()
{
  delete ui;
}

