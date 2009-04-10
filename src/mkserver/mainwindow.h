#ifndef __mainwindow_h_
#define __mainwindow_h_

#include <QMainWindow>

class SerialPort;
class ModbusTcpServer;

//==============================================================================
// главное окно
//==============================================================================
#include "ui_mainwindow.h"
class MainWindow : public QMainWindow,
                   public Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow();
  virtual ~MainWindow();
protected:
  void timerEvent(QTimerEvent *event);
private slots:
  void settingsChanged();
private:
  void closeEvent ( QCloseEvent * event );
  void console_update();

  SerialPort *serialport;
  ModbusTcpServer *tcpserver;
};

#endif
