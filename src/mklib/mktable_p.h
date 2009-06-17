#ifndef __TABLE_P_H__
#define __TABLE_P_H__

#include <QTableWidget>
#include <QTimer>
#include <QItemDelegate>
#include <QWidget>
#include <QModelIndex>
#include <QListWidget>
#include <QVector>

class MKTable;
class QMouseEvent;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class MKTableItemDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  MKTableItemDelegate( MKTable *parent = 0 );
  void paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex &index ) const;
  QWidget *createEditor ( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  void setEditorData( QWidget *editor, const QModelIndex &index ) const;
  void updateEditorGeometry ( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
private slots:
  void commitAndCloseEditor();
private:
  bool eventFilter(QObject *obj, QEvent *ev);
  MKTable *table;
};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class MKTableItemCBWidget : public QListWidget
{
  Q_OBJECT
public:
  MKTableItemCBWidget( QWidget *parent = 0 );
  void highlightcursor();
signals:
  void editingFinished();
protected:
  void mousePressEvent( QMouseEvent * event );
  void mouseReleaseEvent( QMouseEvent * event );
  void mouseMoveEvent ( QMouseEvent * event );
  void leaveEvent ( QEvent * event );

   QStyleOptionViewItem viewOptions() const
   { QStyleOptionViewItem option = QListView::viewOptions();
     option.showDecorationSelected = true;
     return option;
   }
private:
  int row_under_mouse;
};

/*! \page xml_table_desc ������ XML ��������� �������

  �������� ����������� � ��� \b Table.

  \b Table �������� ��������� ���������:
  \li \b ColumnCount
  \li \b RowCount
  \li \b Labels
  \li \b LabelsSize

  ������ ���� \b Table ����������� ���� \b Item � ��������� ������ �������.

  \b Item �������� ��������� ���������:
  \li \b Column
  \li \b Row
  \li \b Text
  \li \b Font
  \li \b Assign

  �������� \b Font ����� ���������� � ���� ����������� ";" ��������:
  \li \b bold         ���������� �����
  \li \b underline    �������������
  \li \b align_left   ������������ � ������ ����
  \li \b align_center ������������ �� ������
  \li \b align_right  ������������ ������� ����

  �������� \b Assign ������ ������� ������ ������ � ������. �� �������� � ����:

  \b \c "[����� ������]/[����� �����]/[����� �������]".

  ���� ������� �� ����� ��� �� ������������� �������, ���������, ��� ������ ��������
  ����� � �� ���������� � ������.

  ������:

\verbatim
\endverbatim

*/

#endif
