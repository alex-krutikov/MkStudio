#ifndef __PLOT_H__
#define __PLOT_H__

#include <QTimer>
#include <QWidget>

#include "mk_global.h"

#include <qwt_plot_curve.h>
class MBMasterXML;
class QTimerEvent;
class QTableWidgetItem;
class QToolButton;
class QFile;
class QTextStream;
class QwtPlotZoomer;
class QwtPlotPicker;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

namespace Ui { class Plot; }

class MK_EXPORT Plot : public QWidget
{
  Q_OBJECT
public:
  Plot( QString title, QList<QTableWidgetItem *> mktableItemList, bool min_flag, QWidget *parent=0, MBMasterXML *mbmaster=0 );
  virtual ~Plot();
  QSize sizeHint () const;
  double noise() const;
  void setPlotList( QString list );
private slots:
  void pb_clear_clicked();
  void pb_pause_clicked();
  void pb_export_clicked();
  void pb_minimize_clicked();
  void updateMinimizeButtonState( bool state );
  void on_cb_speed_currentIndexChanged( int );
  void on_sb_tact_valueChanged( int );
  void on_sb_plot_pen_w_valueChanged( int );
  void change_current_plot( int );
  void on_le_input_signal_range_textChanged( QString signal );
  void params_change();
  void calc_statistic();
  bool load_recorder_params();
  bool save_recorder_params();
  void set_file_for_write();
  void file_write_start_stop();
private:
  Ui::Plot *ui;
  QToolButton *pb_clear;
  QToolButton *pb_pause;
  QToolButton *pb_export;
  QToolButton *pb_minimize;
  QwtPlotCurve *plot_data1;
  QwtPlotCurve *plot_data2;
  QwtPlotCurve *plot_data3;
  QwtPlotCurve *plot_data4;
  int y_data_len;
  int plots_count;
  double *a_value;
  double *x_data;
  double *y_data1;
  double *y_data2;
  double *y_data3;
  double *y_data4;
  double *y_data_stat;
  double y_max;
  double y_min;
  double y_mean;
  double y_std;
  static const int y_hist_data_len = 17;
  double y_hist_data[ y_hist_data_len ];
  double x_hist_data[ y_hist_data_len ];
  int *module_index;
  int *slot_index;
  int *value_index;
  bool firstrun_flag;
  bool pause_flag;
  bool minimize_flag;
  bool hide_minimize_flag;
  bool file_write_flag;
  int points_counter;
  double *avr_a;
  int avr_counter;
  MBMasterXML *mbmaster;
  double y_min1, y_min2, y_min3, y_min4;
  double y_max1, y_max2, y_max3, y_max4;
  double eb_ref;
  QList<QTableWidgetItem *> mktableItemList;
  QFile file;
  QTextStream file_stream;
  int file_write_counter;
  QString titleStr;
  int timerId, timerInterval;
  quint64 time_append;
  QwtPlotZoomer *zoomer;
  QwtPlotPicker *picker;

signals:
  void signalMinimize( bool is_minimize );
  void signalPlotClose();
  void signalPlotOpen();
protected:
  void timerEvent  ( QTimerEvent  * event );
  void closeEvent  ( QCloseEvent  * event );
  void changeEvent ( QEvent       * event );
};

#endif
