#ifndef __SLOTWIDGET_H__
#define __SLOTWIDGET_H__

#include <QMainWindow>
#include <QDialog>
#include <QString>
#include <QMap>

#include "mk_global.h"

namespace Ui
{ class SlotWidget;
  class SlotDialog;
}

class MBMasterXML;
class QwtPlotCurve;
class QToolButton;

//==============================================================================
// Окно с представлением слота (используется по двойному нажатию на слот)
//==============================================================================
class MK_EXPORT SlotWidget : public QMainWindow
{
  Q_OBJECT
public:
  SlotWidget( QWidget *parent = 0, MBMasterXML *mm=0, int module=0, int slot=0, bool min_flag=false );
  virtual ~SlotWidget();
  QString get_mean(){ return QString::number( no_trans_y_mean ); }
private slots:
  void view_changed();
  void format_changed();
  void on_action_set_scale_triggered();
  void export_data();
  void pb_pause_clicked();
  void pb_minimize_clicked();
  void slot_line_trans_used( bool use );
  void slot_statistic_show( bool isVisible );
  void updateMinimizeButtonState( bool state );
signals:
  void attributes_saved(int,int,QString);
  void signalMinimize( bool is_minimize );
  void signalOstsOpen();
  void signalOstsClose();
private:
  QToolButton *pb_pause;
  QToolButton *pb_minimize;
  QWidget *toolWidget;
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
  QVector<double> plot_data_y_trans;
  double ax,bx,kx,ky;
  struct {int m,s,n;} scale_base_x, scale_base_y;
  enum {None, Coeff, Interval, Base} scale_mode_x,scale_mode_y;
  bool plot2_unsigned;
  bool pause_flag;
  bool use_line_trans;
  double line_trans_x1,line_trans_y1,line_trans_x2,line_trans_y2;
  double y_max;
  double y_min;
  double y_mean,no_trans_y_mean;
  double y_std;
  double eb_ref;
  static const int hist_plot_data_y_len = 17;
  double hist_plot_data_x[ hist_plot_data_y_len ];
  double hist_plot_data_y[ hist_plot_data_y_len ];
  bool minimize_flag;
  void calc_statistic();
  QMap<QString,QString> settings;
protected:
  void changeEvent(    QEvent * event );
  void closeEvent( QCloseEvent *event );
};

//==============================================================================
// Диалог о масштабировании графика
//==============================================================================
class MK_EXPORT SlotDialog : public QDialog
{
  Q_OBJECT
public:
  SlotDialog( SlotWidget *slotwidget, QWidget *parent = 0, QMap<QString,QString> *settings = 0 );
  virtual ~SlotDialog();
private slots:
  void accept();
  void radio_buttons();
  void linear_transformation_correct();
private:
  Ui::SlotDialog *ui;
  QMap<QString,QString> *settings;
  SlotWidget *slotwidget;
};

#endif
