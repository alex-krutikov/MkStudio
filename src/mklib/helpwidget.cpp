#include <QtGui>

#include "helpwidget.h"

HelpWidget::HelpWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  setWindowIcon( QIcon(":/icons/res/help2.png" ) );

  QFont f = font();
  f.setPixelSize( 16 );
  tb->setFont( f );

  tb->setOpenExternalLinks( true );

  connect( pb_backward, SIGNAL( clicked() ), tb, SLOT( backward() ) );
  connect( pb_forward, SIGNAL( clicked() ), tb, SLOT( forward() ) );
  connect( pb_home, SIGNAL( clicked() ), tb, SLOT( home() ) );

  connect( tb, SIGNAL( backwardAvailable(bool) ), pb_backward, SLOT( setEnabled(bool) ) );
  connect( tb, SIGNAL( forwardAvailable(bool) ),  pb_forward,  SLOT( setEnabled(bool) ) );
}

void HelpWidget::setContents( const QUrl &url )
{
  tb->setSource( url );
}
