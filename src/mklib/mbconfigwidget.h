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

namespace Ui { class MBConfigWidget; }

class MBConfigWidgetPrivate;

//===================================================================
//! ������ ��� ��������� ������������ ������ �������
//===================================================================
class MBConfigWidget : public QWidget
{
  Q_OBJECT
public:
  MBConfigWidget( QWidget *parent = 0 );
  ~MBConfigWidget();
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
  Ui::MBConfigWidget    *ui;
  MBConfigWidgetPrivate *d;
private slots:
  void on_tw1_customContextMenuRequested ( const QPoint& );
  void on_tw2_activated( const QModelIndex& );
  void tw1_currentRowChanged(const QModelIndex&current,
                             const QModelIndex&previous);
};


#endif
