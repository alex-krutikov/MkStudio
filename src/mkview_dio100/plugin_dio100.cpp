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
    return "Стенд для проверки DIO100";
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
   tb->setHtml( "<p>Два DIO100, соединенные друг с другом через специальную приладу.</p> " );
   d.setWindowTitle(tr("Стенд для проверки DIO100"));
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
    QMessageBox::information(0,"Стенд для проверки DI100",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
  }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, 2, 0, "DI100 REF" );
  mbmaster->add_slot( 1, 1, 0x06, 8, MBDataType::Bits ); // входы
  mbmaster->add_slot( 1, 2, 0x0A, 8, MBDataType::Bits ); // выходы
  mbmaster->add_slot( 1, 3, 0x0B, 8, MBDataType::Bits ); // режимы

  mbmaster->add_module( 2, 1, 0, "DI100 TST" );
  mbmaster->add_slot( 2, 1, 0x06, 8, MBDataType::Bits ); // входы
  mbmaster->add_slot( 2, 2, 0x0A, 8, MBDataType::Bits ); // выходы
  mbmaster->add_slot( 2, 3, 0x0B, 8, MBDataType::Bits ); // режимы


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
  setWindowTitle("Стенд для проверки DIO100");
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
  tw_ref->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw_ref->setColumnCount(3);
  tw_ref->setRowCount(6);
  tw_ref->setHorizontalHeaderLabels( QStringList() << "Режим" << "Вход" << "Выход");
  tw_ref->horizontalHeader()->resizeSection( 0 , 70 );
  tw_ref->horizontalHeader()->resizeSection( 1 , 60 );
  tw_ref->horizontalHeader()->resizeSection( 2 , 60 );
  tw_ref->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw_ref->setSelectionMode( QAbstractItemView::NoSelection );
  tw_ref->setSelectionMode( QAbstractItemView::NoSelection );
  tw_ref->setFocusPolicy( Qt::NoFocus );
  //--------------------------------------------
  tw_tst->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw_tst->setColumnCount(3);
  tw_tst->setRowCount(6);
  tw_tst->setHorizontalHeaderLabels( QStringList() << "Режим" << "Вход" << "Выход");
  tw_tst->horizontalHeader()->resizeSection( 0 , 70 );
  tw_tst->horizontalHeader()->resizeSection( 1 , 60 );
  tw_tst->horizontalHeader()->resizeSection( 2 , 60 );
  tw_tst->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw_tst->setSelectionMode( QAbstractItemView::NoSelection );
  tw_tst->setFocusPolicy( Qt::NoFocus );
  //--------------------------------------------
  QTableWidgetItem ttitem;
  ttitem.setTextAlignment( Qt::AlignCenter );
  for(i=0; i<6; i++)
  { for(j=0; j<3; j++)
    {  tw_ref->setItem(i,j,new QTableWidgetItem(ttitem) );
       tw_tst->setItem(i,j,new QTableWidgetItem(ttitem) );
    }
  }
  //--------------------------------------------
  tw_test->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw_test->setColumnCount(2);
  tw_test->setRowCount(6);
  tw_test->setHorizontalHeaderLabels( QStringList() << "Вход" << "Выход");
  tw_test->setVerticalHeaderLabels( QStringList() << " канал 1" << " канал 2" << " канал 3"
                                                  << " канал 4" << " канал 5" << " канал 6" );
  tw_test->horizontalHeader()->resizeSection( 0 , 100 );
  tw_test->horizontalHeader()->resizeSection( 1 , 100 );
  tw_test->setEditTriggers( QAbstractItemView::NoEditTriggers );
  tw_test->setSelectionMode( QAbstractItemView::NoSelection );
  tw_test->setFocusPolicy( Qt::NoFocus );
  //--------------------------------------------
  for(i=0; i<6; i++)
  { for(j=0; j<2; j++)
    {  tw_test->setItem(i,j,new QTableWidgetItem(ttitem) );
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
  startTimer( 750 );
}

//-------------------------------------------------------------------------------------------
// Нажатие кнопки
//-------------------------------------------------------------------------------------------
void MainWindow::button_clicked(int id)
{
  Q_UNUSED( id );
}

//-------------------------------------------------------------------------------------------
// Таймер
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  static int state;
  static int channel;
  int flag;
  static int static_add         = 0 ;
  static int static_module_tst  = 2;
  static int static_module_ref  = 1;

  static QColor color_red("pink");
  static QColor color_green("lightgreen");
  static QColor color_gray("lightgray");
  static QColor color_white("white");


  int i;

  //----- индикация состояния модулей-------------------------------------------------
  MMSlot slot;
  //- ВХОДЫ REF---------------------------------
  slot = mbmaster->getSlot(1,1);
  for(i=0; i<6; i++)
  {  tw_ref->item(i,1)->setText( slot.data[i].toString() );
     if( slot.data[i].status() != MMValue::Ok )
     { tw_ref->item(i,1)->setText("***");
     } else if( slot.data[i].toInt() )
     { tw_ref->item(i,1)->setBackgroundColor( color_gray );
     } else
     { tw_ref->item(i,1)->setBackgroundColor( color_white );
     }
  }
  //- ВЫХОДЫ REF ---------------------------------
  slot = mbmaster->getSlot(1,2);
  for(i=0; i<6; i++)
  {  tw_ref->item(i,2)->setText( slot.data[i].toString() );
     if( slot.data[i].status() != MMValue::Ok )
     { tw_ref->item(i,2)->setText("***");
     } else if( slot.data[i].toInt() )
     { tw_ref->item(i,2)->setBackgroundColor( color_gray );
     } else
     { tw_ref->item(i,2)->setBackgroundColor( color_white );
     }
  }
  //- РЕЖИМЫ REF ---------------------------------
  slot = mbmaster->getSlot(1,3);
  for(i=0; i<6; i++)
  {  tw_ref->item(i,0)->setText( "вход" );
     if( slot.data[i].toInt() == 1 ) tw_ref->item(i,0)->setText("выход");
     if( slot.data[i].status() != MMValue::Ok ) tw_ref->item(i,0)->setText("***");
  }
  //- ВХОДЫ TST ---------------------------------
  slot = mbmaster->getSlot(2,1);
  for(i=0; i<6; i++)
  {  tw_tst->item(i,1)->setText( slot.data[i].toString() );
     if( slot.data[i].toInt() )
     { tw_tst->item(i,1)->setBackgroundColor( color_gray );
     } else
     { tw_tst->item(i,1)->setBackgroundColor( color_white );
     }
     if( slot.data[i].status() != MMValue::Ok )
     { tw_tst->item(i,1)->setText("***");
       tw_test->item(i,0)->setText("нет связи");
       tw_test->item(i,0)->setBackgroundColor( color_red );
     } else if( tw_test->item(i,0)->text() == "нет связи" )
     { tw_test->item(i,0)->setText( QString() );
     }

  }
  //- ВЫХОДЫ TST ---------------------------------
  slot = mbmaster->getSlot(2,2);
  for(i=0; i<6; i++)
  {  tw_tst->item(i,2)->setText( slot.data[i].toString() );
     if( slot.data[i].toInt() )
     { tw_tst->item(i,2)->setBackgroundColor( color_gray );
     } else
     { tw_tst->item(i,2)->setBackgroundColor( color_white );
     }
     if( slot.data[i].status() != MMValue::Ok )
     { tw_tst->item(i,2)->setText("***");
       tw_test->item(i,1)->setText("нет связи");
       tw_test->item(i,1)->setBackgroundColor( color_red );
     } else if( tw_test->item(i,1)->text() == "нет связи" )
     { tw_test->item(i,1)->setText( QString() );
     }
  }
  //- РЕЖИМЫ TST ---------------------------------
  slot = mbmaster->getSlot(2,3);
  for(i=0; i<6; i++)
  {  tw_tst->item(i,0)->setText( "вход" );
     if( slot.data[i].toInt() == 1 ) tw_tst->item(i,0)->setText("выход");
     if( slot.data[i].status() != MMValue::Ok ) tw_tst->item(i,0)->setText("***");
  }

  switch( state )
  { case(0):
      for( i=0; i<6; i++ )
      { mbmaster->setSlotValue( static_module_ref,3,i,1 ); // как выход
        mbmaster->setSlotValue( static_module_ref,2,i,0 ); // выход = 0
        mbmaster->setSlotValue( static_module_tst,3,i,0 ); // как вход
        mbmaster->setSlotValue( static_module_tst,2,i,0 ); // выход = 0
        mbmaster->setSlotValue( static_module_ref,2,i, (channel==i)?(1):(0) ); // поджиг выхода на контрольном
      }
      state++;
      break;
    case(1):
      // анализ
      slot = mbmaster->getSlot(static_module_tst,1);
      flag=0;
      for( i=0; i<6; i++ )
      { if( slot.data[i].status() != MMValue::Ok ) continue;
        if(i==channel)
        { if(slot.data[i].toString() == "1" )
          {  tw_test->item(i,static_add)->setText("исправен");
             tw_test->item(i,static_add)->setBackgroundColor( color_green );
          } else
          {  tw_test->item(i,static_add)->setText("обрыв");
             tw_test->item(i,static_add)->setBackgroundColor( color_red );
          }
        } else
        { if(slot.data[i].toString() == "1" )
          {  tw_test->item(i,static_add)->setText("КЗ");
             tw_test->item(i,static_add)->setBackgroundColor( color_red );
          }

        }
      }
      channel++;
      if( channel >= 6 )
      { channel = 0;
        if( static_module_tst == 2 )
        { static_add        = 1;
          static_module_tst = 1;
          static_module_ref = 2;
        } else
        { static_add        = 0;
          static_module_tst = 2;
          static_module_ref = 1;
        }
       }
       state=0;
       break;
  }

  //------------------------------------
  full_time       -> setText( QString(" Время опроса: %1 мс ").arg(mbmaster->full_time) );
  status_requests -> setText( QString(" Запросы: %1 ").arg(mbmaster->request_counter) );
  status_answers  -> setText( QString(" Ответы:  %1 ").arg(mbmaster->answer_counter) );
  status_errors   -> setText( QString(" Ошибки:  %1 ").arg(mbmaster->error_counter) );
}
