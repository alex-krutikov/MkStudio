#ifndef MYSLOTCONFIGTABLEVIEW_H
#define MYSLOTCONFIGTABLEVIEW_H

#include <QTableView>
#include <QItemDelegate>

//===================================================================
//! ������� ������ �� ������� "������������ ���� ��"
//===================================================================
class SlotConfigTableView : public QTableView
{
  Q_OBJECT
public:
  SlotConfigTableView(QWidget* parent = 0) : QTableView(parent)  { }
public slots:
  void handleClick(QModelIndex ind);
};

//===================================================================
//! ������� ��� ������� ��������� ������������ ������ �������
//===================================================================
class MBConfigWidgetItemDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  MBConfigWidgetItemDelegate( QWidget *parent = 0 );
  void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex &index ) const;
  QWidget *createEditor ( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  void updateEditorGeometry ( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
private slots:
  void commitAndCloseEditor();
};

#endif
