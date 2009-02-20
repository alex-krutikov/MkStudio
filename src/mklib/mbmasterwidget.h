#ifndef __MBMASTERWIDGET_H__
#define __MBMASTERWIDGET_H__

#include <QString>
#include <QWidget>
#include <QObjectCleanupHandler>

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
class MBMasterWidget : public QWidget
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
private slots:
  void on_tw_doubleClicked ( const QModelIndex & );
public slots:
  void stateUpdate();
signals:
  void attributes_saved( int module, int slot, QString attribute );
};

#endif
