#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "mbmaster.h"

MBMaster   *mbmaster;
SerialPort *port;

int modules_n;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Прогон AI100";
}

void MKViewPlugin::helpWindow( QWidget *parent )
{
   QDialog d( parent );

   QTextBrowser *tb = new QTextBrowser;
   QVBoxLayout *layout = new QVBoxLayout;
   layout->addWidget(tb);
   layout->setContentsMargins(0,0,0,0);
   d.setLayout(layout);

   tb->setFrameShape( QFrame::NoFrame );
   tb->setHtml( "<p>Здесь будет краткое описание модуля.</p> " );
   d.setWindowTitle(tr("Описание AI100"));
   d.resize(300,300);
   d.exec();
}

void MKViewPlugin::mainWindow( QWidget *parent, const QString &portname, int portspeed, int node, int subnode )
{
  Q_UNUSED( parent );
  Q_UNUSED( node );
  Q_UNUSED( subnode );
 
  mbmaster = new MBMaster;
  port     = new SerialPort;

  port->setName( portname );
  port->setSpeed( portspeed );
  if( !port->open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,"Прогон AI100",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
  }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, 1, 0, "AI100" );
  mbmaster->add_slot( 1, 1, 0xFE, 6, MBDataType::Floats );
  mbmaster->polling_start();

  bool ok;
  modules_n = QInputDialog::getInteger(0, "Прогон AI100",
                                      "Количество модулей:", 1, 1, 100, 1, &ok);
  if(ok)
  {

  MainWindow mainwindow;
  mainwindow.show();
  qApp->exec();

  }

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
  int i;
  QScrollArea *sa = new QScrollArea;
  QWidget *cw = new QWidget;
  QFrame *frame;
  //frame.setFrameShape( QFrame::HLine );

  //cw->setContentsMargins(0,0,0,0);


  QVBoxLayout *layout = new QVBoxLayout;

  for(i=0; i<modules_n; i++ )
  {
    ModuleWidget *mw = new ModuleWidget;
    mw->node = i+1;
    module_widgets << mw;
  }

  i=0;
  ModuleWidget *ptr;
  foreach( ptr, module_widgets )
  { if( i )
    {
      frame = new QFrame;
      frame -> setFrameShape( QFrame::HLine );
      frame ->setLineWidth(3);
      frame -> setFrameShape( QFrame::HLine );
      layout->addWidget( frame );
    }
    layout->addWidget( new QLabel(QString("<h4>Адрес: %1</h4>").arg(ptr->node) ) );
    layout->addWidget( ptr );
    i++;
  }

  cw->setLayout( layout );

  sa->setWidget( cw );
  sa->setWidgetResizable( true );

  setCentralWidget( sa );




/*  setupUi( this );

  tw->setColumnCount(3);
  tw->setRowCount(17);
  tw->setHorizontalHeaderLabels( QStringList() << "Канал" << "Значение" << "Разброс" );
  tw->verticalHeader()->hide();
*/



  setWindowTitle("Прогон AI100");
 //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  //rw.setSize( sizeHint() );
  rw.setSize( QSize( 550,450 ) );
  if( rw.height() > ( rs.height() - 70 ) )
  { rw.setHeight(  rs.height() - 50 );
  }
  if( rw.width() > ( rs.width() - 50 ) )
  { rw.setWidth(  rs.width() - 70 );
  }
  rw.moveCenter( rs.center() );
  setGeometry( rw );
  //--------------------------------------------

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

  startTimer( 100 );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );
  //------------------------------------
  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster->full_time) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter) );
}

//###########################################################################################
//
//###########################################################################################
ModuleWidget::ModuleWidget( QWidget *parent )
  : QWidget( parent )
{
  int i;
  setupUi( this );

  tw->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw->setColumnCount(4);
  tw->setRowCount(6);
  tw->setHorizontalHeaderLabels( QStringList() << "Значение\n(мВ)" << "Максимальный\nразброс (мВ)" << "Кол-во выбросов\nвверх" << "Кол-во выбросов\nвниз");
  tw->setVerticalHeaderLabels( QStringList() << "1"<<"2"<<"3"<<"4"<<"5"<<"6" );
  tw->horizontalHeader()->setClickable( false );
  tw->verticalHeader()->setClickable( false );
  //tw->setMinimumSize( QSize( -1, tw->verticalHeader()->height()+tw->horizontalHeader()->height() ) );
  tw->setMinimumSize( QSize( -1, 170 ) );

  for(i=0; i<6; i++)
  {
    tw->setItem( i,0,new QTableWidgetItem );
  }
  startTimer( 100 );

}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void ModuleWidget::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  int i;
  MMSlot slot = mbmaster->getSlot(1,1);
  QString str;

  for(i=0; i<6; i++)
  { if( i >= slot.data.count() ) break;
    ai_data[i] =  slot.data[i].toDouble();
    str.sprintf("     %3.3lf",ai_data[i] );
    tw->item(i,0)->setText( str );
  }
  le_diag->setStyleSheet( "background: palegreen" );
  le_diag->setText("все нормально");

  le_time ->setText( QString("%1 часов").arg((sec/3600.0),3,'g',3) );
  sec += 0.1;

  le_err->setText("0");
  le_resets->setText("0");
}

