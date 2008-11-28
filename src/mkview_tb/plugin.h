#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <QtGui>
#include <qwt_data.h>

class ModuleWidget;
class QwtPlotCurve;
class PlotUstavkaData;

#include "interface.h"
//-----------------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------
class MKViewPlugin : public QObject, MKViewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(MKViewPluginInterface)

public:
    QString echo(const QString &message);
    QString moduleName();
    void helpWindow( QWidget *parent = 0 );
    void mainWindow(  QWidget *parent=0, const QString &portname=QString(),
                            int portspeed=115200, int node=1, int subnode=0   );
};

//-----------------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------
#include "ui_mainwindow.h"
class MainWindow : public QMainWindow,
                   public Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow( QWidget *parent = 0 );
  void timerEvent( QTimerEvent *event );
  void closeEvent( QCloseEvent *event );
private slots:
  void on_pb_start_clicked();
  void on_pb_stop_clicked();
  void on_pb_skip_clicked();
  void pb_clear_clicked();
private:
  QStatusBar *statusbar;
  QLabel *status_requests;
  QLabel *status_answers;
  QLabel *status_errors;
  QLabel *full_time;

  QwtPlotCurve *plot_curve_ustavka;
  QwtPlotCurve *plot_curve_t1;
  QwtPlotCurve *plot_curve_t2;

  PlotUstavkaData *ustavka_data;

  int state;
  double ftime;
  double a;
  double b;
  double t;
  double ust_set;
  int cycle_counter;
  int time_dev;
  int cc;

  static const int buff_len = 500;

  double buff_x[ buff_len ];
  double buff_ustavka[ buff_len ];
  double buff_t1[ buff_len ];
  double buff_t2[ buff_len ];
  int buff_index;

  QStringList state_names;
};


#endif
