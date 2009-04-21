#ifndef __mainwindow_h_
#define __mainwindow_h_

#include <QMainWindow>

class Server;

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

  Server *server;
};

#endif
