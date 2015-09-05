#include <QtGui>
#include <QtXml>

#include <QDesktopWidget>
#include <QMessageBox>

#include "mainwindow.h"
#include "main.h"
#include "interface.h"
#include "serialport.h"
#include "mbtcp.h"
#include "mbmasterxml.h"
#include "serialport.h"
#include "mktable.h"
#include "crc.h"

#ifdef Q_OS_WIN32
  #define PLUGIN_FILE_MASK1 "mkview_*.dll"
  #define PLUGIN_FILE_MASK2 ".dll"
#endif
#ifdef Q_OS_UNIX
  #define PLUGIN_FILE_MASK1 "libmkview_*.so"
  #define PLUGIN_FILE_MASK2 ".so"
#endif

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

//#######################################################################################
//
//#######################################################################################
InitDialog::InitDialog( QWidget *parent )
  : QDialog( parent )
{
  QString str,str2;
  int i;

  setupUi( this );
  setWindowTitle(app_header);

  QSettings settings( QSETTINGS_PARAM );

  //--------------------------------------------------------
  QStringList ports = SerialPort::queryComPorts();
  foreach( str, ports )
  { str2=str;
    str2.replace(';',"        ( ");
    cb_portname->addItem( str2+" )", str.section(';',0,0) );
  }
  cb_portname->addItem( "Modbus TCP", "===TCP===" );
  i = cb_portname->findData( settings.value("portname").toString() );
  if( i >= 0 ) cb_portname->setCurrentIndex(i);
  //--------------------------------------------------------
  cb_portspeed->clear();
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
  //--------------------------------------------------------
  setModulesComboBox( cb_modulename );
  i = cb_modulename->findText( settings.value("modulename").toString() );
  if( i >= 0 ) cb_modulename->setCurrentIndex(i);
  //--------------------------------------------------------
  le_host->setText( settings.value("tcphost","1").toString() );
  //--------------------------------------------------------
  QRegExp rx("(\\d{1,3})(\\.\\d{1,2})?");
  QValidator *validator = new QRegExpValidator(rx, this);
  le_node->setValidator(validator);
  le_node->setText( settings.value("modulenode","1").toString() );
  //--------------------------------------------------------
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
  l_host->setHidden(      !b );
  le_host->setHidden( !b );
}

//==============================================================================
//
//==============================================================================
void InitDialog::setModulesComboBox( QComboBox *cb )
{
  QStringList sl;
  QString str,str2;

  cb->clear();

  QDir current_dir(qApp->applicationDirPath() );
  //-------------------------------------------------
  sl = current_dir.entryList( QStringList() << "*.xml" );
  foreach( str, sl )
  { str2=str.toUpper();
    str2.remove(".XML");
    cb->addItem( str2, current_dir.absoluteFilePath(str) );
  }
  //-------------------------------------------------
  QDir mikkon_dir( qApp->applicationDirPath()+"/mikkon" );
  sl = mikkon_dir.entryList( QStringList() << "*.xml" );
  foreach( str, sl )
  { str2=str.toUpper();
    str2.remove(".XML");
    cb->addItem( str2, mikkon_dir.absoluteFilePath(str) );
  }
  //-------------------------------------------------
  sl = current_dir.entryList( QStringList() << PLUGIN_FILE_MASK1 );
  QPluginLoader pluginLoader;
  QObject *plugin;
  MKViewPluginInterface *ptr;
  foreach( str, sl )
  {
    str2 = current_dir.absoluteFilePath(str);
    pluginLoader.setFileName( str2 );
    plugin = pluginLoader.instance();
    if( plugin == 0) QMessageBox::critical(this,"error",pluginLoader.errorString() );
    if( plugin == 0) continue;
    ptr = qobject_cast<MKViewPluginInterface*>(plugin);
    if( ptr == 0 ) continue;
    cb->addItem( ptr->moduleName() + " [плагин]", str2 );
    if( !pluginLoader.unload() )
    { QMessageBox::critical( this,app_header,"Ошибка при выгрузке плагина.");
    }
  }
  //-------------------------------------------------
  cb->model()->sort(0);
}

//=======================================================================================
//
//=======================================================================================
void InitDialog::on_tb_modulehelp_clicked()
{
  QString filename = cb_modulename->itemData( cb_modulename->currentIndex() ).toString();
  if( filename.contains(".xml") )
  {
    QMessageBox::information( this, app_header, "Описание модуля отсутствует." );
    return;
  }
  if( filename.contains( PLUGIN_FILE_MASK2 ) )
  {
    QObject *plugin;
    MKViewPluginInterface *ptr;
    QPluginLoader pluginLoader( filename );
    plugin = pluginLoader.instance();
    if( plugin == 0) return;
    ptr = qobject_cast<MKViewPluginInterface*>(plugin);
    if( ptr == 0 ) return;
    ptr->helpWindow( this );
    if( !pluginLoader.unload() )
    { QMessageBox::critical( this,app_header,"Ошибка при выгрузке плагина.");
    }
  }
}

//=======================================================================================
//
//=======================================================================================
void InitDialog::on_tb_moduleinfo_clicked()
{
  int ret,i;

  qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );

  QString node = module_node();

  SerialPort serialport;
  serialport.setName( portname() );
  serialport.setSpeed( portspeed() );
  if( !serialport.open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,app_header,
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением."   );
    return;
  }

  QByteArray req;
  QByteArray ans;
  req.resize(6);
  ans.resize(128+6);
  req[0] = int_module_node();
  req[1] = 0x41;
  req[2] = int_module_subnode() << 4;
  req[3] = 0;
  req[4] = 0;
  req[5] = 128;
  CRC::appendCRC16( req );

  i=5;
  while( i-- )
  {
    ret = serialport.query( req, ans );
    if( ret != (128+6) )  continue;
    if( CRC::CRC16(ans) ) continue;
    break;
  }

  qApp->restoreOverrideCursor();

  if( ret==0 )
  {  QMessageBox::information( this, app_header, "Нет связи с модулем." );
     return;
  }
  if( ret!=(128+6) )
  {  QMessageBox::information( this, app_header, "Ошибка связи с модулем: неверная длина ответа." );
     return;
  }
  if( CRC::CRC16(ans) )
  {  QMessageBox::information( this, app_header, "Ошибка связи с модулем: ошибка CRC." );
     return;
  }

  ModuleInfoDialog dialog( this, ans.right(128+2) );
  dialog.exec();
}

//=======================================================================================
//
//=======================================================================================
QString InitDialog::filename()
{
  return cb_modulename->itemData( cb_modulename->currentIndex() ).toString();
}

//=======================================================================================
//
//=======================================================================================
QString InitDialog::portname()
{
  return cb_portname->itemData( cb_portname->currentIndex() ).toString();
}

//=======================================================================================
//
//=======================================================================================
int InitDialog::portspeed()
{
  return cb_portspeed->currentText().toInt();
}

//=======================================================================================
//
//=======================================================================================
QString InitDialog::module_node()
{
  return le_node->text();
}

//=======================================================================================
//
//=======================================================================================
QString InitDialog::module_name()
{
  return cb_modulename->currentText();
}

//=======================================================================================
//
//=======================================================================================
QString InitDialog::host() const
{
  return le_host->text();
}

//=======================================================================================
//
//=======================================================================================
int InitDialog::int_module_node()
{
  QRegExp rx("(\\d+)");
  rx.indexIn( module_node() );
  return rx.cap(1).toInt();
}
//=======================================================================================
//
//=======================================================================================
int InitDialog::int_module_subnode()
{
  QRegExp rx("\\d+\\.(\\d+)");
  rx.indexIn( module_node() );
  return rx.cap(1).toInt();
}

//=======================================================================================
//
//=======================================================================================
void InitDialog::accept()
{
  QSettings settings( QSETTINGS_PARAM );
  settings.setValue( "portname",   cb_portname->itemData( cb_portname->currentIndex() ) );
  settings.setValue( "portspeed",  cb_portspeed->currentText() );
  settings.setValue( "tcphost",    le_host->text() );
  settings.setValue( "modulename", cb_modulename->currentText() );
  settings.setValue( "modulenode", le_node->text() );
  done( QDialog::Accepted );
}

//#######################################################################################
//
//#######################################################################################
MainWindowXml::MainWindowXml( QWidget *parent, const QString &portname, int portspeed ,
                 const QString &host, const QString &xml_filename,int node, int subnode )
  : QMainWindow( parent )
{
  setupUi( this );

  QSettings settings( QSETTINGS_PARAM );
  int i = settings.value("font_size").toInt();
  if( i > 6 )
  { QFont fnt = qApp->font();
    fnt.setPointSize( i );
    qApp->setFont( fnt );
  }

  if( portname == "===TCP===" )
  { port = new MbTcpPort;
    port->setName( host );
  } else
  { port = new SerialPort;
    port->setName( portname );
  }

  port->setSpeed( portspeed );

  if( !port->open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,app_header,
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
  }
  mbmaster.setTransport( port );

  tw = new MKTable;
  tw->setMBMaster( &mbmaster );
  //--------------------------------------------------------------------------------------

  QDomNodeList list;
  QDomDocument doc("MConf");
  QFile file(xml_filename);
  file.open(QIODevice::ReadOnly);
  doc.setContent(&file);
  file.close();

  list = doc.elementsByTagName("Table");
  if( list.count() )
  { QDomDocument doc_table;
    doc_table.appendChild( doc_table.importNode( list.item(0), true ) );
    tw->loadConfiguration( doc_table );
  }
  list = doc.elementsByTagName("MBConfig");
  if( list.count() )
  {
    QDomDocument doc_config;
    doc_config.appendChild( doc_config.importNode( list.item(0), true ) );
    mbmaster.load_configuration( doc_config );
    mbmaster.set_module_node( 1, node, subnode );
    mbmaster.polling_start();
  }
  tw->setMode( MKTable::Polling );

  //------- строка статуса -------------------------------------------------------------

  statusbar = new QStatusBar();
  full_time       = new InfoLabel;
  status_requests = new InfoLabel;
  status_answers  = new InfoLabel;
  status_errors   = new InfoLabel;
  statusbar->addPermanentWidget( full_time       );
  statusbar->addPermanentWidget( status_requests );
  statusbar->addPermanentWidget( status_answers  );
  statusbar->addPermanentWidget( status_errors   );
  setStatusBar( statusbar );

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(group_update()));
  timer->start(200);

 //------- геометрия окна по центру экрана ----------------------------------------------
 setWindowTitle( app_header );
 setCentralWidget( tw );
 QRect rs = QApplication::desktop()->availableGeometry();
 QRect rw;
 rw.setSize( sizeHint() );
 if( rw.height() > ( rs.height() - 70 ) )
 { rw.setHeight(  rs.height() - 50 );
 }
 if( rw.width() > ( rs.width() - 50 ) )
 { rw.setWidth(  rs.width() - 70 );
 }
 rw.moveCenter( rs.center() );
 setGeometry( rw );
}

//==============================================================================
//
//==============================================================================
MainWindowXml::~MainWindowXml()
{
  delete port;
}

//==============================================================================
//
//==============================================================================
void MainWindowXml::on_action_font_increase_activated()
{
  QFont fnt = qApp->font();
  int a = fnt.pointSize();
  a += 1;
  fnt.setPointSize( a );
  qApp->setFont( fnt );
}

//==============================================================================
//
//==============================================================================
void MainWindowXml::on_action_font_decrease_activated()
{
  QFont fnt = qApp->font();
  int a = fnt.pointSize();
  if( a > 8 )  a -= 1;
  fnt.setPointSize( a );
  qApp->setFont( fnt );
}



//==============================================================================
//
//==============================================================================
void MainWindowXml::group_update()
{
  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster.full_time()) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster.request_counter()) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster.answer_counter()) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster.error_counter()) );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindowXml::closeEvent( QCloseEvent *event )
{
  Q_UNUSED( event );

  mbmaster.polling_stop();

  QSettings settings( QSETTINGS_PARAM );
  settings.setValue( "font_size",   qApp->font().pointSize() );
}

//#######################################################################################
//
//#######################################################################################
ModuleInfoDialog::ModuleInfoDialog( QWidget *parent, const QByteArray &ba )
  : QDialog ( parent )
{
  setupUi( this );
  setWindowTitle( "Информация о модуле" );
  MikkonModuleDefinition mmd = MBMasterXML::decodeModuleDefinition( ba );
  le_name      -> setText( mmd.productName );
  le_desc      -> setText( mmd.productDescription );
  le_firmware  -> setText( QString("%1 [%2] %3.%4")
                               .arg( mmd.releaseDate   )
                               .arg( mmd.releaseMadeBy )
                               .arg( mmd.majorVersion  )
                               .arg( mmd.minorVersion  ) );
  le_company   -> setText( mmd.companyName );
  le_copyright -> setText( mmd.legalCopyright );
  le_id        -> setText( QString().sprintf("%4.4X", mmd.id ) );
}
