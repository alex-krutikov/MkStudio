#include <QtGui>

#include "consolewidget.h"
#include "mbcommon.h"

#include "serialport.h"
#include "serialport_p.h"

//###################################################################
//
//###################################################################
SerialPort::SerialPort()
  : d( new SerialPortPrivate(this) )
{
  console_out_packets = false;
  answer_timeout         = 100;
  current_answer_timeout = -1;
}

//===================================================================
//
//===================================================================
SerialPort::~SerialPort()
{
  delete d;
}


//===================================================================
//
//===================================================================
void SerialPort::setName( const QString &portname_arg )
{
  d->close();
  portname=portname_arg;
}
//===================================================================
//
//===================================================================
void SerialPort::setSpeed( const int speed_arg )
{
  d->close();
  portspeed=speed_arg;
}

//===================================================================
//
//===================================================================
bool SerialPort::open()
{
  return d->open();
}

//===================================================================
//
//===================================================================
void SerialPort::close()
{
  d->close();
}

//===================================================================
//
//===================================================================
int SerialPort::setupDialog()
{
  SerialPortSetupDialog dialog;
  dialog.port = this;
  return dialog.exec();
}

//==============================================================================
// Поиск присутствующих в системе COM портов
//==============================================================================
QStringList SerialPort::queryComPorts()
{
  return SerialPortPrivate::queryComPorts();
}


//===================================================================
//
//===================================================================
int SerialPort::request( const QByteArray &request, QByteArray &answer,
                         int *errorcode)
{
  return d->request( request, answer,errorcode );
}

//===================================================================
//
//===================================================================
void SerialPort::reset_current_error()
{
  d->last_error_id = 0;
}

//===================================================================
//
//===================================================================
SerialPortSetupDialog::SerialPortSetupDialog()
{
  QTableWidgetItem *titem;
  setupUi( this );
  connect( pbOK, SIGNAL( clicked() ), SLOT( accept() ) );
  setWindowTitle("Настройка");
  resize(300,300);

  connect( tw, SIGNAL( cellDoubleClicked(int,int) ), this, SLOT( accept() ) );

  QStringList ports = SerialPort::queryComPorts();

  int i;
  tw->setColumnCount(2);
  QStringList sl;
  sl << "Порт" << "Описание";
  tw->setHorizontalHeaderLabels( sl );
  tw->horizontalHeader()->setHighlightSections( false );
  tw->horizontalHeader()->setClickable( false );
  tw->horizontalHeader()->resizeSection(0, 100 );
  tw->horizontalHeader()->setStretchLastSection( true );
  tw->verticalHeader()->setDefaultSectionSize(font().pointSize()+11);
  tw->verticalHeader()->hide();
  tw->setSelectionBehavior( QAbstractItemView::SelectRows );
  tw->setSelectionMode( QAbstractItemView::SingleSelection );
  tw->setRowCount( ports.count() );
  for( i=0; i<ports.count(); i++ )
  { titem = new QTableWidgetItem( ports.at(i).section(';',0,0) );
    titem ->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    tw->setItem(i,0,titem );
    titem = new QTableWidgetItem( ports.at(i).section(';',1,1) );
    titem ->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    tw->setItem(i,1,titem );
  }
  setFocus();
}

//===================================================================
//
//===================================================================
void SerialPortSetupDialog::accept()
{
  QList<QTableWidgetSelectionRange> sr = tw->selectedRanges();

  if( sr.count() )
  { port->setName( tw->item( sr.at(0).bottomRow(), 0 )->text() );
  } else
  { QMessageBox::warning(0,"Настройка","Порт не выбран.");
    return;
  }
  port->setSpeed( cb1->currentText().toInt() );

  done( QDialog::Accepted );
}
