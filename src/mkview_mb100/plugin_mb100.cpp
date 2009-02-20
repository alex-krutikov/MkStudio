#include <QtGui>
#include <QtCore>
#include <QtXml>

#include "plugin.h"
#include "serialport.h"
#include "mbmasterxml.h"
#include "crc.h"

MBMasterXML   *mbmaster;
SerialPort *port;

int modules_n;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Стенд для проверки MB100";
}

void MKViewPlugin::helpWindow( QWidget *parent )
{
   QDialog d( parent );

   QTextBrowser *tb = new QTextBrowser;
   QVBoxLayout *layout = new QVBoxLayout;
   layout->addWidget(tb);
   layout->setContentsMargins(0,0,0,0);
   d.setLayout(layout);

   QFont f = tb->font();
   f.setPixelSize( 16 );
   tb->setFont( f );
   tb->setFrameShape( QFrame::NoFrame );
   tb->setSource( QUrl::fromLocalFile(":/html/html/index.html" ) );
   d.setWindowTitle("Стенд для проверки MB100");
   d.resize(300,300);
   d.exec();
}

void MKViewPlugin::mainWindow( QWidget *parent, const QString &portname, int portspeed, int node, int subnode )
{
  Q_UNUSED( parent );
  Q_UNUSED( node );
  Q_UNUSED( subnode );

  mbmaster = new MBMasterXML;
  port     = new SerialPort;

  port->setName( portname );
  port->setSpeed( portspeed );
  if( !port->open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,"Стенд для проверки MB100",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
 }
  mbmaster->setTransport( port );
/*
  mbmaster->add_module( 1, 1, 0, "MB1730" );
  mbmaster->add_slot( 1, 1, 0x0a,  8, MBDataType::Bits );  // Сервисный режим
  mbmaster->add_module( 2, 2, 0, "MB1730" );
  mbmaster->add_slot( 2, 1, 0x0a,  8, MBDataType::Bits );  // Сервисный режим
  mbmaster->polling_start();
*/
/*
  mbmaster->add_module( 1, 1, 0, "MB1730" );
  mbmaster->add_slot( 1, 1, 0x0652,  5, MBDataType::Dwords );  // Сервисный режим
  mbmaster->add_slot( 1, 2, 0x0032,  2, MBDataType::Floats );  // Значения входов (мВ)
  mbmaster->add_slot( 1, 3, 0x01CE,  2, MBDataType::Dwords );  // Коды АЦП
  mbmaster->add_slot( 1,11, 0x067A, 68, MBDataType::Bytes  );  // Окно для записи калибровки
  mbmaster->polling_start();
*/



  MainWindow mainwindow;
  mainwindow.show();
  qApp->exec();

  delete mbmaster;
  delete port;
}

Q_EXPORT_PLUGIN2(EchoInterface, MKViewPlugin);

//###############################################################################################
//
//###############################################################################################
MainWindow::MainWindow( QWidget *parent  )
  : QMainWindow( parent )
{
  setupUi( this );
  setWindowTitle("Стенд для проверки MB100");
 //-------- по центру экрана -------------------
/*
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
*/

 tw->setMBMaster( mbmaster );

 QDomNodeList list;
 QDomDocument doc("MKStudio");
 QFile file(":/res/res/conf.xml");
 if (!file.open(QIODevice::ReadOnly))  return;
 if(!doc.setContent(&file))
 {   file.close();
     return;
 }
 file.close();
 if( doc.doctype().name() != "MKStudio" )
 { QMessageBox::information( this, "weqwE", "Файл не является конфигурацией MKStudio.");
   return;
 }
 list = doc.elementsByTagName("Table");
 if( list.size() )
 { QDomDocument doc_table;
   doc_table.appendChild( doc_table.importNode( list.item(0), true ) );
   tw->loadConfiguration( doc_table );
 }

 list = doc.elementsByTagName("MBConfig");
 if( list.size() )
 { QDomDocument doc_config;
   doc_config.appendChild( doc_config.importNode( list.item(0), true ) );
   mbmaster->load_configuration( doc_config );
 } else
 { QMessageBox::information( this, "weqwE", "Файл не является конфигурацией MKStudiosdfsdf.");
 }

 mbmaster->polling_start();
 tw->setMode( MKTable::Polling );

/*
  tw_ai->setColumnCount(2);
  tw_ai->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw_ai->setHorizontalHeaderLabels( QStringList() << "Значение" << "Код АЦП" );
  tw_ai->horizontalHeader()->resizeSection( 0 , 75 );
  tw_ai->horizontalHeader()->resizeSection( 1 , 75 );
  //tw->horizontalHeader()->setStretchLastSection( true );
  //tw->setSelectionBehavior( QAbstractItemView::SelectRows );
  tw_ai->setSelectionMode( QAbstractItemView::NoSelection );
  tw_ai->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw_ai->setRowCount(3);
  tw_ai->setVerticalHeaderLabels( QStringList() << "Канал 1" << "Канал 2" << "Канал 3" );

*/
/*

  tw_ai->setColumnCount(2);
  tw_ai->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw_ai->setHorizontalHeaderLabels( QStringList() << "Значение" << "Код АЦП" );
  tw_ai->horizontalHeader()->resizeSection( 0 , 75 );
  tw_ai->horizontalHeader()->resizeSection( 1 , 75 );
  //tw->horizontalHeader()->setStretchLastSection( true );
  //tw->setSelectionBehavior( QAbstractItemView::SelectRows );
  tw_ai->setSelectionMode( QAbstractItemView::NoSelection );
  tw_ai->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw_ai->setRowCount(3);
  tw_ai->setVerticalHeaderLabels( QStringList() << "Канал 1" << "Канал 2" << "Канал 3" );

  for(i=0;i<tw_ai->rowCount(); i++ )
  { for(j=0;j<tw_ai->columnCount(); j++ )
    { QTableWidgetItem *titem = new QTableWidgetItem;
      tw_ai->setItem(i,j,titem);
    }
  }

  tw_do->setColumnCount(2);
  tw_do->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw_do->setHorizontalHeaderLabels( QStringList() << "Управление" << "Контроль" );
  tw_do->horizontalHeader()->resizeSection( 0 , 100 );
  tw_do->horizontalHeader()->resizeSection( 1 , 100 );
  //tw_do->horizontalHeader()->setStretchLastSection( true );
  //tw_do->setSelectionBehavior( QAbstractItemView::SelectRows );
  tw_do->setSelectionMode( QAbstractItemView::NoSelection );
  tw_do->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw_do->setRowCount(4);
  tw_do->setVerticalHeaderLabels( QStringList() << "Канал 1" << "Канал 2" << "Канал 3" << "Канал 4" );

  for(i=0;i<tw_do->rowCount(); i++ )
  { for(j=0;j<tw_do->columnCount(); j++ )
    { QTableWidgetItem *titem = new QTableWidgetItem;
      tw_do->setItem(i,j,titem);
    }
  }

  tw_do->setCellWidget( 0,0, new QPushButton("ВКЛ",this) );
  tw_do->setCellWidget( 1,0, new QPushButton("ВКЛ",this) );
  tw_do->setCellWidget( 2,0, new QPushButton("ВКЛ",this) );
  tw_do->setCellWidget( 3,0, new QPushButton("ВКЛ",this) );

*/
  statusbar = new QStatusBar();
  full_time       = new QLabel;
  status_requests = new QLabel;
  status_answers  = new QLabel;
  status_errors   = new QLabel;
  statusbar->addPermanentWidget( full_time       );
  statusbar->addPermanentWidget( status_requests );
  statusbar->addPermanentWidget( status_answers  );
  statusbar->addPermanentWidget( status_errors   );
  setStatusBar( statusbar );

  startTimer( 50 );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  mbmaster->setSlotValue(2,1,8,1);
  mbmaster->setSlotValue(2,1,9,0);
  mbmaster->setSlotValue(2,1,10,0);
  mbmaster->setSlotValue(2,1,11,0);
  mbmaster->setSlotValue(2,1,12,0);

  mbmaster->setSlotValue(1,4,0,0);
  mbmaster->setSlotValue(1,4,1,0);
  mbmaster->setSlotValue(1,4,2,0);
  mbmaster->setSlotValue(1,4,3,0);

  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster->full_time()) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter()) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter()) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter()) );
}


//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::closeEvent( QCloseEvent *event )
{
  Q_UNUSED( event );
}
