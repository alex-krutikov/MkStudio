#ifndef __DIALOGS__H_
#define __DIALOGS__H_

class QwtPlotCurve;

#include "ui_initdialog.h"
//==============================================================================
/// ���� ������ ����� � ��������
//==============================================================================
class InitDialog : public QDialog,
                   public Ui::InitDialog
{
  Q_OBJECT
public:
  InitDialog( QWidget *parent = 0);
  void accept();
};

#include "ui_assigndialog.h"
//==============================================================================
/// ������ "�������� ����������"
//==============================================================================
class AssignDialog : public QDialog,
                     public Ui::AssignDialog
{
  Q_OBJECT
public:
  AssignDialog( QWidget *parent = 0,
                const QString &assign = QString(),
                const QString &format = QString(),
                const QString &ss     = QString(),
                const QStringList &sl = QStringList() );
  QString assign;
  QString format;
  QString ss;
private:
  void accept();
};

#include "ui_settingsdialog.h"
//==============================================================================
/// ������ "���������"
//==============================================================================
class SettingsDialog : public QDialog,
                       public Ui::SettingsDialog
{
  Q_OBJECT
public:
  SettingsDialog( QWidget *parent = 0 );
};

#include "ui_texteditdialog.h"
//==============================================================================
/// ������ "�������������� ������"
//==============================================================================
class TextEditDialog : public QDialog,
                       public Ui::TextEditDialog
{
  Q_OBJECT
public:
  TextEditDialog( QWidget *parent = 0 );
};

#include "ui_unitedslots.h"
//==============================================================================
/// ������ "�������������� ������"
//==============================================================================
class UnitedSlots : public QWidget,
                    public Ui::UnitedSlots
{
  Q_OBJECT
public:
  UnitedSlots( QWidget *parent = 0 );
  void timerEvent ( QTimerEvent * event );
private:
  static const int curves_num = 10;
  QwtPlotCurve *curves[curves_num];
};

#endif
