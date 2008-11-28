#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "crc.h"
#include "consolewidget.h"

SerialPort     *port;
TestThread     *testthread;

volatile bool status_1_115200;
volatile bool status_42_115200;
volatile bool status_85_9600;

volatile int counter_1_115200;
volatile int counter_42_115200;
volatile int counter_85_9600;

volatile char therad_exit_flag;


QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Тест переключателя адреса";
}

void MKViewPlugin::helpWindow( QWidget *parent )
{
  const char *html_help =
  "<p>Тест осуществляет опрос модуля по трем вариантам: "
  "<ul>"
  "<li> адрес 1, скорость 115200,</li>"
  "<li> адрес 42, скорость 115200,</li>"
  "<li> адрес 85, скорость 9600.</li>"
  "</ul>"
  "При наличии корректных ответов соответствующая строчка "
  "окрашивается зеленым цветом.</p>"
  "<p>Используется запрос на чтение из базы одного байта по адресу 2.</p>";

   QDialog d( parent );

   QTextBrowser *tb = new QTextBrowser;
   QVBoxLayout *layout = new QVBoxLayout;
   layout->addWidget(tb);
   layout->setContentsMargins(0,0,0,0);
   d.setLayout(layout);

   tb->setFrameShape( QFrame::NoFrame );
   tb->setHtml( html_help );
   d.setWindowTitle("Тест переключателя адреса");
   d.exec();
}

void MKViewPlugin::mainWindow( QWidget *parent, const QString &portname, int portspeed, int node, int subnode )
{
  Q_UNUSED( parent );
  Q_UNUSED( node );
  Q_UNUSED( subnode );

  port         = new SerialPort;
  testthread   = new TestThread;

  port->setName(  portname );
  port->setSpeed( portspeed );
  if( !port->open() )
  { qApp->restoreOverrideCursor();
    QMessageBox::information(0,"Тест переключателя адреса",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
  }

  therad_exit_flag=false;
  testthread->start();

  MainWindow mainwindow;
  mainwindow.show();
  qApp->exec();

  therad_exit_flag=true;
  testthread->wait(5000);

  delete testthread;
  delete port;
}

Q_EXPORT_PLUGIN2(EchoInterface, MKViewPlugin);

//###############################################################################################
//
//###############################################################################################
MainWindow::MainWindow( QWidget *parent  )
  : QWidget( parent )
{
  setWindowTitle("Тест переключателя адреса");

  l1 = new QLabel("-----");
  l2 = new QLabel("-----");
  l3 = new QLabel("-----");

  QFrame *line1 =  new QFrame;
  line1 -> setFrameShape(QFrame::HLine);
  line1 -> setFrameShadow(QFrame::Sunken);

  QFrame *line2 =  new QFrame;
  line2 -> setFrameShape(QFrame::HLine);
  line2 -> setFrameShadow(QFrame::Sunken);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(l1);
  layout->addWidget(line1);
  layout->addWidget(l2);
  layout->addWidget(line2);
  layout->addWidget(l3);
  setLayout( layout );


 //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  rw.setSize( QSize( 300,180 ) );
  //rw.setSize( sizeHint() );
  rw.moveCenter( rs.center() );
  setGeometry( rw );
  //--------------------------------------------
  startTimer( 10 );

  //ConsoleWidget *cw = new ConsoleWidget;
  //cw->show();
  //global_colsole = cw;
}

//-------------------------------------------------------------------------------------------
// Таймер
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  static int f1,f2,f3;
  static int c1,c2,c3;

  const static char *char_table[4] = { "-","\\","|","/" };


  if(( !status_1_115200 ) && (f1 !=1 ) )
  { f1 = 1;
    l1->setStyleSheet("background : rgb(255,70,70)" );
    l1->setText( "  Адрес 1, скорость 115200:  Нет связи. " );
  }
  if(( status_1_115200 ) && (f1 !=2 ) )
  { f1 = 2;
    l1->setStyleSheet("background : lightgreen" );
  }
  if( c1 != counter_1_115200 )
  { c1 = counter_1_115200;
    l1->setText( "  Адрес 1, скорость 115200:  OK. ( " + QString(char_table[c1&3]) + " ) " );
  }

  if(( !status_42_115200 ) && (f2 !=1 ) )
  { f2 = 1;
    l2->setStyleSheet("background : rgb(255,70,70)" );
    l2->setText( "  Адрес 42, скорость 115200:  Нет связи. " );
  }
  if(( status_42_115200 ) && (f2 !=2 ) )
  { f2 = 2;
    l2->setStyleSheet("background : lightgreen" );
  }
  if( c2 != counter_42_115200 )
  { c2 = counter_42_115200;
    l2->setText( "  Адрес 42, скорость 115200:  OK. ( " + QString(char_table[c2&3]) + " ) " );
  }


  if(( !status_85_9600 ) && (f3 !=1 ) )
  { f3 = 1;
    l3->setStyleSheet("background : rgb(255,70,70)" );
    l3->setText( "  Адрес 85, скорость 9600:  Нет связи. " );
  }
  if(( status_85_9600 ) && (f3 !=2 ) )
  { f3 = 2;
    l3->setStyleSheet("background : lightgreen" );
  }
  if( c3 != counter_85_9600 )
  { c3 = counter_85_9600;
    l3->setText( "  Адрес 85, скорость 9600:  OK. ( " + QString(char_table[c3&3]) + " ) " );
  }

}

//###############################################################################################
//
//###############################################################################################
TestThread::TestThread( QObject *parent )
  : QThread( parent )
{
  status_1_115200    = false;
  status_42_115200   = false;
  status_85_9600     = false;
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void TestThread::run()
{
  int i;
  QByteArray req;
  QByteArray ans;

  req.resize(6);
  req[0] = 0x01;
  req[1] = 0x43;
  req[2] = 0x00;
  req[3] = 0x00;
  req[4] = 0x02;
  req[5] = 0x01;
  CRC::appendCRC16( req );

  ans.resize(7);

  while( !therad_exit_flag )
  {

    // адрес 1 скорость 115200
    port->setSpeed( 115200 );
    port->open();

    req.resize(6);
    req[0] = 0x01;
    CRC::appendCRC16( req );
    i = port->request( req, ans );
    if( ( i == 7 ) && ( CRC::CRC16(ans) == 0 ) )
    { status_1_115200 = true;
      counter_1_115200++;
    } else
    { status_1_115200 = false;
    }

    // адрес 42 скорость 115200
    req.resize(6);
    req[0] = 0x2A;
    CRC::appendCRC16( req );
    i = port->request( req, ans );
    if( ( i == 7 ) && ( CRC::CRC16(ans) == 0 ) )
    { status_42_115200 = true;
      counter_42_115200++;
    } else
    { status_42_115200 = false;
    }

    // адрес 85 скорость 9600
    port->setSpeed( 9600 );
    port->open();

    req.resize(6);
    req[0] = 0x55;
    CRC::appendCRC16( req );
    i = port->request( req, ans );
    if( ( i == 7 ) && ( CRC::CRC16(ans) == 0 ) )
    { status_85_9600 = true;
      counter_85_9600++;
    } else
    { status_85_9600 = false;
    }
  }
}
