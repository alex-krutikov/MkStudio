#ifndef __mainwindow_h_
#define __mainwindow_h_

#include <QMainWindow>

class SerialPort;

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
private slots:
  void settingsChanged();
private:
  void closeEvent ( QCloseEvent * event );
  void console_update();

  SerialPort *serialport;
};

#endif
