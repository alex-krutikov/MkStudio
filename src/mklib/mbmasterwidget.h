#ifndef __MBMASTERWIDGET_H__
#define __MBMASTERWIDGET_H__

#include <QWidget>
#include <QDialog>
#include <QAbstractTableModel>
#include <QMainWindow>

#include "mbmaster.h"
#include "mbcommon.h"

class MBMasterValuesWidget;
class QwtPlotCurve;

namespace Ui { class MBMasterWidget; }

struct MTMSlot
{
  int        flag;
  int        status;
  int        n;
  int        mm_index;
  MMModule   module;
  QString    name;
  MBDataType datatype;
  QString    addr;
  int        len;
  QString    desc;
};

//===================================================================
//
//===================================================================
class MBMasterWidgetTableModel : public QAbstractTableModel
{
  public:
    MBMasterWidgetTableModel( QObject *parent = 0);
  public:
    int rowCount    ( const QModelIndex & parent = QModelIndex() ) const
      { Q_UNUSED( parent ); return table.count(); }
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const
      { Q_UNUSED( parent ); return 5; }
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData ( int section, Qt::Orientation orientation,
                                   int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    bool setData ( const QModelIndex & index,
                   const QVariant & value, int role = Qt::EditRole );
  public:
    void refresh();
    void update_slots( MBMaster *mm );

    QVector<MTMSlot> table;
};


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
  void setMBMaster( MBMaster *mm );
private:
  void delete_slotwidgets();

  Ui::MBMasterWidget *ui;
  MBMaster *mbmaster;
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
