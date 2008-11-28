#ifndef __MBMASTERWIDGET_H__
#define __MBMASTERWIDGET_H__

#include <QtCore>
#include <QtXml>

#include <QWidget>
#include <QDialog>
#include <QAbstractTableModel>
#include <QMainWindow>

#include "mbmaster.h"
#include "mbcommon.h"

class MBMasterValuesWidget;
class QwtPlotCurve;

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
#include "ui/ui_mbmasterwidget.h"
class MBMasterWidget : public QWidget,
                       public Ui::MBMasterWidget
{
  Q_OBJECT
public:
  MBMasterWidget( QWidget *parent = 0 );
  void load_config( QByteArray &xml );
  void load_config( QDomDocument &doc );
  void clear_config();
  void polling_start();                               // запустить опрос
  void polling_stop();                                // остоновить опрос
  void setMBMaster( MBMaster *mm );
private:
  void delete_slotwidgets();

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

//===================================================================
//
//===================================================================
#include "ui/ui_mbmslotwidget.h"
class MBMasterSlotWidget : public QMainWindow,
                           public Ui::MBMasterSlotWidget
{
  Q_OBJECT
public:
  MBMasterSlotWidget( QWidget *parent = 0, MBMaster *mm = 0, MTMSlot slot = MTMSlot() );
private:
  void timerEvent( QTimerEvent *event);
  void uncheck_all_formats();
  void configure_table();
  MTMSlot slot;
  MBMaster *mm;
  QwtPlotCurve *plot_curve;
  QVector<double> plot_data_x;
  QVector<double> plot_data_y;
  static QMap<QString,QString> class_settings;
private slots:
  void on_pb_format_clicked();
  void on_action_format_dec_activated();
  void on_action_format_hex_activated();
  void on_action_format_bin_activated();
  void on_action_format_float_activated();
  void on_action_format_unsigned_activated();
  void on_action_format_text_cp1251_activated();
  void on_action_format_text_866_activated();
  void on_action_format_text_KOI8_R_activated();
  void on_action_format_options_activated();
  void on_action_view_table_toggled(bool);
  void on_action_view_plot_toggled(bool);
  void action_export_activated();
};

//===================================================================
//
//===================================================================
#include "ui/ui_mbmslotexportdialog.h"
class MBMasterSlotExportDialog : public QDialog,
                           public Ui::MBMasterSlotExportDialog
{
  Q_OBJECT
public:
  MBMasterSlotExportDialog( QWidget *parent = 0 );
};


#endif
