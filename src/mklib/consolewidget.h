#ifndef __CONSOLEWIDGET_H__
#define __CONSOLEWIDGET_H__

/*!
  \file consolewidget.h
  \brief �������

  ������ ��� ����������� ���������� ���������. ������������ ������ "��������" � �������
  "�������������".
*/

#include <QtGui>

//===================================================================
//! ������ ������� ��� ������ ��������� � �����.
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
