#ifndef __CONSOLEWIDGET_H__
#define __CONSOLEWIDGET_H__

/*!
  \file consolewidget.h
  \brief Консоль

  Виджет для отображения логов. Присутствует кнопка "Очистить" и галочка
  "автопрокрутка". Вывод сообщений осуществляется при помощи глобального
  макроса CONSOLE_OUT.

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
  void print( const QString &str ); //!< Вывод сообщения
protected:
  void timerEvent(QTimerEvent * event);
private slots:
  void on_pb_clear_clicked();
private:
  QString data;
  QMutex mutex;
  int div_counter;
};

/**
  \brief Глобальная переменная -- ссылка на экземпляр консоли
  \warning Эту переменную нужно инициализировать перед первым использованием макроса CONSOLE_OUT
*/
extern ConsoleWidget *global_colsole;

/*!
  \brief Макрос для выдачи сообщений
  Макрос для выдачи сообщений на консоль из любого места и любого потока программы.

  Использует глобальную переменную \ref global_colsole, которую нужно инициализировать
  реально используемым экземпляром класса \ref ConsoleWidget
*/
#define CONSOLE_OUT( s ) \
 { if(global_colsole) global_colsole->print(s); }

#endif
