#include <QtGui>

#include "mbmasterwidget.h"
#include "mbcommon.h"
#include "slotwidget.h"
#include "mbmasterxml.h"
#include "mbmaster_p.h"

#include "ui_mbmasterwidget.h"

struct MTMSlot
{
  int        flag;
  int        status;
  int        n;
  int        mm_index;
  MMModule   module;
  QString    name;
  MBDataType datatype;
  QString    addr;
  int        len;
  QString    desc;
};

//===================================================================
//
//===================================================================
class MBMasterWidgetTableModel : public QAbstractTableModel
{
  public:
    MBMasterWidgetTableModel( QObject *parent = 0);
  public:
    int rowCount    ( const QModelIndex & parent = QModelIndex() ) const
      { Q_UNUSED( parent ); return table.count(); }
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const
      { Q_UNUSED( parent ); return 5; }
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation,
                                   int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    bool setData ( const QModelIndex & index,
                   const QVariant & value, int role = Qt::EditRole );
  public:
    void refresh();
    void update_slots( MBMasterXML *mm );

    QVector<MTMSlot> table;
};

//##############################################################################
//
//##############################################################################
MBMasterWidget::MBMasterWidget( QWidget *parent )
  : QWidget( parent ), ui( new Ui::MBMasterWidget )
{
  ui->setupUi( this );
  mbmaster = 0;
  mbmodel = new MBMasterWidgetTableModel( this );
  mktable_minimize_flag = false;

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
void MBMasterWidget::setMBMaster( MBMasterXML *mm )
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
  int m = mbmaster->d->mmslots.count();
  for( i=0; i<n; i++ )
  { j = mbmodel->table[i].mm_index;
    if( j < 0   ) continue;
    if( j >= m  ) continue;
    mbmodel->setData( mbmodel->index(i,0),mbmaster->d->mmslots[j].status, Qt::UserRole );
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
  SlotWidget *p = new SlotWidget( this, mbmaster, slot.module.n, slot.n, mktable_minimize_flag );
  connect( p,    SIGNAL( attributes_saved(int,int,QString) ),
           this, SIGNAL( attributes_saved(int,int,QString) ) );
  connect( this, SIGNAL( signalMkTableMinimizeStateChange( bool ) ),
           p,    SLOT  ( updateMinimizeButtonState( bool ) ) );
  connect( p,    SIGNAL( signalMinimize(bool)),
           this, SIGNAL( signalMinimize(bool)));
  connect( p,    SIGNAL( signalOstsOpen() ),
           this, SIGNAL( signalOstsOpen() ) );
  connect( p,    SIGNAL( signalOstsClose() ),
           this, SIGNAL( signalOstsClose() ) );

  emit signalOstsOpen();

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
void MBMasterWidget::slotMkTableMinimizeStateChange( bool state )
{
  mktable_minimize_flag = state;
  emit signalMkTableMinimizeStateChange( mktable_minimize_flag );
}
//==============================================================================
//
//==============================================================================
void MBMasterWidget::slotMkTableMinimizeAllHide( bool state )
{ mktable_minimize_flag = state;
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
void MBMasterWidgetTableModel::update_slots( MBMasterXML *mm )
{
  QMutexLocker locker(&mm->d->mutex);

  int i;
  int len = mm->d->mmslots.count();

  table.clear();

  MTMSlot ss;
  MMModule module;
  MMModule module_old;
  module_old.n = -1;

  for( i=0; i<len; i++ )
  { module = mm->d->mmslots[i].module;
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
    ss.n        = mm->d->mmslots[i].n;
    ss.mm_index = i;
    ss.datatype = mm->d->mmslots[i].datatype;
    if( ss.datatype.isRegister() ) ss.addr = QString::number( mm->d->mmslots[i].addr,10 );
    else ss.addr = QString::number( mm->d->mmslots[i].addr,16 ).toUpper();
    ss.len      = mm->d->mmslots[i].len;    
    ss.desc     = mm->d->mmslots[i].desc;
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
