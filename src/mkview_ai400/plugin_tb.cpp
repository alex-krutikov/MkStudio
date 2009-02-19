#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "mbmaster.h"
#include "mktable.h"
#include "plot.h"

#include "speedo_meter.h"

#include <qwt_plot.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_math.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>

MBMaster   *mbmaster;
SerialPort *port;

int modules_n;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "Измерение шума AI400";
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
   tb->setSource( QUrl::fromLocalFile(":/html/help/index.html" ) );
   d.setWindowTitle( "Измерение шума AI400" );
   d.resize(500,300);
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
    QMessageBox::information(0,"Измерение шума AI400",
                             "Ошибка открытия порта.\n\n"
                             "Возможно, что порт используется\n"
                             "другим приложением.\n\n"
                             "Порт будет автоматически открыт\nпри его освобождении." );
 }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, 1, 0, "AI100" );
  mbmaster->add_slot( 1, 1, 0x000E, 1, MBDataType::Floats  ); // значения входов
  mbmaster->add_slot( 1, 2, 0x0082, 2, MBDataType::Dwords  );
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
  setAutoFillBackground(true);

  setupUi( this );
  setWindowTitle( "Уровень шума (СКО)" );
  resize(450,350);

  d1->setPalette(colorTheme(QColor(Qt::darkGray).dark(150)));

  startTimer( 100 );

  plot34 = new Plot(this, mbmaster, "1/1/1" );
  plot34->setWindowTitle("Вход");
  plot34->show();
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  MMSlot slot = mbmaster->getSlot( 1, 1 );
  MMSlot slot2 = mbmaster->getSlot( 1, 2 );

  QString s;
  s = QString().sprintf("0x%8.8X",slot2.data[1].toInt());
  if( !slot2.data[0].isValid() ) s = "Нет связи";
  le_cf->setText( s );

  bool b = slot.data[0].isValid();

  d1->setEnabled( b );
  if( !b )
  { d1->setLabel( "НЕТ CВЯЗИ" );
    return;
  }

  double f = plot34->noise()*1e6;
  d1->setValue( f );
  if( f < 1000 )  d1->setLabel( QString::number(f,     'f',1) + " нВ" );
  else            d1->setLabel( QString::number(f/1000,'f',1) + " мкВ" );
}

//==============================================================================
///
//==============================================================================
void MainWindow::on_pb_applay_clicked()
{
  int sr = le_sr->text().toInt();
  int dl = le_dl->text().toInt();
  bool chp  = cb_chp->isChecked();
  bool ac   = cb_ac->isChecked();
  bool skip = cb_skip->isChecked();
  bool fast = cb_fast->isChecked();

  int a=0;
             a |= ( sr << 12 );
  if( skip ) a |= ( 1 << 9 );
  if( fast ) a |= ( 1 << 8 );
  if( ac   ) a |= ( 1 << 5 );
  if( chp  ) a |= ( 1 << 4 );
             a |= dl;

 mbmaster->setSlotValue( 1, 2, 0, a );

}

//==============================================================================
///
//==============================================================================
QPalette MainWindow::colorTheme(const QColor &base) const
{
    const QColor background = base.dark(150);
    const QColor foreground = base.dark(200);

    const QColor mid   = base.dark(110);
    const QColor dark  = base.dark(170);
    const QColor light = base.light(170);
    const QColor text  = foreground.light(800);

    QPalette palette;
    for ( int i=0; i<QPalette::NColorGroups; i++ )
    {   QPalette::ColorGroup cg = (QPalette::ColorGroup)i;
        palette.setColor(cg, QPalette::Base, dark);
        palette.setColor(cg, QPalette::Text, text);
        palette.setColor(cg, QPalette::WindowText, dark);
    }
    return palette;
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::closeEvent( QCloseEvent *event )
{
}


