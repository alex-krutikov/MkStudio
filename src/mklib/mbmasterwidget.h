#ifndef __MBMASTERWIDGET_H__
#define __MBMASTERWIDGET_H__

#include <QString>
#include <QWidget>
#include <QObjectCleanupHandler>

#include "mk_global.h"

class MBMasterValuesWidget;
class MBMasterXML;
class MBMasterWidgetTableModel;
class QByteArray;
class QDomDocument;
class QModelIndex;


namespace Ui { class MBMasterWidget; }

//===================================================================
//
//===================================================================
class MK_EXPORT MBMasterWidget : public QWidget
{
  Q_OBJECT
public:
  MBMasterWidget( QWidget *parent = 0 );
  virtual ~MBMasterWidget();
  void load_config( QByteArray &xml );
  void load_config( QDomDocument &doc );
  void clear_config();
  void polling_start();                               // запустить опрос
  void polling_stop();                                // остоновить опрос
  void setMBMaster( MBMasterXML *mm );
private:
  void delete_slotwidgets();

  Ui::MBMasterWidget *ui;
  MBMasterXML *mbmaster;
  MBMasterWidgetTableModel *mbmodel;
  QObjectCleanupHandler slotwidgets;
  bool mktable_minimize_flag;
private slots:
  void on_tw_doubleClicked ( const QModelIndex & );
  void slotMkTableMinimizeStateChange( bool state );
  void slotMkTableMinimizeAllHide( bool state );
public slots:
  void stateUpdate();
signals:
  void attributes_saved( int module, int slot, QString attribute );
  void signalMkTableMinimizeStateChange( bool state );
  void signalMinimize( bool is_minimize );
  void signalOstsOpen();
  void signalOstsClose();
};

#endif
