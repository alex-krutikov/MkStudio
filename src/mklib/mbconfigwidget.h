#ifndef __MBCONFIGWIDGET_H__
#define __MBCONFIGWIDGET_H__

#include <QWidget>
#include <QAbstractTableModel>
#include <QString>
#include <QByteArray>
#include <QtXml>

#include "mbcommon.h"

#define MODULES_MAX_N   50
#define SLOTS_MAX_N    100


//===================================================================
//! ������ ��� ��������� ������������ ������ �������
//===================================================================
#include "ui/ui_mbconfigwidget.h"
class MBConfigWidget : public QWidget,
                       public Ui::MBConfigWidget
{

class ModulesModel : public QAbstractTableModel
{
  friend class MBConfigWidget;

private:

  struct t_module_desc
  { QString addr;
    QString name;
    QString desc;
  };

  int rowCount    ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return MODULES_MAX_N; }
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return 4; }
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QVariant headerData ( int section, Qt::Orientation orientation,
                                   int role = Qt::DisplayRole ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  bool setData ( const QModelIndex & index,
                   const QVariant & value, int role = Qt::EditRole );
  t_module_desc md[MODULES_MAX_N];
  void refresh();
};

class SlotsModel : public QAbstractTableModel
{
  friend class MBConfigWidget;

private:

  struct t_slot_desc
  { QString addr;
    int     n;
    MBDataType type;
    QString desc;
    QString attributes;
  };

  int rowCount    ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return SLOTS_MAX_N; }
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const
    { Q_UNUSED( parent ); return 5; }
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QVariant headerData ( int section, Qt::Orientation orientation,
                                   int role = Qt::DisplayRole ) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  bool setData ( const QModelIndex & index,
                   const QVariant & value, int role = Qt::EditRole );
  void set_current_module( int i );
  int current_module;
  t_slot_desc sd[MODULES_MAX_N][SLOTS_MAX_N];
};



  Q_OBJECT
public:
  MBConfigWidget( QWidget *parent = 0 );
//! �������� ������������
  void clearConfiguration();

//! ��������� ������������
/*!
   \param xml ������ ������ � ������������� (����� xml-��������)
*/
  void loadConfiguration( QByteArray &xml );

//! ��������� ������������
/*!
   \param doc XML �������� � ��������� ������������
*/
  void loadConfiguration( QDomDocument &doc );

//! �������� ������� ������������
/*!
   \param xml ������ ������ � ������������� (����� xml-��������)
*/
  void saveConfiguration( QByteArray &xml );

//! �������� ������� ������������
/*!
   \param doc XML �������� � ��������� ������������
*/
  void saveConfiguration( QDomDocument &doc );

//! �������������� ������� ������������ � ��������� ����
/*!
   \return ��������� �������� ������������
*/
  QString exportConfiguration();

//! ������ ��������� �����
/*!
   \param module     ����� ������
   \param slot       ����� �����
   \param attributes ���������
*/
  void setSlotAttributes(int module, int slot, const QString &attributes);

private:
  ModulesModel modules_model;
  SlotsModel   slots_model;

  ModulesModel::t_module_desc mbuffer;
  SlotsModel::t_slot_desc     sbuffer[SLOTS_MAX_N];

private slots:
  void on_tw1_customContextMenuRequested ( const QPoint& );
  void on_tw2_activated( const QModelIndex& );
  void tw1_currentRowChanged(const QModelIndex&current,
                             const QModelIndex&previous);
};


#endif
