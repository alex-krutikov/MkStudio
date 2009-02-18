#include <QtGui>

#include "consolewidget.h"
#include "console.h"

//###################################################################
//
//###################################################################
ConsoleWidget::ConsoleWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  div_counter=0;
  te->setFont( QFont( "Courier New" , 10 ) );
  te->setLineWrapMode( QTextEdit::NoWrap );
  bcb_autoscroll->setChecked( true );
  te->setReadOnly( true );
  startTimer( 200 );
}

//===================================================================
/// Функция таймера
//===================================================================
void ConsoleWidget::timerEvent(QTimerEvent * event)
{
  Q_UNUSED( event );

  te->insertPlainText( Console::takeMessage() );

	// лимитирование размера
	if( div_counter++ > 25 ) // 5 секунд
	{ div_counter = 0;
	  QTextDocument *doc = te->document();
	  if( doc->blockCount() > 10000 )
	  { doc->setMaximumBlockCount(5000);
	    doc->setMaximumBlockCount(0);
	  }
	}
	// автопрокрутка
  if( bcb_autoscroll->isChecked() )
	{ QScrollBar *sb = te->verticalScrollBar();
	  if( ! sb->isSliderDown() )	sb->setValue( sb->maximum() );
	}
}

//===================================================================
/// Очистка консоли
//===================================================================
void ConsoleWidget::on_pb_clear_clicked()
{
	te->clear();
}
