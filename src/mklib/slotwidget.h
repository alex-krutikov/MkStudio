#ifndef __SLOTWIDGET_H__
#define __SLOTWIDGET_H__

#include <QMainWindow>
#include <QDialog>
#include <QString>
#include <QMap>

namespace Ui
{ class SlotWidget;
  class SlotDialog;
}

class MBMasterXML;
class QwtPlotCurve;

//==============================================================================
// Окно с представлением слота (используется по двойному нажатию на слот)
//==============================================================================
class SlotWidget : public QMainWindow
{
  Q_OBJECT
public:
  SlotWidget( QWidget *parent = 0, MBMasterXML *mm=0, int module=0, int slot=0 );
  virtual ~SlotWidget();
private slots:
  void view_changed();
  void format_changed();
  void on_action_set_scale_triggered();
  void export_data();
signals:
  void attributes_saved(int,int,QString);
private:
  void save_settings();
  void timerEvent( QTimerEvent *event);
  void configure_table();
  void applay_settings();

  Ui::SlotWidget *ui;
  int module_n,slot_n;
  MBMasterXML *mm;
  QwtPlotCurve *plot_curve;
  QwtPlotCurve *plot2_curve;
  QVector<double> plot_data_x;
  QVector<double> plot_data_y;
  double ax,bx,kx,ky;
  struct {int m,s,n;} scale_base_x, scale_base_y;
  enum {None, Coeff, Interval, Base} scale_mode_x,scale_mode_y;
  bool plot2_unsigned;

  QMap<QString,QString> settings;
};

//==============================================================================
// Диалог о масштабировании графика
//==============================================================================
class SlotDialog : public QDialog
{
  Q_OBJECT
public:
  SlotDialog( QWidget *parent = 0, QMap<QString,QString> *settings = 0 );
  virtual ~SlotDialog();
private slots:
  void accept();
  void radio_buttons();
private:
  Ui::SlotDialog *ui;
  QMap<QString,QString> *settings;
};

#endif
