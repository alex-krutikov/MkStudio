#ifndef __PLOT_H__
#define __PLOT_H__

#include <QTimer>
#include <QWidget>

#include "mk_global.h"

class MBMasterXML;
class QTimerEvent;

class QToolButton;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

namespace Ui { class Plot; }

class MK_EXPORT Plot : public QWidget
{
  Q_OBJECT
public:
  Plot( QWidget *parent=0, MBMasterXML *mbmaster=0, const QString &config = QString() );
  virtual ~Plot();
  void timerEvent ( QTimerEvent * event );
  QSize sizeHint () const;
  double noise() const;
private slots:
  void pb_clear_clicked();
  void pb_pause_clicked();
  void pb_export_clicked();
  void on_cb_speed_currentIndexChanged( int );
private:
  Ui::Plot *ui;
  QToolButton *pb_pause;
  QToolButton *pb_export;
  static const int y_data_len = 200;
  double y_data[ y_data_len ];
  double x_data[ y_data_len ];
  double y_max;
  double y_min;
  double y_mean;
  double y_std;
  static const int y_hist_data_len = 17;
  double y_hist_data[ y_hist_data_len ];
  double x_hist_data[ y_hist_data_len ];
  int module_index;
  int slot_index;
  int value_index;
  bool firstrun_flag;
  bool pause_flag;
  int points_counter;
  double avr_a;
  int avr_counter;
  MBMasterXML *mbmaster;
};

#endif
