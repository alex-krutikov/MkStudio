#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <QObject>

#include "interface.h"

#include "mbmaster.h"

class MKViewPlugin : public QObject, MKViewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(MKViewPluginInterface)

public:
    QString echo(const QString &message);
    QString moduleName();
    void helpWindow( QWidget *parent = 0 );
    void mainWindow(  QWidget *parent=0, const QString &portname=QString(), int portspeed=115200, int node=1, int subnode=0 );
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

class QwtPlotCurve;

#include "ui_mainwindow.h"
//==============================================================================
//
//==============================================================================
class MainWindow : public QMainWindow,
                   public Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow();
private:
  void calc();
private slots:
  void group_update();
  void on_pb_zahvat1_clicked();
  void on_pb_zahvat2_clicked();
  void on_pb_adc_clear_clicked();
  void on_pb_adc_write_clicked();
private:
  QwtPlotCurve *plot_data, *hist_data;
  static const int plot_length = 150;
  double plot_x[plot_length];
  double plot_y[plot_length];
  static const int codes_length = 5;
  int codes_array[codes_length];
  bool first_run;
  int write_automat_state;
  union {
  TADC_EERPOM_DATA st;
  unsigned char buff[64]; } eeprom;
  QString app_header;
};



#endif
