#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "mbmaster.h"

MBMaster   *mbmaster;
SerialPort *port;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Стенд для проверки DI102";
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
   tb->setHtml( "<p>Адрес 1 - проверяемый DI102, адрес 2 - модуль DIO100.</p> " );
   d.setWindowTitle(tr("Стенд для проверки DI102"));
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
    QMessageBox::information(0,"Стенд для проверки DI102",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
  }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, 2, 0, "DIO100 REF" );
  mbmaster->add_slot( 1, 1, 0x0A, 8, MBDataType::Bits ); // выходы
  mbmaster->add_slot( 1, 2, 0x0B, 8, MBDataType::Bits ); // режимы

  mbmaster->add_module( 2, 1, 0, "DI102" );
  mbmaster->add_slot( 2, 1, 0x0006, 16, MBDataType::Bits   ); // входы
  mbmaster->add_slot( 2, 2, 0x0052, 16, MBDataType::Floats ); // частота
  mbmaster->add_slot( 2, 3, 0xC0C0, 16, MBDataType::Dwords ); // системная статистика
  mbmaster->add_slot( 2, 4, 0x0012, 16, MBDataType::Dwords ); // счетчики
  mbmaster->add_slot( 2, 5, 0x0002, 1,  MBDataType::Bytes  ); // очистка счетчиков

  mbmaster->polling_start();

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
  int i,j;
  setupUi( this );
  setWindowTitle("Стенд для проверки DI102");
  run_flag = true;
  pb->setText("Стоп");
 //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
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
  tw->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw->setColumnCount(4);
  tw->setRowCount(16);
  tw->setHorizontalHeaderLabels( QStringList() << "Состояние\nвхода"
                                               << "Счетчики"
                                               << "Частота\n(Гц)"
                                               << "Системная\nстатистика" );
  tw->horizontalHeader()->resizeSection( 0 , 80 );
  tw->horizontalHeader()->resizeSection( 0 , 80 );
  tw->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw->setSelectionMode( QAbstractItemView::NoSelection );
  tw->setSelectionMode( QAbstractItemView::NoSelection );
  tw->setFocusPolicy( Qt::NoFocus );
  QTableWidgetItem ttitem;
  //--------------------------------------------
  ttitem.setTextAlignment( Qt::AlignCenter );
  for(i=0; i<16; i++)
  { for(j=0; j<4; j++)
    {  tw->setItem(i,j,new QTableWidgetItem(ttitem) );
    }
  }
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
  //--------------------------------------------
  startTimer( 100 );
}

//-------------------------------------------------------------------------------------------
// Таймер
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  int i;

  static int counter;

  if( run_flag ) counter++;
  if( counter == 5 )
  { // режимы
    mbmaster->setSlotValue(1,2,0,1);
    mbmaster->setSlotValue(1,2,1,1);
    mbmaster->setSlotValue(1,2,2,1);
    mbmaster->setSlotValue(1,2,3,1);
    mbmaster->setSlotValue(1,2,4,1);
    mbmaster->setSlotValue(1,2,5,1);
    // выходы
    mbmaster->setSlotValue(1,1,0,1);
    mbmaster->setSlotValue(1,1,1,0);
    mbmaster->setSlotValue(1,1,2,1);
    mbmaster->setSlotValue(1,1,3,0);
    mbmaster->setSlotValue(1,1,4,1);
    mbmaster->setSlotValue(1,1,5,0);
  }
  if( counter >= 10 )
  { counter = 0;
    // выходы
    mbmaster->setSlotValue(1,1,0,0);
    mbmaster->setSlotValue(1,1,1,1);
    mbmaster->setSlotValue(1,1,2,0);
    mbmaster->setSlotValue(1,1,3,1);
    mbmaster->setSlotValue(1,1,4,0);
    mbmaster->setSlotValue(1,1,5,1);
  }


  MMSlot slot;

  //- ВХОДЫ ------------------------------------
  slot = mbmaster->getSlot(2,1);
  for(i=0; i<11; i++)
  {  tw->item(i,0)->setText( slot.data[i].toString() );

     if( tw->item(i,0)->text() == "1" )
     { tw->item(i,0)->setBackgroundColor( QColor("springgreen") );
     }
     if( tw->item(i,0)->text() == "0" )
     { tw->item(i,0)->setBackgroundColor( QColor("seagreen") );
     }

     if( slot.data[i].status() != MMValue::Ok )
     { tw->item(i,0)->setText("***");
       tw->item(i,0)->setBackgroundColor( QColor("white") );
     }
  }
  //---- Перегрузка --------------------------
  if( slot.data[15].status() != MMValue::Ok )
  {
    label->setText("***");
    label->setStyleSheet("");
  }
  else if( slot.data[15].toInt() )
  {
    label->setText("<h3>ПЕРЕГРУЗКА</h3>");
    label->setStyleSheet("background:red");
  } else
  {
    label->setText("Перегрузки нет");
    label->setStyleSheet("");
  }
  //-  Частота  ------------------------------------
  slot = mbmaster->getSlot(2,2);
  for(i=0; i<11; i++)
  {  tw->item(i,2)->setText( slot.data[i].toString() );
     if( slot.data[i].status() != MMValue::Ok )
     { tw->item(i,2)->setText("***");
     }
  }
  //---- Системная статистика -----------------------
  slot = mbmaster->getSlot(2,3);
  for(i=0; i<16; i++)
  {  tw->item(i,3)->setText( slot.data[i].toString() );
     if( slot.data[i].status() != MMValue::Ok )
     { tw->item(i,3)->setText("***");
     }
  }
  //---- Счетчики -----------------------------------
  slot = mbmaster->getSlot(2,4);
  for(i=0; i<11; i++)
  {  tw->item(i,1)->setText( slot.data[i].toString() );
     if( slot.data[i].status() != MMValue::Ok )
     { tw->item(i,1)->setText("***");
     }
  }


  //------------------------------------
  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster->full_time) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter) );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_clicked()
{
  if( run_flag )
  { run_flag = false;
    pb->setText( "Пуск" );
  } else
  { run_flag = true;
    pb->setText( "Стоп" );
  }
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::on_pb_counters_reset_clicked()
{
  mbmaster->setSlotValue(2,5,0,1);
}

