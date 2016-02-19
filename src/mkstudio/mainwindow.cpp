#include "mainwindow.h"

#include "main.h"
#include "dialogs.h"
#include "misc.h"
#include "info.h"
#include "utils.h"
#include "helpwidget.h"

#include <console.h>
#include <mbmasterxml.h>

#include <QColorDialog>
#include <QClipboard>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>

//##############################################################################
// Информационное поле в строке состяния
//##############################################################################
class InfoLabel : public QLineEdit
{
public:
  InfoLabel( QWidget *parent = 0 )
    : QLineEdit( parent )
  {
    setReadOnly( true );
    setFocusPolicy( Qt::NoFocus );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    setMinimumWidth(130);
    setFrame( false );
    QPalette p = palette();
    p.setColor( QPalette::Base, p.color(QPalette::Window) );
    setPalette( p );
  }
};


//##############################################################################
/// Главное окно
//##############################################################################
MainWindow::MainWindow(MBMasterXMLPtr mbmaster)
    : mbmaster(mbmaster)
{
  setupUi( this );
  dw_settingssheet->hide();
  setWindowTitle(Utils::AppInfo::title());

  undo_list_current_point = 0;

  QSettings settings( QSETTINGS_PARAM );
  QPoint pos = settings.value("pos", QPoint(100,100)).toPoint();
  QSize size = settings.value("size", QSize(1000,800)).toSize();
  current_config_values_filename = settings.value( "values_file", "").toString();
  move( pos );
  resize( size );
  restoreState( settings.value("mainwindow").toByteArray() );

  QShortcut *shortcutDel = new QShortcut( QKeySequence( Qt::Key_Delete ), this);
  connect( shortcutDel, SIGNAL( activated() ),
           this,        SLOT( del_key_pressed() ) );

  QShortcut *shortcutCopyToConsole = new QShortcut( QKeySequence( Qt::Key_F5 ), this);
  connect( shortcutCopyToConsole, SIGNAL( activated() ),
           this,                  SLOT(   copy_to_console() ) );


  connect( mbmasterwidget, SIGNAL( attributes_saved(int,int,QString) ),
           this,       SLOT(  slot_attributes_saved(int,int,QString) ) );

  action_copy  -> setShortcut( QKeySequence::Copy  );
  action_cut   -> setShortcut( QKeySequence::Cut   );
  action_paste -> setShortcut( QKeySequence::Paste );

  action_copy  -> setEnabled( false );
  action_cut   -> setEnabled( false );
  action_paste -> setEnabled( true );
  action_undo  -> setEnabled( false );
  action_redo  -> setEnabled( false );
  action_values_save   -> setEnabled(  false );
  action_values_load   -> setEnabled(  false );

  full_time       = new InfoLabel;
  status_requests = new InfoLabel;
  status_answers  = new InfoLabel;
  status_errors   = new InfoLabel;
  statusbar->addPermanentWidget( full_time       );
  statusbar->addPermanentWidget( status_requests );
  statusbar->addPermanentWidget( status_answers  );
  statusbar->addPermanentWidget( status_errors   );

  menu_view->addAction(dw_modules->toggleViewAction());
  menu_view->addAction(dw_opros->toggleViewAction());
  menu_view->addAction(dw_console->toggleViewAction());
  menu_view->addAction(dw_settingssheet->toggleViewAction());
  menu_view->addSeparator();
  menu_view->addAction(tool_bar_1->toggleViewAction());
  menu_view->addAction(tool_bar_2->toggleViewAction());

  connect( tw->horizontalHeader(),
           SIGNAL(  sectionDoubleClicked(int) ),
           SLOT(    action_tw_header_double_clicked(int) ) );
  connect( tw,             SIGNAL( signalMinimizeStateChange( bool ) ),
           mbmasterwidget, SLOT  ( slotMkTableMinimizeStateChange(bool) ) );
  connect( tw,             SIGNAL( signalMinimizeAllHide(bool) ),
           mbmasterwidget, SLOT  ( slotMkTableMinimizeAllHide(bool)) );
  connect( mbmasterwidget, SIGNAL( signalMinimize( bool ) ),
           tw,             SLOT(     slotMinimize( bool ) ) );
  connect( mbmasterwidget, SIGNAL( signalOstsOpen() ),
           tw,             SLOT(   slotPlotOpen() ) );
  connect( mbmasterwidget, SIGNAL( signalOstsClose() ),
           tw,             SLOT(   slotPlotClose() ) );

  startTimer(500);

  mbmasterwidget->setMBMaster(mbmaster);

  tw->setRowCount(6);
  tw->setColumnCount(6);
  load_conf( settings.value("conf_file").toString() );

  tw->setMBMaster( mbmaster );

  play_mode = false;
  connect( tw,   SIGNAL( customContextMenuRequested(const QPoint&)),
           this,   SLOT( context_menu() ) );
  tw->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( tw, SIGNAL( item_changed_by_editor() ),
           this, SLOT( make_undo_point() ) );

  clear_undo_list();

  action_united_graph->setVisible( false );

  te_settingssheet->setFont( QFont("Courier",10 ) );
  te_settingssheet->setLineWrapMode( QTextEdit::NoWrap );
  new ScriptHighlighter(te_settingssheet->document());
}
//==============================================================================
/// Пункт меню "Количество строк"
//==============================================================================
void MainWindow::on_action_tw_rows_triggered()
{
  bool ok;
  int i = QInputDialog::getInt( this, Utils::AppInfo::title(), "Количество строк:",
                 tw->rowCount(), 0, 1000, 1,&ok );
  if( ok ) tw->setRowCount( i );
  fill_empty_items();
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Количество столбцов"
//==============================================================================
void MainWindow::on_action_tw_columns_triggered()
{
  bool ok;
  int i = QInputDialog::getInt( this, Utils::AppInfo::title(), "Количество столбцов:",
                 tw->columnCount(), 0, 1000, 1,&ok );
  if( ok ) tw->setColumnCount( i );
  fill_empty_items();
  make_undo_point();
}

//==============================================================================
/// Двойное нажатие на горизонтальный заголовок
//==============================================================================
void MainWindow::action_tw_header_double_clicked(int logicalIndex)
{
  QString str;
  QStringList sl;

  str = tw->model()->headerData( logicalIndex, Qt::Horizontal ).toString();

  TextEditDialog ted( this );
  ted.resize(200,200);
  ted.te->setPlainText( str );
  ted.label->setText("Название столбца:");
  if( ted.exec() == QDialog::Accepted )
  { str = ted.te->toPlainText();
    str.remove(";");
    tw->setHorizontalHeaderItem( logicalIndex, new QTableWidgetItem( str ) );
    tw->horizontalHeader()->hide(); // иначе не обновляется высота заголовка
    tw->horizontalHeader()->show();
  }
  make_undo_point();
}

//==============================================================================
/// Загрузка конфигурации из файла
//==============================================================================
bool MainWindow::load_conf( const QString &filename )
{
 if (filename.isEmpty())
     return false;

 QDomNodeList list;

 QDomDocument doc("MKStudio");
 QFile file(filename);
 if (!file.open(QIODevice::ReadOnly))
     return false;
 if (!doc.setContent(&file)) {
     file.close();
     return false;
 }
 file.close();

 if( doc.doctype().name() != "MKStudio" )
 { QMessageBox::information( this, Utils::AppInfo::title(), "Файл не является конфигурацией MKStudio.");
   return false;
 }

 list = doc.elementsByTagName("Table");
 if( list.size() )
 { QDomDocument doc_table;
   doc_table.appendChild( doc_table.importNode( list.item(0), true ) );
   tw->loadConfiguration( doc_table );
   te_settingssheet->setPlainText( tw->settingsSheet() );
 } else
 { on_action_new_triggered();
 }

 list = doc.elementsByTagName("MBConfig");
 if( list.size() )
 { QDomDocument doc_config;
   doc_config.appendChild( doc_config.importNode( list.item(0), true ) );
   mbconfigwidget->loadConfiguration( doc_config );
 } else
 { mbconfigwidget->clearConfiguration();
 }


 setWindowTitle( filename + " - " + Utils::AppInfo::title() );
 current_config_filename = filename;

 clear_undo_list();

 action_shortcuts->setEnabled(true);


 return true;
}

//==============================================================================
/// Очистка конфигурации
//==============================================================================
void MainWindow::on_action_new_triggered()
{ tw->closeAllRecorders();
  action_play->setChecked( false );
  current_config_filename.clear();
  setWindowTitle(Utils::AppInfo::title());
  tw->clear();
  te_settingssheet->clear();
  tw->setRowCount(6);
  tw->setColumnCount(6);
  mbconfigwidget->clearConfiguration();

  action_shortcuts->setEnabled(false);

  clear_undo_list();
}

//==============================================================================
/// Пункт меню "Загрузить конфигурацию"
//==============================================================================
void MainWindow::on_action_load_triggered()
{
   QString filename = QFileDialog::getOpenFileName (
                         this,  "Загрузить",
                         current_config_filename,
                         "Конфигурация (*.xml)"  );

  if( filename.isEmpty() ) return;

  tw->closeAllRecorders();
  if( play_mode ) on_action_play_triggered();

  load_conf( filename );
}

//==============================================================================
/// Сохранение конфигурации в файл
//==============================================================================
bool MainWindow::save_conf( const QString &filename )
{
 tw->setSettingsSheet( te_settingssheet->toPlainText() );

 QDomDocument doc("MKStudio");
 QDomElement root = doc.createElement("MKStudio");
 doc.appendChild(root);

 QDomDocument doc_table;
 tw->saveConfiguration( doc_table );
 root.appendChild( doc_table.documentElement() );

 QDomDocument doc_config;
 mbconfigwidget->saveConfiguration( doc_config );
 root.appendChild( doc_config.documentElement() );

 QFile file(filename);
 file.open( QIODevice::WriteOnly | QIODevice::Truncate);

 QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
 file.write( "<?xml version=\"1.0\" encoding=\"cp-1251\"?>\n" );
 file.write( codec->fromUnicode( doc.toString(2) ) );

 setWindowTitle(filename + " - " + Utils::AppInfo::title());
 current_config_filename = filename;
 return true;
}

//==============================================================================
/// Пункт меню "Сохранить"
//==============================================================================
void MainWindow::on_action_save_triggered()
{
  if( !current_config_filename.isEmpty() )
  { save_conf( current_config_filename );
  } else
  { on_action_save_as_triggered();
  }
}

//==============================================================================
/// Пункт меню "Сохранить как"
//==============================================================================
void MainWindow::on_action_save_as_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this, "Сохранить",
                            "",
                            "Конфигурация (*.xml)" );
  if( !fileName.isEmpty() )
  { save_conf( fileName );
  }
}


//==============================================================================
/// Пункт меню "Пуск"
//==============================================================================
void MainWindow::on_action_play_triggered()
{
  QString str,str2;
  QIcon icon;

  if( play_mode )
  { play_mode = false;
    str = "Пуск";
    icon = QIcon(":/icons/res/media_play.png");
    str2 = "Запустить опрос";
  } else
  { play_mode = true;
    str = "Стоп";
    icon = QIcon(":/icons/res/media_stop.png");
    str2 = "Остановить опрос";
  }
  action_play->setText( str  );
  action_play->setIcon( icon );
  action_play->setToolTip( str2 );
  //----------------------------------------------------------
  if( play_mode )
  { QByteArray ba;
    mbconfigwidget->saveConfiguration( ba );
    mbmasterwidget->load_config( ba );
    mbmasterwidget->polling_start();
    tw->setSettingsSheet( te_settingssheet->toPlainText() );
    tw->setMode( MKTable::Polling );
    tw->setContextMenuPolicy( Qt::DefaultContextMenu );
  } else
  { mbmasterwidget->clear_config();
    tw->setMode( MKTable::Edit );
    tw->setContextMenuPolicy( Qt::CustomContextMenu );
  }
  //----------------------------------------------------------
  action_tw_rows          -> setEnabled( !play_mode );
  action_tw_columns       -> setEnabled( !play_mode );
  action_row_add          -> setEnabled( !play_mode );
  action_column_add       -> setEnabled( !play_mode );
  action_cell_add_right   -> setEnabled( !play_mode );
  action_cell_add_down    -> setEnabled( !play_mode );
  action_row_delete       -> setEnabled( !play_mode );
  action_column_delete    -> setEnabled( !play_mode );
  action_cell_delete_left -> setEnabled( !play_mode );
  action_cell_delete_up   -> setEnabled( !play_mode );
  action_link             -> setEnabled( !play_mode );
  action_bold             -> setEnabled( !play_mode );
  action_underline        -> setEnabled( !play_mode );
  action_italic           -> setEnabled( !play_mode );
  action_align_left       -> setEnabled( !play_mode );
  action_align_center     -> setEnabled( !play_mode );
  action_align_right      -> setEnabled( !play_mode );
  action_foreground_color -> setEnabled( !play_mode );
  action_background_color -> setEnabled( !play_mode );
  action_values_save      -> setEnabled(  play_mode );
  action_values_load      -> setEnabled(  play_mode );

  //action_cells_span   -> setEnabled( !play_mode );
  action_save             -> setEnabled( !play_mode );
  if( play_mode )
  { action_copy         -> setEnabled( false );
    action_cut          -> setEnabled( false );
    action_paste        -> setEnabled( false );
    is_undo_enabled = action_undo->isEnabled();
    is_redo_enabled = action_redo->isEnabled();
    action_undo->setEnabled( false );
    action_redo->setEnabled( false );
  } else
  { action_paste        -> setEnabled( true );
    action_undo         -> setEnabled( false );
    action_redo         -> setEnabled( false );
    action_undo->setEnabled( is_undo_enabled );
    action_redo->setEnabled( is_redo_enabled );
  }
}

//==============================================================================
/// Пункт меню "Выделение" (шрифт)
//==============================================================================
void MainWindow::on_action_bold_triggered()
{
  bool b = false;

  QFont fnt;
  QTableWidgetItem *titeml;
  QList<QTableWidgetItem *> si = tw->selectedItems();
  if( si.count() == 0 ) return;
  b = !si.at(0)->font().bold();
  foreach( titeml, si )
  { if( titeml == 0 ) continue;
    fnt = titeml->font();
    fnt.setBold(b);
    titeml->setFont( fnt );
  }
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Подчеркивание" (шрифт)
//==============================================================================
void MainWindow::on_action_underline_triggered()
{
  bool b = false;

  QFont fnt;
  QTableWidgetItem *titeml;
  QList<QTableWidgetItem *> si = tw->selectedItems();
  if( si.count() == 0 ) return;
  b = !si.at(0)->font().underline();
  foreach( titeml, si )
  { if( titeml == 0 ) continue;
    fnt = titeml->font();
    fnt.setUnderline(b);
    titeml->setFont( fnt );
  }
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Наклон" (шрифт)
//==============================================================================
void MainWindow::on_action_italic_triggered()
{
  bool b = false;

  QFont fnt;
  QTableWidgetItem *titeml;
  QList<QTableWidgetItem *> si = tw->selectedItems();
  if( si.count() == 0 ) return;
  b = !si.at(0)->font().italic();
  foreach( titeml, si )
  { if( titeml == 0 ) continue;
    fnt = titeml->font();
    fnt.setItalic(b);
    titeml->setFont( fnt );
  }
  make_undo_point();
}

//==============================================================================
/// Выравнивание (шрифт)
//==============================================================================
void MainWindow::setAlign( int alignment )
{
  QTableWidgetItem *titeml;
  QList<QTableWidgetItem *> si = tw->selectedItems();
  foreach( titeml, si )
  { if( titeml == 0 ) continue;
    titeml->setTextAlignment( alignment );
  }
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Выравнивание влево" (шрифт)
//==============================================================================
void MainWindow::on_action_align_left_triggered()
{
  setAlign( Qt::AlignVCenter | Qt::AlignLeft );
}

//==============================================================================
/// Пункт меню "Выравнивание по центру" (шрифт)
//==============================================================================
void MainWindow::on_action_align_center_triggered()
{
  setAlign( Qt::AlignVCenter | Qt::AlignHCenter );
}

//==============================================================================
/// Пункт меню "Выравнивание вправо" (шрифт)
//==============================================================================
void MainWindow::on_action_align_right_triggered()
{
  setAlign( Qt::AlignVCenter | Qt::AlignRight );
}

//==============================================================================
/// Пункт меню "Добавить строку"
//==============================================================================
void MainWindow::on_action_row_add_triggered()
{
  tw->clearSelection();
  tw->insertRow( tw->currentRow() );
  tw->setCurrentCell ( tw->currentRow()-1, tw->currentColumn() );
  fill_empty_items();
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Удалить строку"
//==============================================================================
void MainWindow::on_action_row_delete_triggered()
{
  QTableWidgetItem *titem;
  int row =  tw->currentRow();
  int i;
  for( i = 0; i<tw->columnCount(); i++ )
  { titem = tw->item( row, i );
    if( titem == 0 ) continue;
    tw->closePersistentEditor( titem );
  }
  tw->clearSelection();
  tw->removeRow( row );
  tw->setCurrentCell ( row, tw->currentColumn() );
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Добавить столбец"
//==============================================================================
void MainWindow::on_action_column_add_triggered()
{
  tw->clearSelection();
  tw->insertColumn( tw->currentColumn() );
  tw->setCurrentCell ( tw->currentRow(), tw->currentColumn()-1 );
  fill_empty_items();
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Удалить столбец"
//==============================================================================
void MainWindow::on_action_column_delete_triggered()
{
  QTableWidgetItem *titem;
  int column =  tw->currentColumn();
  int i;
  for( i = 0; i<tw->rowCount(); i++ )
  { titem = tw->item( i , column );
    if( titem == 0 ) continue;
    tw->closePersistentEditor( titem );
  }
  tw->clearSelection();
  tw->removeColumn( column );
  tw->setCurrentCell ( tw->currentRow(), column );
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Вставка ячейки со сдвигом вправо"
//==============================================================================
void MainWindow::on_action_cell_add_right_triggered()
{
  int i;
  QTableWidgetItem *titem;

  int row     = tw->currentRow();
  int column  = tw->currentColumn();
  int n       = tw->columnCount();

  if((row<0)||(column<0)||(n<3)) return;
  for( i=n-1; i>column; i-- )
  {  titem = tw->item(row,i-1);
     if( titem )
     { tw->setItem(row,i, new QTableWidgetItem(*titem) );
     } else
     { tw->setItem(row,i, new QTableWidgetItem );
     }
  }
  tw->setItem(row,column, new QTableWidgetItem );
  make_undo_point();
}
//==============================================================================
/// Пункт меню "Вставка ячейки со сдвигом вниз"
//==============================================================================
void MainWindow::on_action_cell_add_down_triggered()
{
  int i;
  QTableWidgetItem *titem;

  int row     = tw->currentRow();
  int column  = tw->currentColumn();
  int n       = tw->rowCount();

  if((row<0)||(column<0)||(n<3)) return;
  for( i=n-1; i>row; i-- )
  {  titem = tw->item(i-1,column);
     if( titem )
     { tw->setItem(i,column, new QTableWidgetItem(*titem) );
     } else
     { tw->setItem(i,column, new QTableWidgetItem );
     }
  }
  tw->setItem(row,column, new QTableWidgetItem );
  make_undo_point();
}
//==============================================================================
/// Пункт меню "Удаление ячейки со сдвигом влево"
//==============================================================================
void MainWindow::on_action_cell_delete_left_triggered()
{
  int i;
  QTableWidgetItem *titem;

  int row     = tw->currentRow();
  int column  = tw->currentColumn();
  int n       = tw->columnCount();

  if((row<0)||(column<0)||(n<3)) return;
  for( i=column; i<(n-1); i++ )
  {  titem = tw->item(row,i+1);
     if( titem )
     { tw->setItem(row,i, new QTableWidgetItem(*titem) );
     } else
     { tw->setItem(row,i, new QTableWidgetItem );
     }
  }
  tw->setItem(row,n-1, new QTableWidgetItem );
  make_undo_point();
}
//==============================================================================
/// Пункт меню "Удаление ячейки со сдвигом вверх"
//==============================================================================
void MainWindow::on_action_cell_delete_up_triggered()
{
  int i;
  QTableWidgetItem *titem;

  int row     = tw->currentRow();
  int column  = tw->currentColumn();
  int n       = tw->rowCount();

  if((row<0)||(column<0)||(n<3)) return;
  for( i=row; i<(n-1); i++ )
  {  titem = tw->item(i+1,column);
     if( titem )
     { tw->setItem(i,column, new QTableWidgetItem(*titem) );
     } else
     { tw->setItem(i,column, new QTableWidgetItem );
     }
  }
  tw->setItem(n-1,column, new QTableWidgetItem );
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Привязка"
//==============================================================================
void MainWindow::on_action_link_triggered()
{
  QRegExp rx_assign("^(\\d+)/(\\d+)/(\\d+)$");

  int module_base  = -10;
  int slot_base    = -10;
  int n_base       = -10;
  int module_right = -10;
  int slot_right   = -10;
  int n_right      = -10;
  int module_down  = -10;
  int slot_down    = -10;
  int n_down       = -10;

  // выделение

  QList<QTableWidgetSelectionRange> sr = tw->selectedRanges();
  if( sr.count() != 1 )
  { QMessageBox::warning( this, Utils::AppInfo::title(), "Выделение не выбрано." );
    return;
  }

  int pos,i,j;
  QTableWidgetItem *titem;
  QString str_assign,str_assign_right,str_assign_down,str_format,str_ss;
  bool ss_confirm;
  QTableWidgetSelectionRange sr0 = sr.at(0);

  // разбор значения левой верхней ячейки и двух соседних

  int topRow      = sr0.topRow();
  int bottomRow   = sr0.bottomRow();
  int leftColumn  = sr0.leftColumn();
  int rightColumn = sr0.rightColumn();

  titem = tw->item( topRow, leftColumn );
  if( titem  )
  { str_assign  = titem->data(MKTable::AssignRole).toString();
    str_format  = titem->data(MKTable::FormatRole).toString();
    str_ss      = titem->data(MKTable::SSSelectorRole).toString();
    ss_confirm  = titem->data(MKTable::SSConfirmEditRole).toBool();
  }
  if( rightColumn > leftColumn )
  { titem = tw->item( topRow, leftColumn+1 );
    if( titem )
    { str_assign_right  = titem->data(MKTable::AssignRole).toString();
    }
  }
  if( bottomRow > topRow )
  { titem = tw->item( topRow+1, leftColumn );
    if( titem )
    { str_assign_down  = titem->data(MKTable::AssignRole).toString();
    }
  }

  if( rx_assign.indexIn( str_assign ) >= 0 )
  { module_base = rx_assign.cap(1).toInt();
    slot_base   = rx_assign.cap(2).toInt();
    n_base      = rx_assign.cap(3).toInt();
  }
  if( rx_assign.indexIn( str_assign_right ) >= 0 )
  { module_right = rx_assign.cap(1).toInt();
    slot_right   = rx_assign.cap(2).toInt();
    n_right      = rx_assign.cap(3).toInt();
  }
  if( rx_assign.indexIn( str_assign_down ) >= 0 )
  { module_down = rx_assign.cap(1).toInt();
    slot_down   = rx_assign.cap(2).toInt();
    n_down      = rx_assign.cap(3).toInt();
  }

  // разбор текущего листа настроек и анализ на "enum"

  QStringList sl;
  QString ss_str = te_settingssheet->toPlainText();
  ss_str.remove( QRegExp("//[^\n]*") );
  ss_str = ss_str.simplified();
  QRegExp rx = QRegExp("\\benum\\b\\s*(\\w+)\\b\\s*\\{");
  pos=0;
  while( (pos = rx.indexIn(ss_str, pos)) != -1 )
  { pos += rx.matchedLength();
    sl << rx.cap(1);
  }

  // создание диалога
  QString posAddress;
  if( tw->selectedItems().count() == 1 )
  {
    posAddress = QString("(%1;%2)").arg(titem->column()+1).arg(titem->row()+1);
  }
  else
  {
    posAddress = "(-;-)";
  }

  AssignDialog dialog( this, str_assign,str_format,str_ss,sl,ss_confirm );
  dialog.setWindowTitle("Привязка. Ячейка:"+posAddress);

  // настройка параметров заполнения ячеек

  if( topRow == bottomRow )
  { dialog.cbox_fill_row -> setEnabled( false );
  }
  if( leftColumn == rightColumn )
  { dialog.cbox_fill_column -> setEnabled( false );
  }

  if( !rx_assign.exactMatch( str_assign ) )
  { if((topRow == bottomRow )&&( leftColumn != rightColumn ))
    { dialog.cbox_fill_column -> setChecked( true );
    } else if((topRow != bottomRow )&&( leftColumn == rightColumn ))
    { dialog.cbox_fill_row -> setChecked( true );
    } else if((topRow != bottomRow )&&( leftColumn != rightColumn ))
    { dialog.cbox_fill_row -> setChecked( true );
    }
  }

  if( module_right == (module_base+1) )
  { dialog.cb_fill_column->setCurrentIndex(0);
    dialog.cbox_fill_column -> setChecked( true );
  } else if( slot_right == (slot_base+1) )
  { dialog.cb_fill_column->setCurrentIndex(1);
    dialog.cbox_fill_column -> setChecked( true );
  } else if( n_right == (n_base+1) )
  { dialog.cb_fill_column->setCurrentIndex(2);
    dialog.cbox_fill_column -> setChecked( true );
  }

  if( module_down == (module_base+1) )
  { dialog.cb_fill_row->setCurrentIndex(0);
    dialog.cbox_fill_row->setChecked( true );
  } else if( slot_down == (slot_base+1) )
  { dialog.cb_fill_row->setCurrentIndex(1);
    dialog.cbox_fill_row -> setChecked( true );
  } else if( n_down == (n_base+1) )
  { dialog.cb_fill_row->setCurrentIndex(2);
    dialog.cbox_fill_row -> setChecked( true );
  }

  // вызов диалога

  if( dialog.exec() != QDialog::Accepted ) return;

  str_assign  = dialog.assign;
  str_format  = dialog.format;
  str_ss      = dialog.ss;
  ss_confirm  = dialog.ss_confirm;

  if( !rx_assign.exactMatch( str_assign ) ) return;

  int flag_array[2][3]; // массив флагов заполнения ячеек
  bool b[2]; // признак выделения тррк/столбцов
  int module_titem;
  int slot_titem;
  int n_titem;

  rx_assign.indexIn( str_assign );
  module_base = rx_assign.cap(1).toInt();
  slot_base   = rx_assign.cap(2).toInt();
  n_base      = rx_assign.cap(3).toInt();

  b[0] = dialog.cbox_fill_row->isChecked();
  b[1] = dialog.cbox_fill_column->isChecked();
  memset( flag_array, 0, sizeof(flag_array) );
  if( b[1] ) // столбцы
  { switch( dialog.cb_fill_column->currentIndex() )
    { case(0): flag_array[1][2]=1; break; // модуль
      case(1): flag_array[1][1]=1; break; // слот
      case(2): flag_array[1][0]=1; break; // номер
    }
  }
  if( b[0] ) // строки
  { switch( dialog.cb_fill_row->currentIndex() )
    { case(0): flag_array[0][2]=1; break; // модуль
      case(1): flag_array[0][1]=1; break; // слот
      case(2): flag_array[0][0]=1; break; // номер
    }
  }
  module_titem = rx_assign.cap(1).toInt();
  slot_titem   = rx_assign.cap(2).toInt();
  n_titem      = rx_assign.cap(3).toInt();

  // заполнение ячеек
  for( i=topRow; i<=bottomRow; i++ )
  { for( j=leftColumn; j<=rightColumn; j++ )
    { titem = tw->item( i, j );
      if( titem  == 0 )
      { titem = new QTableWidgetItem;
        tw->setItem( i,j, titem );
      }

      // начальное значение ячейки
      module_titem = module_base;
      slot_titem   = slot_base;
      n_titem      = n_base;

      // заполенние
      if( b[0] && b[1] ) // и строки и столбцы выделены
      { module_titem += (i-topRow)*flag_array[0][2] + (j-leftColumn)*flag_array[1][2];
        slot_titem   += (i-topRow)*flag_array[0][1] + (j-leftColumn)*flag_array[1][1];
        n_titem      += (i-topRow)*flag_array[0][0] + (j-leftColumn)*flag_array[1][0];
      } else // только строки или только столбцы
      { int k=0;
        if( b[0] ) k = (bottomRow-topRow+1)*(j-leftColumn)   + i - topRow;      // строки
        if( b[1] ) k = (i-topRow)*(rightColumn-leftColumn+1) + j - leftColumn;  // столбцы
        module_titem += k*flag_array[0][2] + k*flag_array[1][2];
        slot_titem   += k*flag_array[0][1] + k*flag_array[1][1];
        n_titem      += k*flag_array[0][0] + k*flag_array[1][0];
      }

      // вывод значений в ячейки
      titem->setData( MKTable::AssignRole,
                      QString().sprintf("%d/%d/%d",module_titem,slot_titem,n_titem) );
      titem->setData( MKTable::FormatRole, str_format );
      titem->setData( MKTable::SSSelectorRole, str_ss );
      titem->setData( MKTable::SSConfirmEditRole, ss_confirm );
    }
  }
  tw->setMode( MKTable::Edit );
  make_undo_point();
}

//==============================================================================
/// Перенести внутренний буфер обмена в Clipboard
//==============================================================================
void MainWindow::toClipboard()
{
  QByteArray ba;
  QDataStream ds( &ba, QIODevice::WriteOnly );
  for( int i=0; i<table_clipboard.size(); i++ )
  { ds << table_clipboard[i].item;
    ds << table_clipboard[i].row;
    ds << table_clipboard[i].col;
  }

  QMimeData *mime = new QMimeData;
  mime->setData( "application/mkstudio-cells-mime", ba );
  QApplication::clipboard()->setMimeData( mime );
}

//==============================================================================
/// Считать из Clipboard внутренний буфер обмена
//==============================================================================
void MainWindow::fromClipboard()
{
  table_clipboard.clear();

  const QByteArray ba = QApplication::clipboard()->mimeData()
                                    ->data( "application/mkstudio-cells-mime" );
  QDataStream ds( ba );

  TableClipboardItem tci;
  while( !ds.atEnd() )
  { ds >> tci.item;
    ds >> tci.row;
    ds >> tci.col;
    table_clipboard << tci;
  }
}

//==============================================================================
/// Пункт меню "Копировать" (копирование во внутренний буфер обмена)
//==============================================================================
void MainWindow::on_action_copy_triggered()
{
  QTableWidgetItem *titem;

  TableClipboardItem tci;

  QList<QTableWidgetItem*> si = tw->selectedItems();
  table_clipboard.clear();

  foreach(titem, si )
  { if( titem == 0 ) continue;
    tci.item = *titem;
    tci.row = titem->row();
    tci.col = titem->column();
    table_clipboard << tci;
  }

  int min_col,min_row,i;
  min_col=min_row=10000;
  for( i=0; i<table_clipboard.size(); i++ )
  { if( table_clipboard[i].row < min_row ) min_row = table_clipboard[i].row;
    if( table_clipboard[i].col < min_col ) min_col = table_clipboard[i].col;
  }
  for( i=0; i<table_clipboard.size(); i++ )
  { table_clipboard[i].row -= min_row;
    table_clipboard[i].col -= min_col;
  }

  toClipboard();
}

//==============================================================================
/// Пункт меню "Вырезать" (копировать в буфер обмена и очистить выделенные ячейки)
//==============================================================================
void MainWindow::on_action_cut_triggered()
{
  on_action_copy_triggered();
  QTableWidgetItem *titem;

  QList<QTableWidgetItem*> si = tw->selectedItems();
  foreach(titem, si )
  {  tw->setItem( titem->row(), titem->column(), new QTableWidgetItem );
  }
  toClipboard();
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Вставить" (вставить ячейки из внутреннего буфера обмена)
//==============================================================================
void MainWindow::on_action_paste_triggered()
{
  fromClipboard();

  int i,c,r;
  int row    = 100000;
  int column = 100000;
  QTableWidgetItem *titem;

  int max_row    = tw->rowCount();
  int max_column = tw->columnCount();

  // поиск верхнего левого угла выделения
  QList<QTableWidgetItem*> si = tw->selectedItems();
  foreach(titem, si )
  { if( titem->row()    < row )    row    = titem->row();
    if( titem->column() < column ) column = titem->column();
  }
  // собственно вставка
  for( i=0; i<table_clipboard.size(); i++ )
  { r = table_clipboard[i].row + row;
    c = table_clipboard[i].col + column;
    if( r >= max_row  )    continue;
    if( c >= max_column  ) continue;
    tw->setItem( r, c, new QTableWidgetItem(table_clipboard[i].item)  );
  }
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Цвет текста"
//==============================================================================
void MainWindow::on_action_foreground_color_triggered()
{
  int i,j,k,i1,i2,j1,j2;
  QTableWidgetItem *titem;

  QList<QTableWidgetSelectionRange> sr = tw->selectedRanges();
  if( sr.count() == 0 ) return;
  titem = tw->item( sr.at(0).topRow(), sr.at(0).leftColumn() );
  if( titem == 0 ) return;
  QVariant var = titem->data(Qt::ForegroundRole);
  QColor color = var.isValid() ? var.value<QColor>() : QColor( "black" );
  color = QColorDialog::getColor( color, this );
  if( !color.isValid() ) return;

  for( k=0; k<sr.count(); k++ )
  { i1 = sr.at(k).topRow();
    i2 = sr.at(k).bottomRow();
    j1 = sr.at(k).leftColumn();
    j2 = sr.at(k).rightColumn();
    for( i=i1; i<=i2; i++ )
    { for( j=j1; j<=j2; j++ )
       { titem = tw->item(i,j);
        if( titem == 0 ) continue;
        titem->setData( Qt::ForegroundRole,  color );
      }
    }
  }
  tw->clearSelection();
  make_undo_point();
}

//==============================================================================
/// Пункт меню "Цвет фона"
//==============================================================================
void MainWindow::on_action_background_color_triggered()
{
  int i,j,k,i1,i2,j1,j2;
  QTableWidgetItem *titem;


  QList<QTableWidgetSelectionRange> sr = tw->selectedRanges();
  if( sr.count() == 0 ) return;
  titem = tw->item( sr.at(0).topRow(), sr.at(0).leftColumn() );
  if( titem == 0 ) return;
  QVariant var = titem->data(Qt::BackgroundRole);
  QColor color = var.isValid() ? var.value<QColor>() : QColor( "white" );
  color = QColorDialog::getColor( color, this );
  if( !color.isValid() ) return;

  var = color;
  if( color == QColor("white") ) var = QVariant();
  for( k=0; k<sr.count(); k++ )
  { i1 = sr.at(k).topRow();
    i2 = sr.at(k).bottomRow();
    j1 = sr.at(k).leftColumn();
    j2 = sr.at(k).rightColumn();
    for( i=i1; i<=i2; i++ )
    { for( j=j1; j<=j2; j++ )
       { titem = tw->item(i,j);
        if( titem == 0 ) continue;
        titem->setData( Qt::BackgroundRole,  var );
      }
    }
  }
  tw->clearSelection();
  tw->setMode( MKTable::Edit );
  make_undo_point();
}

//==============================================================================
/// Создание точки отмены
//==============================================================================
void MainWindow::make_undo_point()
{
  QBuffer buffer;
  buffer.open(QIODevice::WriteOnly);
  QDataStream ds( &buffer );
  QTableWidgetItem defaultItem;
  int i,j,ni,nj;
  QTableWidgetItem *si;

  ni = tw->rowCount();
  nj = tw->columnCount();

  ds << ni;
  ds << nj;

  for(i=0;i<ni;i++)
  { si =  tw->verticalHeaderItem(i);
    if( si == 0 ) defaultItem.write(ds);
    else          si->write( ds );
  }

  for(j=0;j<nj;j++)
  { si =  tw->horizontalHeaderItem(j);
    if( si == 0 ) defaultItem.write(ds);
    else          si->write( ds );
  }

  for(i=0;i<tw->rowCount();i++)
  { for(j=0;j<tw->columnCount();j++)
    { si = tw->item(i,j);
       if( si == 0 ) defaultItem.write(ds);
       else          si->write( ds );
    }
  }

  while(undo_list_current_point)
  { undo_list_current_point--;
    undo_list.removeLast();
  }

  undo_list << buffer.data();
  if( undo_list.count() > 20 )
  { undo_list.removeFirst();
  }

  undo_list_current_point = 0;
  action_undo->setEnabled( true  );
  action_redo->setEnabled( false );

}
//==============================================================================
/// Пункт меню "Отменить"
//==============================================================================
void MainWindow::on_action_undo_triggered()
{
  if( undo_list.size() <= (undo_list_current_point+1) ) return;
  undo_list_current_point++;
  applay_undo_point();
  if( undo_list.size() == (undo_list_current_point+1) )
  { action_undo->setEnabled( false );
  }
  action_redo->setEnabled( true );
}


//==============================================================================
/// Пункт меню "Восстановить"
//==============================================================================
void MainWindow::on_action_redo_triggered()
{
 if( undo_list_current_point == 0 ) return;
 undo_list_current_point--;
 applay_undo_point();
 if( undo_list_current_point == 0 )
 { action_redo->setEnabled( false );
 }
 action_undo->setEnabled( true );
}

//==============================================================================
/// Восстановление из "undo_list_current_point" точки UNDO массива
//==============================================================================
void MainWindow::applay_undo_point()
{
  int i,j,ni,nj;

  if( undo_list.size() <= undo_list_current_point ) return;

  tw->clear();
  QByteArray ba = undo_list.at( undo_list.size() - undo_list_current_point - 1 );
  QDataStream ds( ba );
  QTableWidgetItem titem;

  if( ba.size() == 0 ) return;

  ds >> ni;
  ds >> nj;

  tw->setRowCount( ni );
  tw->setColumnCount( nj );

  for( i=0; i<ni; i++ )
  { ds >> titem;
    tw->setVerticalHeaderItem(i, new QTableWidgetItem(titem) );
  }
  for( j=0; j<nj; j++ )
  { titem.read( ds );
    tw->setHorizontalHeaderItem(j, new QTableWidgetItem(titem) );
  }
  for(i=0;i<tw->rowCount();i++)
  { for(j=0;j<tw->columnCount();j++)
    {  titem.read( ds );
       tw->setItem( i,j, new  QTableWidgetItem(titem) );
    }
  }
  tw->horizontalHeader()->hide(); // иначе не обновляется высота заголовка
  tw->horizontalHeader()->show();
}

//==============================================================================
/// Очистка массива UNDO
//==============================================================================
void MainWindow::clear_undo_list()
{
  undo_list.clear();
  undo_list_current_point=0;
  make_undo_point();
  action_undo  -> setEnabled( false );
  action_redo  -> setEnabled( false );
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_values_save_triggered()
{
  QDomDocument doc;
  mbmaster->saveValues( doc );

  QString fileName = current_config_values_filename;
  if( fileName.isEmpty() )
  { fileName=current_config_filename;
    if( fileName.endsWith(".xml") ) fileName.chop(4);
  }
  if( fileName.isEmpty() )
  { fileName="test";
  }
  if( fileName.endsWith(".values.xml") ) fileName.chop(11);
  fileName = QFileDialog::getSaveFileName(this, "Сохранить",
                            fileName+".values.xml",
                            "Значения (*.values.xml)" );

  if( !fileName.isEmpty() )
  { current_config_values_filename = fileName;
    QFile file(fileName);
    file.open( QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(doc.toByteArray(2) );
  }
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_values_load_triggered()
{
  QString fileName = current_config_values_filename;
  if( fileName.isEmpty() )
  { fileName = current_config_filename;
    if( fileName.isEmpty() ) fileName="test";
    if( fileName.endsWith(".xml") ) fileName.chop(4);
  }

  fileName = QFileDialog::getOpenFileName (
                         this,  "Загрузить",
                         fileName,
                         "Значения (*.values.xml)"  );

  if( fileName.isEmpty() ) return;
  current_config_values_filename = fileName;

  QDomDocument doc;
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))  return;
  if (!doc.setContent(&file))    return;
  mbmaster->loadValues( doc );
  file.close();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_export_config_triggered()
{
  QString fileName = current_config_filename;
  if( fileName.endsWith(".xml") ) fileName.chop(4);

  fileName = QFileDialog::getSaveFileName(this, "Сохранить",
                            fileName+".txt",
                            "Текст (*.txt)" );
  if( fileName.isEmpty() ) return;

  QString str = mbconfigwidget->exportConfiguration();

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)) return;
  QTextStream st( &file );
  st.setCodec( QTextCodec::codecForName("Windows-1251") );
  st << str;
}

//==============================================================================
///
//==============================================================================
void MainWindow::on_action_help_triggered()
{
  helpwidget->show();
  helpwidget->activateWindow();
}

//==============================================================================
///
//==============================================================================
void MainWindow::on_action_help_about_triggered()
{

  QMessageBox::about( this, Utils::AppInfo::title(),
     "<body>"
     "Программа <b>MKStudio</b> для работы <br> "
     "с микропроцессорными контроллерами <br> из "
     "состава&nbsp;ПТК УМИКОН. <br><br>"
     "Версия программы: " + ProgramInformation::version() + "<br><br>"
     "ЗАО \"Инкоммет\" <br><br>"
     "<table border=0 cellpadding=1 cellspacing=1> "
     "  <tbody><tr><td>тел.</td><td>(499) 171-97-49</td></tr> "
     "         <tr><td>тел./факс</td><td>(495) 737-56-36</td></tr>"
     "         <tr><td>e-mail&nbsp;&nbsp;</td><td>mail@inkommet.ru</td></tr>"
     "         <tr><td>url</td><td><a href=\"http://www.inkommet.ru\">"
                                      "http://www.inkommet.ru</a></td>"
     "    </tr></tbody></table>"
     "</body>"  );

}

//==============================================================================
///
//==============================================================================
void MainWindow::on_action_united_graph_triggered()
{
  UnitedSlots *us = new UnitedSlots(this);
  us->show();
}

//==============================================================================
/// Удалить выделенные ячейки (нажатие на клавиатуре "DELETE")
//==============================================================================
void MainWindow::del_key_pressed()
{
  QTableWidgetItem *titem;
  QList<QTableWidgetItem*> si = tw->selectedItems();
  foreach(titem, si )
  {  tw->setItem( titem->row(), titem->column(), new QTableWidgetItem );
  }
  make_undo_point();
}

//==============================================================================
/// Скопировать значение в выделенных ячейках в консоль
//==============================================================================
void MainWindow::copy_to_console()
{
  QTableWidgetItem *titem;
  QList<QTableWidgetItem*> si = tw->selectedItems();
  foreach(titem, si )
  { if( titem == 0 ) continue;
    Console::Print( Console::Information, titem->text() + "\n" );
  }
}

//==============================================================================
/// Заполнить все отсутствующие ячейки (QTableWidgetItem::item()==0)
//==============================================================================
void MainWindow::fill_empty_items()
{
  int i,j;
  int r = tw->rowCount();
  int c = tw->columnCount();

  for( i=0; i<r; i++ )
  { for( j=0; j<c; j++ )
    { if( tw->item(i,j) == 0 )
      { tw->setItem( i,j, new QTableWidgetItem );
      }
    }
  }
}

//==============================================================================
/// Контекстное меню
//==============================================================================
void MainWindow::context_menu()
{
  if( ! tw->selectionModel()->hasSelection() ) return;

  QMenu menu;
  menu.addAction( action_copy );
  menu.addAction( action_cut );
  menu.addAction( action_paste );
  menu.addSeparator();
  menu.addAction( action_link );
  menu.addSeparator();
  menu.addAction( action_bold );
  menu.addAction( action_italic );
  menu.addAction( action_underline );
  menu.addSeparator();
  menu.addAction( action_align_left );
  menu.addAction( action_align_center );
  menu.addAction( action_align_right );
  menu.addAction( action_foreground_color );
  menu.addAction( action_background_color );
  menu.exec( QCursor::pos() );
}
/*
//       ОБЪЕДИНЕНИЕ ЯЧЕЕК РАБОТАЕТ НЕАДЕКВАТНО

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_cells_span_triggered()
{
  QList<QTableWidgetSelectionRange> sr = tw->QTableWidget::selectedRanges();
  if( sr.count() < 1 ) return;

  int col_left   = sr.at(0).leftColumn();
  int col_right  = sr.at(0).rightColumn();
  int row_top    = sr.at(0).topRow();
  int row_bottom = sr.at(0).bottomRow();
  foreach( QTableWidgetSelectionRange sr_i, sr )
  { if( sr_i.leftColumn()  < col_left   ) col_left   = sr_i.leftColumn();
    if( sr_i.rightColumn() > col_right  ) col_right  = sr_i.rightColumn();
    if( sr_i.topRow()      < row_top    ) row_top    = sr_i.topRow();
    if( sr_i.bottomRow()   > row_bottom ) row_bottom = sr_i.bottomRow();
  }

  if(    ( tw->rowSpan(row_top,col_left)    == ( row_bottom-row_top+1 ) )
      && ( tw->columnSpan(row_top,col_left) == ( col_right-col_left+1 ) ) )
  { // отмена объединения
    QWidget *p = tw->cellWidget( row_top, col_left);
    if(p) p->deleteLater();
    tw->setSpan(row_top,col_left,1,1);
  }else
  {// объединение
    tw->setSpan( row_top, col_left, row_bottom-row_top+1, col_right-col_left+1);
    tw->setCellWidget( row_top, col_left, new QLabel("TEST",this) );
  }
  make_undo_point();
}
*/

//==============================================================================
/// Пункт меню "Настройки"
//==============================================================================
void MainWindow::on_action_settings_triggered()
{
  SettingsDialog dialog( this );
  dialog.fcb_program->setCurrentFont( qApp->font() );
  dialog.fcb_tw->setCurrentFont( tw->font() );
  dialog.sb_program->setValue( font().pointSize() );
  dialog.sb_tw->setValue( tw->font().pointSize() );
  dialog.cb_mbconsole_packets->setChecked( Console::messageTypes() & Console::ModbusPacket );
  dialog.sb_max_packet_len->setValue( mbmaster->maximumPacketLength() );
  dialog.cb_optimize_read->setChecked( tw->optimizeReadMode() );

  if( dialog.exec() != QDialog::Accepted ) return;

  QFont f = dialog.fcb_program->currentFont();
  f.setPointSize( dialog.sb_program->value() );
  qApp->setFont( f );
  f = dialog.fcb_tw->currentFont();
  f.setPointSize( dialog.sb_tw->value() );
  tw->setFont( f );
  int i,a,n;
  n = tw->verticalHeader()->count();
  a = f.pointSize()+11;
  for(i=0; i<n; i++)
  { tw->verticalHeader()->resizeSection( i, a );
  }
  tw->verticalHeader()->setDefaultSectionSize( a );
  Console::setMessageTypes( Console::ModbusPacket,
                                   dialog.cb_mbconsole_packets->isChecked() );
  mbmaster->setMaximumPacketLength( dialog.sb_max_packet_len->value() );
  tw->setOptimizeReadMode( dialog.cb_optimize_read->isChecked() );
}

//==============================================================================
/// Действия при изменении области выделения в таблице
//==============================================================================
void MainWindow::on_tw_itemSelectionChanged()
{
  QList<QTableWidgetItem *>  si = tw->selectedItems();
  int c = si.count();
  if( !play_mode )
  { action_copy -> setEnabled( c != 0 );
    action_cut  -> setEnabled( c != 0 );
  }
  if( c != 1 ) return;
  if( si.at(0) == 0 ) return;
  statusbar->showMessage( "Связка: " + si.at(0)->data(MKTable::AssignRole).toString() );
}

//==============================================================================
///
//==============================================================================
void MainWindow::slot_attributes_saved( int module, int slot, QString attributes )
{
  mbconfigwidget -> setSlotAttributes( module, slot, attributes );
  mbmaster       -> setSlotAttributes( module, slot, attributes );

}
//==============================================================================
/// Таймер (500 мс)
//==============================================================================
void MainWindow::timerEvent(QTimerEvent *event )
{
  Q_UNUSED( event );

  QList<QTableWidgetItem *> si = tw->selectedItems();

  double f,s=0;
  int n=0;
  bool ok;

  foreach( QTableWidgetItem *ti, si )
  { if(ti)
    { f = ti->text().toDouble( &ok );
      if( ok )
      { s += f;
        n++;
      }
    }
  }

  if(n)
  {  statusbar->showMessage("Среднее: " + QString::number( s/n , 'f') );
  }

  full_time       -> setText( QString(" Время цикла: %1 мс ").arg(mbmaster->full_time()) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter()) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter()) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter()) );
}

//==============================================================================
/// Закрытие окна
//==============================================================================
void MainWindow::closeEvent ( QCloseEvent * event )
{
  Q_UNUSED( event );

  tw->closeAllRecorders();
  action_play->setChecked( false );
  mbmasterwidget->clear_config();

  helpwidget->close();

  QSettings settings( QSETTINGS_PARAM );
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("mainwindow", saveState() );
  settings.setValue("conf_file", current_config_filename );
  settings.setValue("values_file", current_config_values_filename );

}

//==============================================================================
///
//==============================================================================
void MainWindow::on_action_shortcuts_triggered()
{
    ShortCutsDialog dialog(this, current_config_filename);

    if(dialog.exec() != QDialog::Accepted) return;

    tw->shortcutList = dialog.shortcutList;
    tw->hotkey = dialog.hotkey;

    save_conf(current_config_filename);
}
