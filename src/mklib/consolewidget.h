#ifndef __CONSOLEWIDGET_H__
#define __CONSOLEWIDGET_H__

/*!
  \file consolewidget.h
  \brief Консоль

  Виджет для отображения отладочных сообщений. Присутствует кнопка "Очистить" и галочка
  "автопрокрутка".
*/

#include <QtGui>

//===================================================================
//! Виджет консоль для выдачи сообщений и логов.
//===================================================================
#include "ui/ui_consolewidget.h"
class ConsoleWidget : public QWidget,
                      public Ui::ConsoleWidget
{
  Q_OBJECT
public:
  ConsoleWidget( QWidget *parent = 0 );
protected:
  void timerEvent(QTimerEvent * event);
private slots:
  void on_pb_clear_clicked();
private:
  int div_counter;
};

#endif
