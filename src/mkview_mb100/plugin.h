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

  QStatusBar *statusbar;
  QLabel *status_requests;
  QLabel *status_answers;
  QLabel *status_errors;
  QLabel *full_time;

};


#endif
