#include <QtGui>
#include <QtXml>

#include "mktable.h"
#include "mktable_p.h"
#include "plot.h"
#include "mbmasterxml.h"

//-----------------------------------------------------------------------------
// служебная структура - привязка ячеек
//-----------------------------------------------------------------------------
struct MKTable::MKTableAssignData
{ int m_index,s_index,i_index; // модуль,слот,индекс
  int row,column;              // ячейка
  MMValue current_value;       // текущее значение
  bool first_pass_flag;        // флаг первого прохода отображения
  int status;                  // достоверность значения
  int format;                  // формат выдачи
  int ss_enum_index;           // индекс в векторе enum
  static bool lessThan(const MKTableAssignData &s1, const MKTableAssignData &s2);
};

//##############################################################################
//
//##############################################################################
MKTable::MKTable( QWidget *parent )
  : QTableWidget( parent ),
    delegate( new MKTableItemDelegate( this ) )
{
  Q_INIT_RESOURCE( mklib );

  mbmaster = 0;
  mode = MKTable::Edit;

  setMouseTracking( true );

  setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  verticalHeader()->hide();
  horizontalHeader()->setClickable( false );
  verticalHeader()  ->setClickable( false );

  connect(&timer, SIGNAL(timeout()), this, SLOT(refresh()));

  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  
  optimize_read_mode = true;
  plots_count = 0;
}

//==============================================================================
//
//==============================================================================
MKTable::~MKTable()
{
}

//==============================================================================
//
//==============================================================================
void MKTable::setOptimizeReadMode( bool optimizeRead )
{
  optimize_read_mode = optimizeRead;
}

//==============================================================================
//
//==============================================================================
bool MKTable::optimizeReadMode()
{
  return optimize_read_mode;
}

//==============================================================================
//
//==============================================================================
QSize MKTable::sizeHint() const
{
  return QSize(
         horizontalHeader()->length()
        +70 + verticalHeader()->width(),

        verticalHeader()->length()
        +70 + horizontalHeader()->height() );
}
//==============================================================================
//
//==============================================================================
void MKTable::closeAllRecorders()
{
  emit signalCloseAllRecorders();
  plots_count = 0;
  slotMinimize( false );
}

//==============================================================================
//
//==============================================================================
QStringList MKTable::getHorizontalHeaderLabels()
{
  int i;
  QStringList sl;

  for( i=0; i<columnCount(); i++ )
  { sl << model()->headerData( i, Qt::Horizontal ).toString();
  }
  return sl;
}

//==============================================================================
//
//==============================================================================
bool MKTable::loadConfiguration( const QDomDocument &doc )
{
 // int row_span,col_span;
 int i,row,col;
 QTableWidgetItem *titem;
 QString str;
 QString confirm;
 QFont fnt;
 QDomElement element;
 QColor color;

 QDomElement root = doc.documentElement();
 clear();
 setColumnCount( root.attribute( "ColumnCount" ).toInt() );
 setRowCount(    root.attribute( "RowCount" ).toInt() );
 setHorizontalHeaderLabels( root.attribute( "Labels" ).split(";") );
 QStringList sl = root.attribute( "LabelsSize" ).split(";");
 for( i=0; i<sl.count(); i++ )
 { horizontalHeader()->resizeSection(i, sl.at(i).toInt() );
 }
 horizontalHeader()->hide();
 horizontalHeader()->show();
 QDomNodeList items_list = root.elementsByTagName("Item");
 for( i=0; i<items_list.count(); i++ )
 { element = items_list.item(i).toElement();
   titem = new QTableWidgetItem;
   row   = element.attribute("Row").toInt();
   col   = element.attribute("Column").toInt();
   titem->setText( element.attribute("Text") );
   str = element.attribute("Font");
   fnt = font();
   if( str.contains("bold") )      fnt.setBold( true );
   if( str.contains("underline") ) fnt.setUnderline( true );
   if( str.contains("italic") )    fnt.setItalic( true );
   titem->setFont( fnt );
   if( str.contains("align_left")   ) titem->setTextAlignment(Qt::AlignVCenter|Qt::AlignLeft);
   if( str.contains("align_center") ) titem->setTextAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
   if( str.contains("align_right")  ) titem->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
   titem->setData( AssignRole, element.attribute("Assign") );
   titem->setData( FormatRole, element.attribute("Format") );
   str = element.attribute("BackgroundColor");
   if( !str.isEmpty() )
   { color.setNamedColor( str );
     if( color.isValid() )
     { titem->setData( Qt::BackgroundRole, color );
     }
   }
   str = element.attribute("ForegroundColor");
   if( !str.isEmpty() )
   { color.setNamedColor( str );
     if( color.isValid() )
     { titem->setData( Qt::ForegroundRole, color );
     }
   }
   str = element.attribute("SSSelector");
   if( !str.isEmpty() )
   { titem->setData( SSSelectorRole, str );
   }
   confirm = element.attribute("SSConfirmEdit");
   titem->setData( SSConfirmEditRole, confirm );

   str = element.attribute("RecorderParams");
   if( !str.isEmpty() ) titem->setData( RecorderParamsRole, str );
   setItem( row,col,titem );
/*
   // НЕПОНЯТКИ С ОБЪЕДИНЕНИЕМ ЯЧЕЕК
   str = element.attribute("Span");
   row_span=str.section(',',0,0).toInt();
   col_span=str.section(',',1,1).toInt();
   if((rowSpan(row,col)==1)&&(columnSpan(row,col)==1)&&(( row_span > 1 )||( col_span > 1 ))) setSpan(row,col,row_span,col_span);
*/
 }

 QDomNodeList script_list = root.elementsByTagName("SettingsSheet");
 if( script_list.count() )
 { setSettingsSheet( script_list.item(0).toElement().text() );
 } else
 { setSettingsSheet( QString() );
 }

 setMode( MKTable::Edit );
 return true;
}

//==============================================================================
//
//==============================================================================
void MKTable::saveConfiguration( QDomDocument &doc )
{
 int i,j;
 QStringList sl = getHorizontalHeaderLabels();
 QString str;
 bool confirm;
 QTableWidgetItem *titem;
 QVariant var;
 QColor color_white( "white" );
 QColor color_black( "black" );

 for( i=0; i<columnCount(); i++ )
 { str += QString("%1;").arg(horizontalHeader()->sectionSize(i));
 }
 str.chop(1);

 QDomElement tag_table = doc.createElement("Table");
 tag_table.setAttribute("ColumnCount", columnCount() );
 tag_table.setAttribute("RowCount",    rowCount() );
 tag_table.setAttribute("Labels",      sl.join(";") );
 tag_table.setAttribute("LabelsSize",  str );
 doc.appendChild(tag_table);

 QDomElement tag_item;
 for( i=0; i<rowCount(); i++ )
 { for( j=0; j<columnCount(); j++ )
   { titem = item(i,j);
     if( titem == 0 ) continue;
     tag_item = doc.createElement("Item");
     tag_item.setAttribute("Row",    QString::number(i) );
     tag_item.setAttribute("Column", QString::number(j) );
     str.clear();
     if( titem->font().bold() )      str += "bold;";
     if( titem->font().italic() )    str += "italic;";
     if( titem->font().underline() ) str += "underline;";
     if( titem->textAlignment() & Qt::AlignHCenter ) str += "align_center;";
     if( titem->textAlignment() & Qt::AlignRight ) str += "align_right;";
     str.chop(1);
     if( !str.isEmpty() ) tag_item.setAttribute("Font",   str );
     str = titem->text();
     if( !str.isEmpty() ) tag_item.setAttribute("Text", str );
     str = titem->data(AssignRole).toString();
     if( !str.isEmpty() ) tag_item.setAttribute("Assign", str );
     str = titem->data(FormatRole).toString();
     if( !str.isEmpty() ) tag_item.setAttribute("Format", str );
     var = titem->data(Qt::BackgroundRole);
     if( var.isValid()  )
     { if( var.value<QColor>() != color_white )
       { tag_item.setAttribute("BackgroundColor", var.toString() );
       }
     }
     var = titem->data(Qt::ForegroundRole);
     if( var.isValid()  )
     { if( var.value<QColor>() != color_black )
       { tag_item.setAttribute("ForegroundColor", var.toString() );
       }
     }
     str = titem->data(SSSelectorRole).toString();
     if( !str.isEmpty() ) tag_item.setAttribute("SSSelector", str );
     confirm = titem->data(SSConfirmEditRole).toBool();
     if( confirm ) tag_item.setAttribute("SSConfirmEdit", confirm );
     str = titem->data(RecorderParamsRole).toString();
     if( !str.isEmpty() ) tag_item.setAttribute("RecorderParams", str );
     // tag_item.setAttribute("Span",  QString("%1,%2").arg(rowSpan(i,j)).arg(columnSpan(i,j)));
     tag_table.appendChild(tag_item);
   }
 }
 if( !settingssheet_str.isEmpty() )
 { QDomElement tag_ss = doc.createElement("SettingsSheet");
   tag_ss.appendChild( doc.createTextNode( settingsSheet() ) );
   tag_table.appendChild(tag_ss);
 }
}

//==============================================================================
//
//==============================================================================
void MKTable::setSettingsSheet( const QString &ss)
{
  settingssheet_str=ss;
}

//==============================================================================
//
//==============================================================================
QString MKTable::settingsSheet()
{
  return settingssheet_str;
}

//==============================================================================
//
//==============================================================================
bool MKTable::MKTableAssignData::lessThan(const MKTableAssignData &s1, const MKTableAssignData &s2)
{
  bool b = false;
  if     ( s1.m_index  < s2.m_index)     b = true;
  else if( s1.m_index == s2.m_index)
  { if     ( s1.s_index  < s2.s_index)   b = true;
    else if( s1.s_index == s2.s_index)
    { if     ( s1.i_index  < s2.i_index) b = true;
    }
  }
  return b;
};

//==============================================================================
/// Разбор листа настроек
//==============================================================================
void MKTable::ss_process()
{
  ss_enum.clear();
  ss_enum_name2index.clear();
  QString ss = settingssheet_str;
  QString str,str2;
  // удаление комментариев
  ss.remove( QRegExp("//[^\n]*") );
  ss = ss.simplified();

  QStringList sl_enum_name;  // названия селекторов
  QStringList sl_enum_body;  // значения селекторов

  // выделение селекторов из общего текста
  QRegExp rx = QRegExp("\\benum\\b\\s*(\\w+)\\b\\s*\\{");
  int pos,p1,i,j,k;
  pos=0;
  while( (pos = rx.indexIn(ss, pos)) != -1 )
  { pos += rx.matchedLength();
    sl_enum_name << rx.cap(1);
    p1 = pos;
    i  = 1;
    while( pos < ss.length() )
    { if( ss[pos] == '{' ) i++;
      if( ss[pos] == '}' ) i--;
      if( i == 0 ) break;
      pos++;
    }
    sl_enum_body << ss.mid( p1, pos-p1 );
  }

  // разбор селекторов
  MKTable_SS_Enum t;
  QRegExp rx2 = QRegExp("=(-?\\d+):");
  QVector<int> v_flags;
  QVector<int> v_n;

  for( i=0; i<sl_enum_name.count(); i++ )
  {
    //Console::Print( "########\n" + sl_enum_name.at(i) + "|" + sl_enum_body.at(i) + "\n########\n");
    t.map_display.clear();
    t.map_menu.clear();
    str = sl_enum_body.at(i);

    // этап 1
    v_flags.resize( str.length() );
    v_flags.fill(0);
    v_n.clear();
    pos=0;
    while( (pos = rx2.indexIn(str, pos)) != -1 )
    { j = rx2.matchedLength();
      while(j--)
      { v_flags[pos++]=1;
      }
      v_n << rx2.cap(1).toInt();
    }
    //qDebug() << v_n;
    //qDebug() << v_flags;

    // этап 2
    k=0;
    for( pos=0; pos<v_flags.size(); pos++ )
    { if( v_flags[pos] == 0 ) continue;
      for( j=pos; j<v_flags.size(); j++ )
      { if( v_flags[j] == 0 ) break;
      }
      p1=j;
      for( ; j<v_flags.size(); j++ )
      { if( v_flags[j] != 0 ) break;
      }
      str2 = str.mid( p1, j-p1 ).simplified();
      MapEmul me;
      me.key = v_n[k];
      me.value = str2;
      if( str2.startsWith(":") )
      { str2.remove(0,1);
        str2 = str2.simplified();
        me.value = str2;
        t.map_display.append( me );
      } else
      { t.map_display.append( me );
        t.map_menu.append( me );
      }
      //qDebug() << v_n[k] << p1<<j << str2;
      pos=j;
      k++;
    }

    ss_enum << t;
    ss_enum_name2index.insert( sl_enum_name.at(i), i );
  }
}

//==============================================================================
//
//==============================================================================
void MKTable::setMode( enum Mode m )
{
  static QRegExp rx("^(\\d+)/(\\d+)/(\\d+)$");
  int i,j;
  QTableWidgetItem *titem;
  QString str;
  MKTableAssignData ad;
  QStringList unknown_selectors;

  mode=m;

  if( m == MKTable::Edit )
  { clearSelection();
    unsetCursor();
    timer.stop();
    for( i=0; i<rowCount();i++ )
    { for( j=0; j<columnCount(); j++ )
      { titem = item(i,j);
        if( titem == 0 )
        { titem = new QTableWidgetItem;
          setItem( i,j,titem);
        }
        str = titem->data(AssignRole).toString();
        titem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable);
        if( str.isEmpty() ) continue;
        rx.indexIn( str );
        if( !titem->data(Qt::BackgroundRole).isValid() )
        { titem->setBackgroundColor("antiquewhite");
        }
        titem->setText( str );
        titem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
      }
    }
    QItemDelegate *itemdelegate = new QItemDelegate( this );
    connect( itemdelegate, SIGNAL( closeEditor ( QWidget*, QAbstractItemDelegate::EndEditHint ) ),
             this,         SIGNAL( item_changed_by_editor() ) );
    setItemDelegate( itemdelegate );
  }

  if( m == MKTable::Polling )
  { assign_data.clear();
    ss_process();
    clearSelection();
    unsetCursor();
    for( i=0; i<rowCount();i++ )
    { for( j=0; j<columnCount(); j++ )
      { titem = item(i,j);
        if( titem == 0 )
        { titem = new QTableWidgetItem;
          setItem( i,j,titem);
        }
        titem->setFlags(Qt::ItemIsEnabled);
        //---------------
        str = titem->data(AssignRole).toString();
        if( str.isEmpty() ) continue;
        if( !rx.exactMatch( str ) ) continue;
        titem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsSelectable);
        rx.indexIn( titem->data(AssignRole).toString() );
        ad.m_index = rx.cap(1).toInt();
        ad.s_index = rx.cap(2).toInt();
        ad.i_index = rx.cap(3).toInt()-1;
        ad.row = i;
        ad.column = j;
        ad.status = -1;
        ad.first_pass_flag = true;
        //---------------
        str = titem->data(FormatRole).toString();
        ad.format = 0;
        if( str.contains("unsigned") ) ad.format = 1;
        if( str.contains("hex") )      ad.format = 2;
        if( str.contains("bin") )      ad.format = 3;
        //---------------
        str = titem->data(SSSelectorRole).toString();
        ad.ss_enum_index = -1;
        if( !str.isEmpty() )
        { if( ss_enum_name2index.contains( str ) )
          { ad.ss_enum_index = ss_enum_name2index.value( str );
          } else
          { unknown_selectors << "\"" + str + "\" <" +titem->data(AssignRole).toString() + ">";
          }
        }
        ad.ss_enum_index = ss_enum_name2index.value(
                             titem->data(SSSelectorRole).toString() ,-1 );
        //---------------
        assign_data << ad;
      }
    }
    qSort( assign_data.begin(),assign_data.end(), MKTableAssignData::lessThan );
    for( i=0; i<assign_data.size(); i++ )
    { item(assign_data[i].row, assign_data[i].column)->setData(IndexRole, i );
    }

    setItemDelegate( delegate );
    timer.start(100);
    refresh();

    if( unknown_selectors.size() )
    { QMessageBox::warning( this, "Предупреждение",
          "Некоторые используемые в таблице селекторы не найдены в листе настроек!\n\n"
          "Список селекторов и ячеек:\n"
        + unknown_selectors.join(",\n") );
    }
  }
  reset();
}

//==============================================================================
//
//==============================================================================
void MKTable::mouseMoveEvent( QMouseEvent *event )
{
  QTableWidget::mouseMoveEvent( event );
  if( mode != MKTable::Polling ) return;

  QTableWidgetItem *titem = itemAt( event->pos() );
  if( titem )
  { if( titem->data(IndexRole).isValid() )
    { setCursor( Qt::PointingHandCursor );
      return;
    }
  }
  unsetCursor();
}

//==============================================================================
//
//==============================================================================
void MKTable::contextMenuEvent( QContextMenuEvent *e )
{
  if( mode != Polling ) return;

  QList<QTableWidgetItem *> si = QTableWidget::selectedItems();
  if( si.count() == 0 ) return;
  if( si.count() >  4 )
  { QMessageBox::information( this,"Ошибка", "Должно быть выделено не более 4 ячеек." );
    return;
  }

  QMenu menu( this );
  QAction *action1=0;

  action1 = menu.addAction(QIcon(":/icons/res/line-chart.png"),"Самописец");
  if( !si.value(0)->data(IndexRole).isValid() ) action1->setEnabled( false );

  QAction *act  = menu.exec( viewport()->mapToGlobal( e->pos() ) );
  if( act == 0 ) return;
  if( act == action1 )
  { //---------- вывод графика ------------------------------------------------------------------------

    QString str, str_column, str_row, str_title;
    int i,j,si_count;
    si_count=0;
    str_title = "Самописец";
    foreach( QTableWidgetItem * t, si )
    {
      //-------------------- название столбца --------
      j = t->column();
      for( i = t->row(); i>=0; i-- )
      { if( item(i,j)->data(IndexRole).isValid() ) continue;
        if( item(i,j)->text().isEmpty() ) continue;
        str_column = item(i,j)->text();
        break;
      }
      if( str_column.isEmpty() )  { str_column = model()->headerData( j, Qt::Horizontal ).toString();  }
      //-------------------- название строки --------
      i = t->row();
      for( j = t->column(); j>=0; j-- )
      { if( item(i,j)->data(IndexRole).isValid() ) continue;
        if( item(i,j)->text().isEmpty() ) continue;
        str_row = item(i,j)->text();
        break;
      }
      //------добавление составляющей цвета графика----
      switch(si_count)
      { case 0: str += "синий самописец:     ";break;
        case 1: str += "зеленый самописец:";break;
        case 2: str += "красный самописец:";break;
        case 3: str += "черный самописец:  ";break;
      }
        si_count++;
      //-------------------- вывод графика ----------
      if( !str_row.isEmpty() )    str += " ( " + str_row.simplified() + " )";
      if( !str_column.isEmpty() ) str += " ( " + str_column.simplified() + " )\n";
      //-------------------- определение заголовка ----------
      if( si.count() == 1 )
      { if( !str_row.isEmpty() )    str_title += " ( " + str_row.simplified() + " )";
        if( !str_column.isEmpty() ) str_title += " ( " + str_column.simplified() + " )";
      }
    }
    str.remove(str.length()-1,1);
    bool is_minimize = ( size() == QSize(0,0) ) ? true : false;
    Plot *plot = new Plot( str_title, si, is_minimize, this, mbmaster );//si.value(0)->data(AssignRole).toString() );
    if( si.count() > 1 ) plot->setPlotList( str );

    slotPlotOpen();

    connect( plot, SIGNAL(signalPlotClose()),     this, SLOT(slotPlotClose()) );
    connect( plot, SIGNAL(signalPlotOpen()),      this, SLOT(slotPlotOpen() ) );
    connect( plot, SIGNAL(signalMinimize(bool)), this, SLOT(slotMinimize(bool)) );
    connect( this, SIGNAL( signalCloseAllRecorders()), plot, SLOT( close() ));
    connect( this, SIGNAL( signalMinimizeStateChange( bool ) ),
             plot, SLOT  ( updateMinimizeButtonState( bool ) ) );

    plot->show();
  }
}
//==============================================================================
//
//==============================================================================
void MKTable::mousePressEvent( QMouseEvent *event )
{
  QTableWidget::mousePressEvent( event );
}
//==============================================================================
//
//==============================================================================
void MKTable::mouseReleaseEvent( QMouseEvent *event )
{
  if( mode == MKTable::Polling )
  { if( event->button() == Qt::LeftButton )
    { QTableWidgetItem *titem = itemAt( event->pos() );
      if( titem )
      { currentItemConfirmEdit = titem->data( MKTable::SSConfirmEditRole ).toBool();
        editItem( titem );
      }
    }
  }
  QTableWidget::mouseReleaseEvent( event );
}

//==============================================================================
//
//==============================================================================
void MKTable::mouseDoubleClickEvent( QMouseEvent *event )
{
  if( mode == MKTable::Polling ) return;
  QTableWidget::mouseDoubleClickEvent( event );
}

//==============================================================================
//
//==============================================================================
void MKTable::keyPressEvent( QKeyEvent *event )
{
  int mm,ss,ii,i,val;
  bool ok;

  if( mode == MKTable::Edit )
  { QTableWidget::keyPressEvent( event );
    return;
  }

  int key = event->key();
  if( ( key == Qt::Key_Return ) || ( key == Qt::Key_Enter ) )
  { editItem( currentItem() );
  }
  else if( key == Qt::Key_Space )
  { val = ( event->modifiers() & Qt::ControlModifier ) ? 1:0;
    QList<QTableWidgetItem*>  si = selectedItems();
    foreach( QTableWidgetItem *titem, si )
    { i = titem->data(IndexRole).toInt(&ok);
      if( !ok ) continue;
      if( i >= assign_data.size() ) continue;
      mm = assign_data[i].m_index;
      ss = assign_data[i].s_index;
      ii = assign_data[i].i_index;
      mbmaster->setSlotValue(mm,ss,ii, MMValue::fromInt(val) );
    }
  }else
  { QTableWidget::keyPressEvent( event );
  }
}

//==============================================================================
//! обновление содержимого таблицы (собственно опрос)
//==============================================================================
void MKTable::refresh()
{
  QString str;
  int mm,ss,ii;
  MMValue value;
  MMSlot  slot;
  int i,j;
  int slot_size;
  QTableWidgetItem *titem;
  bool vflag;

  if( mbmaster == 0 ) return;
  if( mode != MKTable::Polling ) return;

  QRect vr =  viewport()->rect();

  int assign_data_size = assign_data.size();

  for( i=0; i<assign_data_size; i++ )
  {
cycle_begin:
    mm  = assign_data[i].m_index;
    ss  = assign_data[i].s_index;

    if( optimize_read_mode )
    { vflag = false;
      for(j=i; (j<assign_data_size) && (assign_data[j].m_index==mm) && (assign_data[j].s_index==ss); j++ )
      { titem = item( assign_data[j].row, assign_data[j].column );
        if( titem == 0 ) continue;
        QRect r = visualItemRect( titem );
        if( vr.intersects(r) )
        { vflag = true;
          break;
        }
      }
    } else
    { vflag = true;
    }

    if( vflag )
    { slot = mbmaster->getSlot(mm,ss); // запрос данных
    } else
    { slot = MMSlot();
    }

    slot_size = slot.data.size();
    for( ; i<assign_data_size ; i++)
    { if( assign_data[i].m_index != mm ) goto cycle_begin;
      if( assign_data[i].s_index != ss ) goto cycle_begin;
      ii = assign_data[i].i_index;
      if( ii < slot_size )
      { value = slot.data[ii];
      } else
      { value = MMValue();
      }

      titem = item( assign_data[i].row, assign_data[i].column );
      if( titem == 0 ) continue;

      if( assign_data[i].first_pass_flag || (value != assign_data[i].current_value ) )
      { // обновление
        assign_data[i].first_pass_flag = false;
        if( value.status() == MMValue::Ok )
        { if( assign_data[i].status != 1 )
          { assign_data[i].status = 1;
            titem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
          }
          if( assign_data[i].ss_enum_index < 0 )
          { str = format_output( value, slot.datatype, assign_data[i].format );
          } else
          { QVector<MapEmul> &m = ss_enum[assign_data[i].ss_enum_index].map_display;
            MapEmul me;
            QString strValue;
            foreach(me, m)
            {
              if( me.key == value.toInt()
                  || me.key == value.toUInt() )
              {
                strValue = me.value;
                break;
              }
            }
            if( strValue.isEmpty() ) str.clear();
            else str = strValue;
          }
          titem->setText( str );
        } else if( value.status() == MMValue::NotInit )
        { if( assign_data[i].status != 2 )
          { assign_data[i].status = 2;
            titem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
            titem->setText( QString() );
          }
        } else
        { if( assign_data[i].status != 3 )
          { assign_data[i].status = 3;
            titem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
            titem->setText( "***" );
          }
        }
        assign_data[i].current_value = value;
      }
    }
  }
}

//==============================================================================
//
//==============================================================================
QString MKTable::format_output( const MMValue &value, const MBDataType &datatype, int type )
{
  QString str;
  char *cf;
  int datatype_id = datatype.id();
  char bs[]="0b00000000000000000000000000000000";
  unsigned int i,j,mask,k;

  int d = value.toInt();

  cf = (char*)"";

  switch( type )
  { case(1): // беззнаковый
      switch( datatype_id )
      { case( MBDataType::Floats )          : cf=(char*)"%7.7g";           break;
        case( MBDataType::Bits   )          : cf=(char*)"%u"; d &= 0x01;   break;
        case( MBDataType::Bytes  )          : cf=(char*)"%u"; d &= 0xFF;   break;
        case( MBDataType::Words  )          : cf=(char*)"%u"; d &= 0xFFFF; break;
        case( MBDataType::Dwords )          : cf=(char*)"%u";              break;
        case( MBDataType::InputRegisters )  : cf=(char*)"%u"; d &= 0xFFFF; break;
        case( MBDataType::HoldingRegisters ): cf=(char*)"%u"; d &= 0xFFFF; break;
        case( MBDataType::DiscreteInputs   ): cf=(char*)"%u"; d &= 0x01;   break;
        case( MBDataType::Coils)            : cf=(char*)"%u"; d &= 0x01;   break;
      }
      break;
    case(2): // hex
      switch( datatype_id )
      { case( MBDataType::Floats )          : cf=(char*)"%7.7g"; break;
        case( MBDataType::Bits   )          : cf=(char*)"0x%2.2X"; d &= 0x01;   break;
        case( MBDataType::Bytes  )          : cf=(char*)"0x%2.2X"; d &= 0xFF;   break;
        case( MBDataType::Words  )          : cf=(char*)"0x%4.4X"; d &= 0xFFFF; break;
        case( MBDataType::Dwords )          : cf=(char*)"0x%8.8X";              break;
        case( MBDataType::InputRegisters )  : cf=(char*)"0x%4.4X"; d &= 0xFFFF; break;
        case( MBDataType::HoldingRegisters ): cf=(char*)"0x%4.4X"; d &= 0xFFFF; break;
        case( MBDataType::DiscreteInputs   ): cf=(char*)"0x%2.2X"; d &= 0x01;   break;
        case( MBDataType::Coils   )         : cf=(char*)"0x%2.2X"; d &= 0x01;   break;
      }
      break;
    default: // формат по умолчанию
      switch( datatype_id )
      { case( MBDataType::Floats )          : cf=(char*)"%7.7g"; break;
        case( MBDataType::Bits   )          : cf=(char*)"%d"; break;
        case( MBDataType::Bytes  )          : cf=(char*)"%d"; break;
        case( MBDataType::Words  )          : cf=(char*)"%d"; break;
        case( MBDataType::Dwords )          : cf=(char*)"%d"; break;
        case( MBDataType::InputRegisters )  : cf=(char*)"%d"; break;
        case( MBDataType::HoldingRegisters ): cf=(char*)"%d"; break;
        case( MBDataType::DiscreteInputs   ): cf=(char*)"%d"; break;
        case( MBDataType::Coils   )         : cf=(char*)"%d"; break;
      }
      break;
  }

  mask=0; i=0;
  if( type == 3 )
  {   switch( datatype_id )
      { case( MBDataType::Floats ): goto next; break;
        case( MBDataType::Bits   ): goto next; break;
        case( MBDataType::Bytes  ): mask=1<<7; i=8; break;
        case( MBDataType::Words  ): mask=1<<15;i=16; break;
        case( MBDataType::Dwords ): mask=1<<31;i=32; break;
        case( MBDataType::DiscreteInputs  ): goto next; break;
        case( MBDataType::Coils ): goto next; break;
      }
      j = value.toInt();
      for(k=0;k<i;k++)
      { bs[2+k] = (j&mask) ? '1' : '0';
        mask >>= 1;
      }
      return QString::fromAscii(bs,i+2);
  }

next:
  switch( datatype_id )
  { case( MBDataType::Floats ):
      str.sprintf(cf, value.toDouble() );
      break;
    case( MBDataType::Bits            )  :
    case( MBDataType::Bytes           )  :
    case( MBDataType::Words           )  :
    case( MBDataType::Dwords          )  :
    case( MBDataType::InputRegisters )   :
    case( MBDataType::HoldingRegisters ) :
    case( MBDataType::DiscreteInputs )   :
    case( MBDataType::Coils )            :
      str.sprintf(cf, d  );
      break;
  }
  return str;
}
//==============================================================================
//
//==============================================================================
void MKTable::slotMinimize( bool to_minimize )
{
  if( to_minimize )
  { setFixedSize(0,0);
  } else
  { setMinimumSize(0,0);
    setMaximumSize(16777215,16777215);
  }

  if( plots_count ) emit signalMinimizeStateChange(  to_minimize );
}

//==============================================================================
//
//==============================================================================
void MKTable::slotPlotClose()
{
  plots_count--;
  if( !plots_count )
  { slotMinimize( false );
    emit signalMinimizeAllHide( false );
  }
}
//==============================================================================
//
//==============================================================================
void MKTable::slotPlotOpen()
{ plots_count++;
}
//##############################################################################
//
//##############################################################################
MKTableItemDelegate::MKTableItemDelegate( MKTable *parent )
  : QItemDelegate( parent )
{
  table = parent;
}

//==============================================================================
//
//==============================================================================
void MKTableItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option,
                                  const QModelIndex & index ) const
{
  QItemDelegate::paint( painter, option, index );
}

//==============================================================================
//
//==============================================================================
QWidget* MKTableItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                            const QModelIndex &index ) const
{
  Q_UNUSED( option );
  int i;
  if( table->mode == MKTable::Polling )
  { i = index.data( MKTable::IndexRole ).toInt();
    i = table->assign_data[i].ss_enum_index;
    if( i >= 0 )
    {
      MKTableItemCBWidget *w = new MKTableItemCBWidget( parent );
      w->setConfirmEdit( table->getCurrentItemConfirmEdit() );
      connect( w, SIGNAL( editingFinished() ), this, SLOT( commitAndCloseEditor() ) );

      MKTable::MKTable_SS_Enum &ss_enum = table->ss_enum[i];
      QList<int> keys;
      MKTable::MapEmul me;
      foreach( me, ss_enum.map_menu)
      {
        keys << me.key;
      }
      QString current_text = index.data( Qt::DisplayRole ).toString();
      QString str;
      foreach(i,keys)
      {
        foreach(me, ss_enum.map_menu)
        {
          if( me.key == i )
          {
            str = me.value;
            break;
          }
        }
        QListWidgetItem *litem =  new QListWidgetItem;
        litem->setText( str );
        litem->setData( Qt::UserRole, i );
        w->addItem( litem );
        if( current_text == str ) w->setCurrentItem( litem );
      }
      w->setStyleSheet( "background-color:lightyellow" );
      w->setCursor(Qt::PointingHandCursor);
      return w;
    } else
    {
      QLineEdit *le = new QLineEdit( parent );
      le->setFrame( false );
      le->setFocusPolicy( Qt::StrongFocus );
      le->setText( index.data( Qt::DisplayRole ).toString().simplified() );
      le->setAlignment( Qt::AlignCenter );
      le->installEventFilter( const_cast<MKTableItemDelegate*>(this) );
      return le;
    }
  } else
  { return QItemDelegate::createEditor( parent, option, index );
  }
}

//==============================================================================
//
//==============================================================================
void MKTableItemDelegate::updateEditorGeometry ( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  int i;
  QItemDelegate::updateEditorGeometry( editor, option, index );
  if( table->mode != MKTable::Polling ) return;
  i = index.data( MKTable::IndexRole ).toInt();
  i = table->assign_data[i].ss_enum_index;
  if( i >= 0 )
  { MKTableItemCBWidget *w = qobject_cast<MKTableItemCBWidget*>(editor);
    if( w == 0 ) return;
      // расчет высоты и ширины
      QStyleOption opt;
      opt.initFrom(w);
      QSize sz = w->size();
      int hh = ( w->sizeHintForRow(0) + 2*w->spacing() ) * qMin( w->count(),15);
      hh +=  w->spacing() + w->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, w) * 2;
      hh +=  w->style()->pixelMetric(QStyle::PM_MenuVMargin, &opt, w) * 2;
      int ww = w->sizeHintForColumn(0);
      ww +=  w->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, w) * 2;
      ww +=  w->style()->pixelMetric(QStyle::PM_MenuVMargin, &opt, w) * 2;
      if( w->count() > 10 ) ww += 20;
      sz.setHeight( hh );
      sz.setWidth( qMax( sz.width(),ww ) );
      w->resize( sz );
      QRect r_border = table->viewport()->geometry();
      int z = table->horizontalHeader()->frameGeometry().height()+5;
      r_border.adjust(0,z,-5,-z);
      QRect r_w      = w->frameGeometry();
      if( !r_border.contains( r_w ) )
      { int a;
        QRect r = w->geometry();
        a = r_border.right()-r_w.right();
        if( a < 0 ) r.translate( a, 0 );
        a = r_border.bottom()-r_w.bottom();
        if( a < 0 ) r.translate( 0, a );
        w->setGeometry( r );
      }
      w->highlightcursor();
  }
}

//==============================================================================
//
//==============================================================================
void MKTableItemDelegate::setEditorData( QWidget *editor,
                                            const QModelIndex &index ) const
{
  QItemDelegate::setEditorData( editor, index );
}

//==============================================================================
//
//==============================================================================
void MKTableItemDelegate::commitAndCloseEditor()
{
  QWidget *editor = qobject_cast<QWidget*>( sender() );
  if( editor == 0 ) return;
  emit commitData( editor );
  emit closeEditor( editor );
}

//==============================================================================
//
//==============================================================================
void MKTableItemDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model,
                                               const QModelIndex & index ) const
{
  Q_UNUSED( model );
  Q_UNUSED( index );

  if( table->mbmaster == 0 ) return;
  if( table->mode != MKTable::Polling ) return;

  int mm,ss,ii,i;
  bool ok;
  QVariant var;

  QLineEdit *le = qobject_cast<QLineEdit*>( editor );
  if( le )
  { QString str = le->text();
    if( str.startsWith("0x") )
    { var=str.remove(0,2).toInt(&ok,16);
    } else if( str.startsWith("0b") )
    { var=str.remove(0,2).toInt(&ok,2);
    } else if( str.startsWith("\'") )
    { QChar c;
      if( str.size()>1 ) c=str[1];
      var = c.toAscii();
    } else
    { var = str;
    }
  }else
  { MKTableItemCBWidget *w = qobject_cast<MKTableItemCBWidget*>(editor);
    if( w )
    { QListWidgetItem *litem = w->currentItem();
      if( litem ) var = litem->data( Qt::UserRole ).toString();
    }
  }

  if( var.isNull() ) return;

  // заполняем всю выделенную область
  QList<QTableWidgetItem*>  si = table->QTableWidget::selectedItems();
  foreach( QTableWidgetItem *titem, si )
  { i = titem->data(MKTable::IndexRole).toInt(&ok);
    if( !ok ) continue;
    if( i >= table->assign_data.size() ) continue;
    mm = table->assign_data[i].m_index;
    ss = table->assign_data[i].s_index;
    ii = table->assign_data[i].i_index;
    table->mbmaster->setSlotValue(mm,ss,ii, MMValue::fromVariant(var) );
  }
}

//==============================================================================
//
//==============================================================================
bool MKTableItemDelegate::eventFilter(QObject *obj, QEvent *event)
{
  QLineEdit *le = qobject_cast<QLineEdit*>( obj );
  if( le )
  {
    if (event->type() == QEvent::KeyPress)
    {  QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
       if( keyEvent->key() == Qt::Key_Space )
       { // обнуляем значение и подменяем нажатую клавишу
         le->clear();
         if(keyEvent->modifiers() & Qt::ControlModifier ) le->setText("1");
         QKeyEvent enterKeyPress(QEvent::KeyPress, Qt::Key_Enter, 0);
         QApplication::sendEvent(obj, &enterKeyPress);
         return true;
       }
    }
  }
  return QItemDelegate::eventFilter(obj, event);
}

//##############################################################################
//
//##############################################################################
MKTableItemCBWidget::MKTableItemCBWidget( QWidget *parent )
  : QListWidget( parent )
{
  row_under_mouse = -1;
  setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setTextElideMode(Qt::ElideNone);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setMouseTracking( true );
  setSpacing( 1 );
  setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );
}

//==============================================================================
//
//==============================================================================
void MKTableItemCBWidget::mouseReleaseEvent( QMouseEvent * event )
{
  QListWidget::mouseReleaseEvent( event );
  if( confirmEdit )
  {
      if( QMessageBox::question(this,"редактирование ячейки",
          "Вы действительно хотите изменить значение данной ячейки ?",
          QMessageBox::Yes|QMessageBox::Default,
          QMessageBox::No|QMessageBox::Escape)==QMessageBox::Yes)
      {
        emit editingFinished();
      }
      else deleteLater();
  }
  else emit editingFinished();
}

//==============================================================================
//
//==============================================================================
void MKTableItemCBWidget::mouseMoveEvent ( QMouseEvent * event )
{
  QListWidget::mouseMoveEvent( event );
  highlightcursor();
}

//==============================================================================
//
//==============================================================================
void MKTableItemCBWidget::leaveEvent ( QEvent * event )
{
  QListWidget::leaveEvent( event );
  QListWidgetItem *litem1 = item(row_under_mouse);
  if( litem1 ) litem1->setBackground( Qt::NoBrush  );
  row_under_mouse = -1;
}

//==============================================================================
//
//==============================================================================
void MKTableItemCBWidget::showEvent( QShowEvent * event )
{ QListWidget::showEvent( event );
  move( QCursor::pos() );
}
//==============================================================================
//
//==============================================================================
void MKTableItemCBWidget::highlightcursor()
{
  QListWidgetItem *litem;
  litem = itemAt( mapFromGlobal( QCursor::pos() ) );
  if( litem )
  { int i = row( litem );
    if( i != row_under_mouse )
    { QListWidgetItem *litem1 = item(row_under_mouse);
      if( litem1 ) litem1->setBackground( Qt::NoBrush );
      row_under_mouse = i;
      litem->setBackground( QColor("lightgrey") );
    }
  }
}
