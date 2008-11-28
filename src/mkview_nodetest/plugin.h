#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <QtGui>

class ModuleWidget;

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
class MainWindow : public QWidget
{
  Q_OBJECT
public:
  MainWindow( QWidget *parent = 0 );
  void timerEvent( QTimerEvent * event );

private:
  QLabel *l1;
  QLabel *l2;
  QLabel *l3;

  int mode;
};

//-----------------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------------
class TestThread : public QThread
{
public:
  TestThread( QObject *parent = 0);
  void run();
};





#endif
