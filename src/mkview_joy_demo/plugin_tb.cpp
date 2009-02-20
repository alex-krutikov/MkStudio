#include <QtGui>

#include "plugin.h"
#include "serialport.h"
#include "mbmasterxml.h"

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

#define QSETTINGS_PARAM (qApp->applicationDirPath()+"/joystick_demo.ini"),QSettings::IniFormat

MBMasterXML   *mbmaster;
SerialPort *port;

int modules_n;

QString MKViewPlugin::echo(const QString &message)
{
    return message;
}

QString MKViewPlugin::moduleName()
{
    return "������������ ���������";
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
   d.setWindowTitle( "������������ ���������" );
   d.resize(500,300);
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
    QMessageBox::information(0,"���������",
                             "������ �������� �����.\n\n"
                             "��������, ��� ���� ������������\n"
                             "������ �����������.\n\n"
                             "���� ����� ������������� ������\n��� ��� ������������." );
 }
  mbmaster->setTransport( port );

  mbmaster->add_module( 1, 1, 0, "AI100" );
  mbmaster->add_slot( 1, 1, 0x000E, 6, MBDataType::Floats  ); // �������� ������
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
  setPalette(colorTheme(QColor(Qt::darkGray).dark(150)));

  setupUi( this );
  setWindowTitle( "������������ ���������" );
  resize(1000,400);

  startTimer( 100 );
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::timerEvent( QTimerEvent * event )
{
  Q_UNUSED( event );

  MMSlot slot = mbmaster->getSlot( 1, 1 );

  bool b = slot.data[0].isValid();

  d1->setEnabled( b );
  d2->setEnabled( b );
  d3->setEnabled( b );
  if( !b )
  { d1->setLabel( "��� C����" );
    d2->setLabel( "��� C����" );
    d3->setLabel( "��� C����" );
    return;
  }

  double f = slot.data[0].toDouble()+0.01;
  d1->setValue( f );
  d1->setLabel( QString::number(f,'f',1) );

  f = slot.data[2].toDouble()+0.01;
  d2->setValue( f );
  d2->setLabel( QString::number(f,'f',1) );

  f = slot.data[4].toDouble()+0.01;
  d3->setValue( f );
  d3->setLabel( QString::number(f,'f',1) );
}

//==============================================================================
///
//==============================================================================
QPalette MainWindow::colorTheme(const QColor &base) const
{
    const QColor background = base.dark(150);
    const QColor foreground = base.dark(200);

    const QColor mid = base.dark(110);
    const QColor dark = base.dark(170);
    const QColor light = base.light(170);
    const QColor text = foreground.light(800);

    QPalette palette;
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        QPalette::ColorGroup cg = (QPalette::ColorGroup)i;

        palette.setColor(cg, QPalette::Base, base);
        palette.setColor(cg, QPalette::Background, background);
        palette.setColor(cg, QPalette::Mid, mid);
        palette.setColor(cg, QPalette::Light, light);
        palette.setColor(cg, QPalette::Dark, dark);
        palette.setColor(cg, QPalette::Text, text);
        palette.setColor(cg, QPalette::Foreground, foreground);
    }
    return palette;
}

//-------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------
void MainWindow::closeEvent( QCloseEvent *event )
{
}

