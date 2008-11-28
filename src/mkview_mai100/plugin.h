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

class QwtPlotCurve;

typedef struct __attribute__(( packed ))
{ DWORD EEPROM_valid;    // 0x20  ������ ������� ������ ��� EEPROM:
                         //       ���� ���������� ������ ����� ������:
                         //       = 0 - ��������� (���� �������� 0)
                         //       = 1 - �������
  // ��������� ����������
  float c_adc_a;         // 0x20  ����������� a (y=a*x+b) ��� ������������� ���
  float c_adc_b;         // 0x24  --//--      b
  float c_dac_1mA;       // 0x28  ����������� ������������� ��� �� 1mA
  float c_dac_5mA;       // 0x2C  --//--                           5mA
  float c_impedance;     // 0x30  ������������� ������������ ����, ��

  BYTE EEPROM_CRC8;      //       ������� CRC8 ������ � EEPROM
  BYTE EEPROM_end;       //       ����� ������� ������ ��� EEPROM (��������� �������� ��� ��������)
} TEEPROM;

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
  void timerEvent( QTimerEvent * event );
private:
  void calc();
private slots:
  void on_pb_write_clicked();
private:
  QString app_header;

  int write_automat_state;
};



#endif
