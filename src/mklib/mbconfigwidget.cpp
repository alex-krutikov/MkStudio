#include <QtGui>

#include "mbconfigwidget.h"
#include "ui_mbconfigwidget.h"

#include "mbcommon.h"

//###################################################################
//
//###################################################################
MBConfigWidget::MBConfigWidget( QWidget *parent )
  : QWidget( parent ), ui( new Ui::MBConfigWidget )
{
  int i,j;
  ui->setupUi( this );
  ui->splitter->setStretchFactor(0,10);
  ui->splitter->setStretchFactor(1,15);

  for( i=0; i<MODULES_MAX_N; i++ )
  { for( j=0; j<SLOTS_MAX_N; j++ )
    { slots_model.sd[i][j].n=0;
      slots_model.sd[i][j].type = MBDataType::Bits;
    }
  }
  slots_model.current_module=-1;

  ui->tw1->setContextMenuPolicy( Qt::CustomContextMenu );
  ui->tw1->setSelectionBehavior( QAbstractItemView::SelectRows );
  ui->tw1->setSelectionMode( QAbstractItemView::SingleSelection );
  ui->tw1->verticalHeader()->setDefaultSectionSize( ui->tw1->font().pointSize()+11 );
  ui->tw1->verticalHeader()->hide();
  ui->tw1->setModel( &modules_model );
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
  ui->tw2->setModel( &slots_model );
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
}

//===================================================================
//
//===================================================================
void MBConfigWidget::on_tw1_customContextMenuRequested ( const QPoint& pos)
{
  int i;

  QMenu menu;
  QAction *act_copy  = menu.addAction(QIcon(":/icons/res/copy.png"),  "����������" );
  QAction *act_paste = menu.addAction(QIcon(":/icons/res/paste.png"), "��������"   );
  QAction *act = menu.exec( ui->tw1->mapToGlobal( pos ) );
  if( act == 0 ) return;

  int r = ui->tw1->selectionModel()->selectedRows().value(0).row();
  if(( r < 0) || (r >= MODULES_MAX_N ) ) return;

  if( act == act_copy )
  { mbuffer = modules_model.md[ r ];
    for( i=0; i<SLOTS_MAX_N; i++ )
    { sbuffer[i]=slots_model.sd[r][i];
    }
  } else if( act == act_paste )
  { modules_model.md[ r ] = mbuffer;
    for( i=0; i<SLOTS_MAX_N; i++ )
    { slots_model.sd[r][i]=sbuffer[i];
    }
    modules_model.refresh();
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
  slots_model.set_current_module( current.row() );
  ui->tw2->clearSelection();
}

//===================================================================
//
//===================================================================
void MBConfigWidget::clearConfiguration()
{
  int i,j;

  for( i=0; i<MODULES_MAX_N; i++ )
  { modules_model.md[i].addr.clear();
    modules_model.md[i].name.clear();
    modules_model.md[i].desc.clear();
    for( j=0; j<SLOTS_MAX_N; j++ )
    { slots_model.sd[i][j].addr.clear();
      slots_model.sd[i][j].n=0;
      slots_model.sd[i][j].type = MBDataType::Bits;
      slots_model.sd[i][j].desc.clear();
      slots_model.sd[i][j].attributes.clear();
    }
  }
  slots_model.current_module=-1;

  modules_model.refresh();
  slots_model.set_current_module(0);
  ui->tw1->selectRow(0);
}

//===================================================================
//
//===================================================================
void MBConfigWidget::loadConfiguration( QDomDocument &doc )
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
    modules_model.md[n].addr = element.attribute("Node");
    modules_model.md[n].name = element.attribute("Name");
    modules_model.md[n].desc = element.attribute("Desc");
    slot_list = element.elementsByTagName("Slot");
    for(j=0; j<slot_list.count(); j++ )
    { element = slot_list.item( j ).toElement();
      k = element.attribute("N").toInt()-1;
      if( ( k >= SLOTS_MAX_N ) || ( k < 0 )) continue;
      slots_model.sd[n][k].addr = element.attribute("Addr");
      slots_model.sd[n][k].n    = element.attribute("Length").toInt();
      str = element.attribute("Type");
      slots_model.sd[n][k].type.fromTextId( str );
      str = element.attribute("Operation");
      slots_model.sd[n][k].desc       = element.attribute("Desc");
      slots_model.sd[n][k].attributes = element.attribute("Attributes");
    }
  }

  modules_model.refresh();
  slots_model.set_current_module(0);
  ui->tw1->selectRow(0);
}

//===================================================================
//
//===================================================================
void MBConfigWidget::loadConfiguration( QByteArray &xml )
{
  QDomDocument doc;
  doc.setContent( xml, false );
  loadConfiguration( doc );
}

//===================================================================
//
//===================================================================
void MBConfigWidget::saveConfiguration( QDomDocument &doc )
{
  int i,j;
  QString str;

  QDomElement root = doc.createElement("MBConfig");
  doc.appendChild(root);

  for(i=0; i<MODULES_MAX_N; i++ )
  {
    if( modules_model.md[i].addr.size()==0 ) continue;
    QDomElement module = doc.createElement("Module");
    module.setAttribute("N",i+1);
    module.setAttribute("Node", modules_model.md[i].addr );
    module.setAttribute("Name", modules_model.md[i].name );
    str = modules_model.md[i].desc;
    if( !str.isEmpty() )
    {  module.setAttribute("Desc", str );
    }
    root.appendChild(module);
    for(j=0; j<SLOTS_MAX_N; j++ )
    { if( slots_model.sd[i][j].addr.size() == 0 ) continue;
      QDomElement slot = doc.createElement("Slot");
      slot.setAttribute("N",j+1);
      slot.setAttribute("Addr",   slots_model.sd[i][j].addr );
      slot.setAttribute("Length", slots_model.sd[i][j].n    );
      str = slots_model.sd[i][j].type.toTextId();
      slot.setAttribute("Type", str );
      str = slots_model.sd[i][j].desc;
      if( !str.isEmpty() )
      { slot.setAttribute("Desc", str );
      }
      str = slots_model.sd[i][j].attributes;
      if( !str.isEmpty() )
      { slot.setAttribute("Attributes", str );
      }
      module.appendChild(slot);
    }
  }
}

//===================================================================
//
//===================================================================
void MBConfigWidget::setSlotAttributes(int module, int slot, const QString &attributes)
{
  if( ( module <= 0 ) || ( module > MODULES_MAX_N ) ) return;
  if( ( slot   <= 0 ) || ( slot   > SLOTS_MAX_N   ) ) return;
  module--;
  slot--;
  slots_model.sd[module][slot].attributes = attributes;
}

//===================================================================
//
//===================================================================
void MBConfigWidget::saveConfiguration( QByteArray &xml )
{
  QDomDocument doc;
  saveConfiguration( doc );
  xml = doc.toByteArray( 2 );
}

//===================================================================
//
//===================================================================
QString MBConfigWidget::exportConfiguration()
{
  int i,j,maxlen,a;
  QString str,s;

  for(i=0; i<MODULES_MAX_N; i++ )
  {
    if( modules_model.md[i].addr.size()==0 ) continue;

    str += "������  : " + modules_model.md[i].name + "\n";
    str += "����    : " + modules_model.md[i].addr + "\n";
    s = modules_model.md[i].desc;
    if( !s.isEmpty() )  str += "��������: " + s + "\n";

    maxlen=0;
    for(j=0; j<SLOTS_MAX_N; j++ )
    { if( slots_model.sd[i][j].addr.size() == 0 ) continue;
      a = slots_model.sd[i][j].desc.length();
      if( a > maxlen ) maxlen=a;
    }

    str +=  QString("��������").leftJustified(maxlen, ' ') + "\t";
    str +=  QString("�����").leftJustified(8, ' ') + "\t";;
    str +=  QString("���").leftJustified(8, ' ') + "\t";;
    str +=  QString("���-��").leftJustified(8, ' ') + "\n";

    for(j=0; j<SLOTS_MAX_N; j++ )
    { if( slots_model.sd[i][j].addr.size() == 0 ) continue;

      str +=  slots_model.sd[i][j].desc.leftJustified(maxlen, ' ') + "\t";
      str +=  slots_model.sd[i][j].addr.leftJustified(8, ' ') + "\t";
      str +=  slots_model.sd[i][j].type.toName().leftJustified(8, ' ') + "\t";
      str +=  QString::number(slots_model.sd[i][j].n).leftJustified(8, ' ');
      str +=  "\n";
    }
    str +=  "\n";
  }
  return str;
}

//###################################################################
//
//###################################################################
QVariant MBConfigWidget::ModulesModel::data ( const QModelIndex & index, int role ) const
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
QVariant MBConfigWidget::ModulesModel::headerData ( int section, Qt::Orientation orientation,
                                   int role  ) const
{
  if( orientation != Qt::Horizontal ) return QVariant();
  switch( role )
  { case( Qt::DisplayRole ):
      switch( section )
      { case (0): return("N");
        case (1): return("����");
        case (2): return("��������");
        case (3): return("��������");
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
Qt::ItemFlags MBConfigWidget::ModulesModel::flags ( const QModelIndex & index ) const
{
  int column = index.column();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
  if( column > 0 ) flags |= Qt::ItemIsEditable;

  return flags;
}

//===================================================================
//
//===================================================================
bool MBConfigWidget::ModulesModel::setData ( const QModelIndex & index,
                   const QVariant & value, int role )
{
  int row = index.row();
  int column = index.column();

  QString str;
  QRegExp rx1("^[0-9]{1,5}(?:\\.[0-9]{1,2})?$|^$");       // ���������� ����� 1-5 ������� ��� ������

  switch( role )
  { case( Qt::EditRole ):
      switch( column )
      { case(1):
          str = value.toString();
          if( !rx1.exactMatch( str ) )
          { QMessageBox::warning( 0, "MBConfigWidget",
              "���� ������ ���� ���������� ������.\n"
              "������� ����� ��������� � ������� [����].[�������]" );
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
void MBConfigWidget::ModulesModel::refresh()
{
  reset();
}

//###################################################################
//
//###################################################################
QVariant MBConfigWidget::SlotsModel::data ( const QModelIndex & index, int role ) const
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
      // ��������������� ���������� ������
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
        return "��������� �����: "+ str;
      }
      break;
  }
  return QVariant();
}

//===================================================================
//
//===================================================================
QVariant MBConfigWidget::SlotsModel::headerData ( int section, Qt::Orientation orientation,
                                   int role  ) const
{
  if( orientation != Qt::Horizontal ) return QVariant();
  switch( role )
  { case( Qt::DisplayRole ):
      switch( section )
      { case (0): return("N");
        case (1): return("�����");
        case (2): return("���-��");
        case (3): return("���");
        case (4): return("��������");
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
Qt::ItemFlags MBConfigWidget::SlotsModel::flags ( const QModelIndex & index ) const
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
bool MBConfigWidget::SlotsModel::setData ( const QModelIndex & index,
                   const QVariant & value, int role )
{
  if( current_module < 0 ) return false;
  QString str;
  QRegExp rx1("^[0-9A-F]{1,4}$|^$"); // ����������������� ����� 1-4 ������� ��� ������
  QRegExp rx2("^[0-9]{1,5}$");       // ���������� ����� 1-5 ������� ��� ������

  int row = index.row();
  int column = index.column();

  switch( role )
  { case( Qt::EditRole ):
      switch( column )
      { case(1):
          str = value.toString().toUpper();
          if( !rx1.exactMatch( str ) )
          { QMessageBox::warning( 0, "MBConfigWidget", "����� ������ ���� ����������������� ������!" );
            break;
          }
          sd[current_module][row].addr = str;
          goto ok;
        case(2):
          if( !rx2.exactMatch( value.toString() ) )
          { QMessageBox::warning( 0, "MBConfigWidget", "���������� ������ ���� ���������� ������!" );
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
void MBConfigWidget::SlotsModel::set_current_module( int i )
{
  current_module=i;
  reset();
}

