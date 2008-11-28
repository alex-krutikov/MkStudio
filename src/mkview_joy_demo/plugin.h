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
protected:
  void timerEvent( QTimerEvent *event );
  void closeEvent( QCloseEvent *event );
private:
  QPalette colorTheme(const QColor &base) const;
};


#endif
