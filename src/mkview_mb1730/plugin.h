#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <QtGui>
#include <qwt_data.h>

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

typedef quint32 DWORD;
typedef quint16 WORD;
typedef quint8  BYTE;

typedef struct Q_PACKED // Общая длина 64 байта
{
  DWORD signature;  // 0xA5B5C5D5 Сигнатура
  DWORD version;    // версия платы АЦП
  float a,b;        // коэффициенты в y=a*x+b
  BYTE reserv[46];  // резерв
  WORD crc16;
} TADC_EERPOM_DATA;

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
  void on_pb_ao_0_clicked();
  void on_pb_ao_2048_clicked();
  void on_pb_ao_4095_clicked();
  void on_pb_sm_enable_clicked();
  void on_pb_sm_disable_clicked();
  void on_pb_lp_trap_clicked();
  void on_pb_hp_trap_clicked();
  void on_pb_adc_clear_clicked();
  void on_pb_adc_write_clicked();
  void on_pb_font_plus_clicked();
  void on_pb_font_minus_clicked();
  void on_pb_ai1_graph_clicked();
  void on_pb_ai2_graph_clicked();
private:
  void calc();

  QStatusBar *statusbar;
  QLabel *status_requests;
  QLabel *status_answers;
  QLabel *status_errors;
  QLabel *full_time;
  bool close_flag;

  int write_automat_state;
  union
  { TADC_EERPOM_DATA st;
    unsigned char buff[64];
  } eeprom;

};

#endif
