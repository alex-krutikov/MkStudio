#include <QtGui>

#include <qwt_plot.h>

#include "mkselfrecorder.h"

MKSelfRecorder::MKSelfRecorder( QWidget *parent )
  : QWidget( parent )
{
  plot = new QwtPlot;
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget( plot );
  setLayout( layout );
}
