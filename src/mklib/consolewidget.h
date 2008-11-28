#ifndef __CONSOLEWIDGET_H__
#define __CONSOLEWIDGET_H__

/*!
  \file consolewidget.h
  \brief �������

  ������ ��� ����������� �����. ������������ ������ "��������" � �������
  "�������������". ����� ��������� �������������� ��� ������ �����������
  ������� CONSOLE_OUT.

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
  void print( const QString &str ); //!< ����� ���������
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
  \brief ���������� ���������� -- ������ �� ��������� �������
  \warning ��� ���������� ����� ���������������� ����� ������ �������������� ������� CONSOLE_OUT
*/
extern ConsoleWidget *global_colsole;

/*!
  \brief ������ ��� ������ ���������
  ������ ��� ������ ��������� �� ������� �� ������ ����� � ������ ������ ���������.

  ���������� ���������� ���������� \ref global_colsole, ������� ����� ����������������
  ������� ������������ ����������� ������ \ref ConsoleWidget
*/
#define CONSOLE_OUT( s ) \
 { if(global_colsole) global_colsole->print(s); }

#endif
