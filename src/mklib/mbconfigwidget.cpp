#include <QtGui>
#include <QtXml>

#include "mbconfigwidget.h"
#include "ui_mbconfigwidget.h"

#include "mbcommon.h"

#define MODULES_MAX_N   50
#define SLOTS_MAX_N    100

class ModulesModel : public QAbstractTableModel
{
public:

  struct t_module_desc
  { QString addr;
    QString name;
    QString desc;
  };

  int rowCount    ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return MODULES_MAX_N; }
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return 4; }
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QVariant headerData ( int section, Qt::Orientation orientation,
                                   int role = Qt::DisplayRole ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  bool setData ( const QModelIndex & index,
                   const QVariant & value, int role = Qt::EditRole );
  t_module_desc md[MODULES_MAX_N];
  void refresh();
};

class SlotsModel : public QAbstractTableModel
{
public:

  struct t_slot_desc
  { QString addr;
    int     n;
    MBDataType type;
    QString desc;
    QString attributes;
  };

  int rowCount    ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return SLOTS_MAX_N; }
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return 5; }
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QVariant headerData ( int section, Qt::Orientation orientation,
                                   int role = Qt::DisplayRole ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  bool setData ( const QModelIndex & index,
                   const QVariant & value, int role = Qt::EditRole );
  void set_current_module( int i );
  int current_module;
  t_slot_desc sd[MODULES_MAX_N][SLOTS_MAX_N];
};

struct MBConfigWidgetPrivate
{
  ModulesModel       modules_model;
  SlotsModel         slots_model;

  ModulesModel::t_module_desc mbuffer;
  SlotsModel::t_slot_desc     sbuffer[SLOTS_MAX_N];

};

//###################################################################
//
//###################################################################
MBConfigWidget::MBConfigWidget( QWidget *parent )
  : QWidget( parent ), ui( new Ui::MBConfigWidget ),
    d( new MBConfigWidgetPrivate )
{
  int i,j;
  ui->setupUi( this );
  ui->splitter->setStretchFactor(0,10);
  ui->splitter->setStretchFactor(1,15);

  for( i=0; i<MODULES_MAX_N; i++ )
  { for( j=0; j<SLOTS_MAX_N; j++ )
    { d->slots_model.sd[i][j].n=0;
      d->slots_model.sd[i][j].type = MBDataType::Bits;
    }
  }
  d->slots_model.current_module=-1;

  ui->tw1->setContextMenuPolicy( Qt::CustomContextMenu );
  ui->tw1->setSelectionBehavior( QAbstractItemView::SelectRows );
  ui->tw1->setSelectionMode( QAbstractItemView::SingleSelection );
  ui->tw1->verticalHeader()->setDefaultSectionSize( ui->tw1->font().pointSize()+11 );
  ui->tw1->verticalHeader()->hide();
  ui->tw1->setModel( &d->modules_model );
  ui->tw1->horizontalHeader()->setStretchLastSection( true );
  ui->tw1->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
  ui->tw1->horizontalHeader()->setClickable( false );
  connect( ui->tw1->selectionModel(),
          SIGNAL( currentRowChanged(const QModelIndex&, const QModelIndex&) ),
          this,
          SLOT( tw1_currentRowChanged(const QModelIndex&, const QModelIndex&)));
  ui->tw1->selectRow(0);

  ui->tw2->setContextMenuPolicy( Qt::CustomContextMenu );
  ui->tw2->setSelectionBehavior( QAbstractItemView::SelectRows );
  ui->tw2->setSelectionMode( QAbstractItemView::SingleSelection );
  ui->tw2->verticalHeader()->setDefaultSectionSize( ui->tw1->font().pointSize()+11 );
  ui->tw2->verticalHeader()->hide();
  ui->tw2->setModel( &d->slots_model );
  ui->tw2->horizontalHeader()->setStretchLastSection( true );
  ui->tw2->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
  ui->tw2->horizontalHeader()->setClickable( false );
}

//===================================================================
//
//===================================================================
MBConfigWidget::~MBConfigWidget()
{
  delete ui;
  delete d;
}

//===================================================================
//
//===================================================================
void MBConfigWidget::on_tw1_customContextMenuRequested ( const QPoint& pos)
{
  int i;

  QMenu menu;
  QAction *act_copy  = menu.addAction(QIcon(":/icons/res/copy.png"),  "Копировать" );
  QAction *act_paste = menu.addAction(QIcon(":/icons/res/paste.png"), "Вставить"   );
  QAction *act = menu.exec( ui->tw1->mapToGlobal( pos ) );
  if( act == 0 ) return;

  int r = ui->tw1->selectionModel()->selectedRows().value(0).row();
  if(( r < 0) || (r >= MODULES_MAX_N ) ) return;

  if( act == act_copy )
  { d->mbuffer = d->modules_model.md[ r ];
    for( i=0; i<SLOTS_MAX_N; i++ )
    { d->sbuffer[i]=d->slots_model.sd[r][i];
    }
  } else if( act == act_paste )
  { d->modules_model.md[ r ] = d->mbuffer;
    for( i=0; i<SLOTS_MAX_N; i++ )
    { d->slots_model.sd[r][i]=d->sbuffer[i];
    }
    d->modules_model.refresh();
    ui->tw1->selectRow(r);
  }
  return;
}

//===================================================================
//
//===================================================================
void MBConfigWidget::on_tw2_activated ( const QModelIndex & index)
{
  ui->tw2->model()->setData( index, 1 , Qt::UserRole );
}

//===================================================================
//
//===================================================================
void MBConfigWidget::tw1_currentRowChanged(const QModelIndex&current,
                             const QModelIndex&previous)
{
  Q_UNUSED( previous );
  d->slots_model.set_current_module( current.row() );
  ui->tw2->clearSelection();
}

//===================================================================
//! Очистить конфигурацию
//===================================================================
void MBConfigWidget::clearConfiguration()
{
  int i,j;

  for( i=0; i<MODULES_MAX_N; i++ )
  { d->modules_model.md[i].addr.clear();
    d->modules_model.md[i].name.clear();
    d->modules_model.md[i].desc.clear();
    for( j=0; j<SLOTS_MAX_N; j++ )
    { d->slots_model.sd[i][j].addr.clear();
      d->slots_model.sd[i][j].n=0;
      d->slots_model.sd[i][j].type = MBDataType::Bits;
      d->slots_model.sd[i][j].desc.clear();
      d->slots_model.sd[i][j].attributes.clear();
    }
  }
  d->slots_model.current_module=-1;

  d->modules_model.refresh();
  d->slots_model.set_current_module(0);
  ui->tw1->selectRow(0);
}

//===================================================================
//! Загрузить конфигурацию
/*!
   \param doc XML документ с описанием конфигурации
*/
//===================================================================
void MBConfigWidget::loadConfiguration( const QDomDocument &doc )
{
  int i,j;
  int n,k;

  clearConfiguration();

  QDomElement element;
  QDomNodeList slot_list;
  QString str;

  QDomElement docElem = doc.documentElement();
  QDomNodeList module_list = doc.elementsByTagName("Module");
  for(i=0; i<module_list.count(); i++ )
  { element = module_list.item( i ).toElement();
    n = element.attribute("N").toInt()-1;
    if( ( n >= MODULES_MAX_N ) || ( n < 0 )) continue;
    d->modules_model.md[n].addr = element.attribute("Node");
    d->modules_model.md[n].name = element.attribute("Name");
    d->modules_model.md[n].desc = element.attribute("Desc");
    slot_list = element.elementsByTagName("Slot");
    for(j=0; j<slot_list.count(); j++ )
    { element = slot_list.item( j ).toElement();
      k = element.attribute("N").toInt()-1;
      if( ( k >= SLOTS_MAX_N ) || ( k < 0 )) continue;
      d->slots_model.sd[n][k].addr = element.attribute("Addr");
      d->slots_model.sd[n][k].n    = element.attribute("Length").toInt();
      str = element.attribute("Type");
      d->slots_model.sd[n][k].type.fromTextId( str );
      str = element.attribute("Operation");
      d->slots_model.sd[n][k].desc       = element.attribute("Desc");
      d->slots_model.sd[n][k].attributes = element.attribute("Attributes");
    }
  }

  d->modules_model.refresh();
  d->slots_model.set_current_module(0);
  ui->tw1->selectRow(0);
}

//===================================================================
//! Загрузить конфигурацию
/*!
   \param xml массив данных с конфигурацией (текст xml-описания)
*/
//===================================================================
void MBConfigWidget::loadConfiguration( const QByteArray &xml )
{
  QDomDocument doc;
  doc.setContent( xml, false );
  loadConfiguration( doc );
}

//===================================================================
//! Получить текущую конфигурацию
/*!
   \param doc XML документ с описанием конфигурации
*/
//===================================================================
void MBConfigWidget::saveConfiguration( QDomDocument &doc )
{
  int i,j;
  QString str;

  QDomElement root = doc.createElement("MBConfig");
  doc.appendChild(root);

  for(i=0; i<MODULES_MAX_N; i++ )
  {
    if( d->modules_model.md[i].addr.size()==0 ) continue;
    QDomElement module = doc.createElement("Module");
    module.setAttribute("N",i+1);
    module.setAttribute("Node", d->modules_model.md[i].addr );
    module.setAttribute("Name", d->modules_model.md[i].name );
    str = d->modules_model.md[i].desc;
    if( !str.isEmpty() )
    {  module.setAttribute("Desc", str );
    }
    root.appendChild(module);
    for(j=0; j<SLOTS_MAX_N; j++ )
    { if( d->slots_model.sd[i][j].addr.size() == 0 ) continue;
      QDomElement slot = doc.createElement("Slot");
      slot.setAttribute("N",j+1);
      slot.setAttribute("Addr",   d->slots_model.sd[i][j].addr );
      slot.setAttribute("Length", d->slots_model.sd[i][j].n    );
      str = d->slots_model.sd[i][j].type.toTextId();
      slot.setAttribute("Type", str );
      str = d->slots_model.sd[i][j].desc;
      if( !str.isEmpty() )
      { slot.setAttribute("Desc", str );
      }
      str = d->slots_model.sd[i][j].attributes;
      if( !str.isEmpty() )
      { slot.setAttribute("Attributes", str );
      }
      module.appendChild(slot);
    }
  }
}

//===================================================================
//! Задать аттрибуты слота
/*!
   \param module     Номер модуля
   \param slot       Номер слота
   \param attributes Аттрибуты
*/
//===================================================================
void MBConfigWidget::setSlotAttributes(int module, int slot, const QString &attributes)
{
  if( ( module <= 0 ) || ( module > MODULES_MAX_N ) ) return;
  if( ( slot   <= 0 ) || ( slot   > SLOTS_MAX_N   ) ) return;
  module--;
  slot--;
  d->slots_model.sd[module][slot].attributes = attributes;
}

//===================================================================
//! Получить текущую конфигурацию
/*!
   \param xml массив данных с конфигурацией (текст xml-описания)
*/
//===================================================================
void MBConfigWidget::saveConfiguration( QByteArray &xml )
{
  QDomDocument doc;
  saveConfiguration( doc );
  xml = doc.toByteArray( 2 );
}

//===================================================================
//! Экспортировать текущую конфигурацию в текстовый файл
/*!
   \return Текстовое описание конфигурации
*/
//===================================================================
QString MBConfigWidget::exportConfiguration()
{
  int i,j,maxlen,a;
  QString str,s;

  for(i=0; i<MODULES_MAX_N; i++ )
  {
    if( d->modules_model.md[i].addr.size()==0 ) continue;

    str += "Модуль  : " + d->modules_model.md[i].name + "\n";
    str += "Узел    : " + d->modules_model.md[i].addr + "\n";
    s = d->modules_model.md[i].desc;
    if( !s.isEmpty() )  str += "Описание: " + s + "\n";

    maxlen=0;
    for(j=0; j<SLOTS_MAX_N; j++ )
    { if( d->slots_model.sd[i][j].addr.size() == 0 ) continue;
      a = d->slots_model.sd[i][j].desc.length();
      if( a > maxlen ) maxlen=a;
    }

    str +=  QString("Описание").leftJustified(maxlen, ' ') + "\t";
    str +=  QString("Адрес").leftJustified(8, ' ') + "\t";;
    str +=  QString("Тип").leftJustified(8, ' ') + "\t";;
    str +=  QString("Кол-во").leftJustified(8, ' ') + "\n";

    for(j=0; j<SLOTS_MAX_N; j++ )
    { if( d->slots_model.sd[i][j].addr.size() == 0 ) continue;

      str +=  d->slots_model.sd[i][j].desc.leftJustified(maxlen, ' ') + "\t";
      str +=  d->slots_model.sd[i][j].addr.leftJustified(8, ' ') + "\t";
      str +=  d->slots_model.sd[i][j].type.toName().leftJustified(8, ' ') + "\t";
      str +=  QString::number(d->slots_model.sd[i][j].n).leftJustified(8, ' ');
      str +=  "\n";
    }
    str +=  "\n";
  }
  return str;
}

//###################################################################
//
//###################################################################
QVariant ModulesModel::data ( const QModelIndex & index, int role ) const
{
  int row = index.row();
  int column = index.column();

  switch( role )
  { case( Qt::DisplayRole ):
    case( Qt::EditRole ):
      switch( column )
      { case (0): return QString::number( index.row() + 1 );
        case (1): return md[row].addr;
        case (2): return md[row].name;
        case (3): return md[row].desc;
      }
      break;
    case( Qt::TextAlignmentRole ):
      switch( column )
      { case (0): return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case (1): return QVariant(Qt::AlignRight | Qt::AlignVCenter);
      }
      break;
    case( Qt::ToolTipRole   ):
    case( Qt::WhatsThisRole ):
      switch( column )
      { //case(1): return( md[row].addr+" 1234" );
      }
      break;
  }
  return QVariant();
}

//===================================================================
//
//===================================================================
QVariant ModulesModel::headerData ( int section, Qt::Orientation orientation,
                                   int role  ) const
{
  if( orientation != Qt::Horizontal ) return QVariant();
  switch( role )
  { case( Qt::DisplayRole ):
      switch( section )
      { case (0): return("N");
        case (1): return("Узел");
        case (2): return("Название");
        case (3): return("Описание");
      }
      break;
    case( Qt::SizeHintRole ):
      switch( section )
      { case (0): return QSize(30,20);
        case (1): return QSize(50,20);
        case (2): return QSize(100,20);
        case (3): return QSize(100,20);
      }
      break;
  }
  return QVariant();
}

//===================================================================
//
//===================================================================
Qt::ItemFlags ModulesModel::flags ( const QModelIndex & index ) const
{
  int column = index.column();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
  if( column > 0 ) flags |= Qt::ItemIsEditable;

  return flags;
}

//===================================================================
//
//===================================================================
bool ModulesModel::setData ( const QModelIndex & index,
                   const QVariant & value, int role )
{
  int row = index.row();
  int column = index.column();

  QString str;
  QRegExp rx1("^[0-9]{1,5}(?:\\.[0-9]{1,2})?$|^$");       // десятичное число 1-5 разряда или ничего

  switch( role )
  { case( Qt::EditRole ):
      switch( column )
      { case(1):
          str = value.toString();
          if( !rx1.exactMatch( str ) )
          { QMessageBox::warning( 0, "MBConfigWidget",
              "Узел должен быть десятичным числом.\n"
              "Подузел может задаватся в формате [узел].[подузел]" );
            break;
          }
          md[row].addr = str;
          goto ok;
        case(2):
          md[row].name = value.toString();
          goto ok;
        case(3):
          md[row].desc = value.toString();
          goto ok;
      }
      break;
  }
  return false;
ok:
  emit dataChanged( index, index );
  return true;
}

//===================================================================
//
//===================================================================
void ModulesModel::refresh()
{
  reset();
}

//###################################################################
//
//###################################################################
QVariant SlotsModel::data ( const QModelIndex & index, int role ) const
{
  if( current_module < 0 ) return QVariant();
  int row = index.row();
  int column = index.column();

  switch( role )
  { case( Qt::DisplayRole ):
    case( Qt::EditRole ):
      switch( column )
      { case (0): return QString::number( index.row() + 1 );
        case (1): return sd[current_module][row].addr;
        case (2):
          if(sd[current_module][row].addr.size())
             return QString::number(sd[current_module][row].n);
          break;
        case (3):
          if( sd[current_module][row].addr.size() )
          { return sd[current_module][row].type.toName();
          }
          break;
        case (4):
          if(sd[current_module][row].addr.size())
            return sd[current_module][row].desc;
          break;
      }
      break;
    case( Qt::TextAlignmentRole ):
      switch( column )
      { case (0): return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case (1): return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        case (2): return QVariant(Qt::AlignRight | Qt::AlignVCenter);
      }
      break;
    case( Qt::ToolTipRole):
      // прогнозирование следующего адреса
      if( row > 0 )
      { QString str;
        bool ok;
        str = sd[current_module][row-1].addr;
        int a   = str.toInt(&ok,16);
        int len = sd[current_module][row-1].n;
        if( !ok ) break;
        switch( sd[current_module][row-1].type.id() )
        { case( MBDataType::Bits  ):  len /= 8;  break;
          case( MBDataType::Bytes ):          ;  break;
          case( MBDataType::Words ):  len *= 2;  break;
          case( MBDataType::Dwords ):
          case( MBDataType::Floats ): len *= 4;  break;
          default: len=0;
        }
        str = QString::number( a + len, 16 ).toUpper();
        return "Расчетный адрес: "+ str;
      }
      break;
  }
  return QVariant();
}

//===================================================================
//
//===================================================================
QVariant SlotsModel::headerData ( int section, Qt::Orientation orientation,
                                   int role  ) const
{
  if( orientation != Qt::Horizontal ) return QVariant();
  switch( role )
  { case( Qt::DisplayRole ):
      switch( section )
      { case (0): return("N");
        case (1): return("Адрес");
        case (2): return("Кол-во");
        case (3): return("Тип");
        case (4): return("Описание");
      }
      break;
    case( Qt::SizeHintRole ):
      switch( section )
      { case (0): return QSize(20,20);
        case (1): return QSize(50,20);
        case (2): return QSize(50,20);
        case (3): return QSize(75,20);
        case (4): return QSize(100,20);
      }
      break;
  }
  return QVariant();
}

//===================================================================
//
//===================================================================
Qt::ItemFlags SlotsModel::flags ( const QModelIndex & index ) const
{
  if( current_module < 0 ) return 0;
  int column = index.column();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
  if( column == 1 ) flags |= Qt::ItemIsEditable;
  if( column == 2 ) flags |= Qt::ItemIsEditable;
  if( column == 4 ) flags |= Qt::ItemIsEditable;

  return flags;
}

//===================================================================
//
//===================================================================
bool SlotsModel::setData ( const QModelIndex & index,
                   const QVariant & value, int role )
{
  if( current_module < 0 ) return false;
  QString str;
  QRegExp rx1("^[0-9A-F]{1,4}$|^$"); // шестнадцатиричное число 1-4 разряда или ничего
  QRegExp rx2("^[0-9]{1,5}$");       // десятичное число 1-5 разряда или ничего

  int row = index.row();
  int column = index.column();

  switch( role )
  { case( Qt::EditRole ):
      switch( column )
      { case(1):
          str = value.toString().toUpper();
          if( !rx1.exactMatch( str ) )
          { QMessageBox::warning( 0, "MBConfigWidget", "Адрес должен быть шестнадцатиричное числом!" );
            break;
          }
          sd[current_module][row].addr = str;
          goto ok;
        case(2):
          if( !rx2.exactMatch( value.toString() ) )
          { QMessageBox::warning( 0, "MBConfigWidget", "Количество должен быть десятичным числом!" );
            break;
          }
          sd[current_module][row].n = value.toInt();
          goto ok;
        case(4):
          sd[current_module][row].desc = value.toString();
          goto ok;
      }
      break;
    case( Qt::UserRole ):
      switch( column )
      { case(3):
          sd[current_module][row].type.step();
          goto ok;
      }
      break;
  }
  return false;
ok:
  emit dataChanged( index, index );
  return true;
}

//===================================================================
//
//===================================================================
void SlotsModel::set_current_module( int i )
{
  current_module=i;
  reset();
}

