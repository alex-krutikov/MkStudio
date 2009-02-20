#include <QtGui>

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>

#include "plugin.h"

#include "mbmasterxml.h"
#include "serialport.h"
#include "crc.h"

SerialPort *port;
MBMasterXML   *mbmaster;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Калибровка mAI100 (AI100)";
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
   tb->setHtml( "<p>Калибровка мезонина АЦП mAI100 при помощи модуля AI00.</p> " );
   d.setWindowTitle(tr("Описание"));
   d.resize(300,300);
   d.exec();
}

void MKViewPlugin::mainWindow( QWidget *parent, const QString &portname, int portspeed, int node, int subnode )
{
  Q_UNUSED( parent );
  Q_UNUSED( node );
  Q_UNUSED( subnode );

   port     = new SerialPort;
   mbmaster = new MBMasterXML;

   port->setName( portname );
   port->setSpeed( portspeed );
   if( !port->open() )
   { qApp->restoreOverrideCursor();
     QMessageBox::information(0,"Калибровка mAI100 (AI100)",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
   }
   mbmaster->setTransport( port );

   mbmaster->add_module( 1, 1, 0, "AI100" );
   mbmaster->add_slot( 1, 1, 0x0B0E, 32, MBDataType::Bytes  );
   mbmaster->add_slot( 1, 2, 0x0B2E, 1,  MBDataType::Dwords );
   mbmaster->polling_start();


   MainWindow mainwindow;
   mainwindow.show();
   qApp->exec();

   delete mbmaster;
   delete port;
}

Q_EXPORT_PLUGIN2(EchoInterface, MKViewPlugin);

//########################################################################################

//==============================================================================
// Главное окно
//==============================================================================
MainWindow::MainWindow()
{
  setupUi( this );

  app_header = "Калибровка mAI100";

  setWindowTitle( app_header );

  //-------- по центру экрана -------------------
  QRect rs = QApplication::desktop()->availableGeometry();
  QRect rw;
  //rw.setSize( QSize(750,550 ));
  rw.setSize( sizeHint() );
  if( rw.height() > ( rs.height() - 70 ) )
  { rw.setHeight(  rs.height() - 50 );
  }
  if( rw.width() > ( rs.width() - 50 ) )
  { rw.setWidth(  rs.width() - 70 );
  }
  rw.moveCenter( rs.center() );
  setGeometry( rw );
  //--------------------------------------------

  write_automat_state=0;

  startTimer( 100 );
/*
  QStringList sl;
  sl << "Напряжение (мВ)"
     << "Разброс (мВ)"
     << "Код АЦП";

  tw_inputs->verticalHeader()->setDefaultSectionSize(font().pointSize()+16);
  tw_inputs->setSelectionMode( QAbstractItemView::NoSelection );
  tw_inputs->setColumnCount(3);
  tw_inputs->setRowCount(17);
  tw_inputs->setHorizontalHeaderLabels( sl );
  tw_inputs->horizontalHeader()->resizeSection(  0 , 100 );
  tw_inputs->horizontalHeader()->resizeSection(  1 , 120 );
  QTableWidgetItem titem;
  titem.setTextAlignment( Qt::AlignRight|Qt::AlignVCenter );
  for( i=0; i< tw_inputs->rowCount(); i++ )
  {
    tw_inputs->setItem( i,0, new QTableWidgetItem(titem) );
    tw_inputs->setItem( i,1, new QTableWidgetItem(titem) );
  }
*/
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_pb_write_clicked()
{
  write_automat_state = 1;
  pb_write->setEnabled( false );
}

//==============================================================================
//
//==============================================================================
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  static TEEPROM calib_eeprom;
  int i;

  switch( write_automat_state )
  {
    case(0):
      break;
    case(1):
      calib_eeprom.c_adc_a     = le_adc_a->text().toDouble();
      calib_eeprom.c_adc_b     = le_adc_b->text().toDouble();
      calib_eeprom.c_dac_1mA   = le_dac_1mA->text().toDouble();
      calib_eeprom.c_dac_5mA   = le_dac_5mA->text().toDouble();
      calib_eeprom.c_impedance = le_impedance->text().toDouble();
      for( i=0; i<32; i++ )
      { if( i >= (int)sizeof( calib_eeprom ) ) break;
        mbmaster->setSlotValue(1,1,i, *(((unsigned char*)&calib_eeprom)+i) );
      }
      mbmaster->setSlotValue(1,2,0, 123 );
      write_automat_state++;
      break;
    case(2):
      pb_write->setEnabled( true );
      write_automat_state=0;
      break;

  }
}

