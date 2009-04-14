#ifndef __CONSOLEWIDGET_H__
#define __CONSOLEWIDGET_H__

/*!
  \file consolewidget.h
  \brief �������

  ������ ��� ����������� ���������� ���������. ������������ ������ "��������" � �������
  "�������������".
*/

#include <QWidget>

#include "mk_global.h"

namespace Ui { class ConsoleWidget; }

//===================================================================
//! ������ ������� ��� ������ ��������� � �����.
//===================================================================
class MK_EXPORT ConsoleWidget : public QWidget
{
  Q_OBJECT
public:
  ConsoleWidget( QWidget *parent = 0 );
  virtual ~ConsoleWidget();
protected:
  void timerEvent(QTimerEvent * event);
private slots:
  void on_pb_clear_clicked();
private:
  int div_counter;
  Ui::ConsoleWidget *ui;
};

#endif
