#ifndef __mainwindow_h_
#define __mainwindow_h_

#include <QMainWindow>

#include <memory>

class AbstractSerialPort;
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
protected:
  void timerEvent(QTimerEvent *event);
private slots:
  void settingsChanged();
private:
  void closeEvent ( QCloseEvent * event );
  void console_update();

  std::auto_ptr<AbstractSerialPort> serialport;
  std::auto_ptr<ModbusTcpServer> tcpserver;
};

#endif
