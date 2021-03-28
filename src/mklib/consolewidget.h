#ifndef __CONSOLEWIDGET_H__
#define __CONSOLEWIDGET_H__

/*!
  \file consolewidget.h
  \brief Консоль

  Виджет для отображения отладочных сообщений. Присутствует кнопка "Очистить" и
  галочка "автопрокрутка".
*/

#include "mk_global.h"

#include <QWidget>

namespace Ui {
class ConsoleWidget;
}

//===================================================================
//! Виджет консоль для выдачи сообщений и логов.
//===================================================================
class MK_EXPORT ConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    ConsoleWidget(QWidget *parent = 0);
    virtual ~ConsoleWidget();

protected:
    void timerEvent(QTimerEvent *event);
private slots:
    void on_pb_clear_clicked();

private:
    int div_counter;
    Ui::ConsoleWidget *ui;
};

#endif
