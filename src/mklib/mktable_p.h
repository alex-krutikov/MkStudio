#ifndef __TABLE_P_H__
#define __TABLE_P_H__

#include <QItemDelegate>
#include <QListWidget>
#include <QModelIndex>
#include <QTableWidget>
#include <QTimer>
#include <QVector>
#include <QWidget>

class MKTable;
class QMouseEvent;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class MKTableItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    MKTableItemDelegate(MKTable *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
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
    MKTableItemCBWidget(QWidget *parent = 0);
    void setConfirmEdit(bool confEdit) { confirmEdit = confEdit; }
    void highlightcursor();
signals:
    void editingFinished();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void showEvent(QShowEvent *event);

    QStyleOptionViewItem viewOptions() const
    {
        QStyleOptionViewItem option = QListView::viewOptions();
        option.showDecorationSelected = true;
        return option;
    }

private:
    int row_under_mouse;
    bool confirmEdit;
};

/*! \page xml_table_desc Формат XML документа таблицы

  Документ заключается в тег \b Table.

  \b Table содержит следующие аттрибуты:
  \li \b ColumnCount
  \li \b RowCount
  \li \b Labels
  \li \b LabelsSize

  Внутри тега \b Table расположены теги \b Item с описанием ячейки таблицы.

  \b Item содержит следующие аттрибуты:
  \li \b Column
  \li \b Row
  \li \b Text
  \li \b Font
  \li \b Assign

  Аттрибут \b Font может задаваться в виде разделенных ";" значений:
  \li \b bold         полужирный шрифт
  \li \b underline    подчеркивание
  \li \b align_left   выравнивание к левому краю
  \li \b align_center выравнивание по центру
  \li \b align_right  выравнивание правому краю

  Аттрибут \b Assign задает привзку данной ячейки к опросу. Он задается в виде:

  \b \c "[номер модуля]/[номер слота]/[номер сигнала]".

  Если атрибут не задан или не соответствует формату, считается, что ячейка
содержит текст и не отностится к опросу.

  Пример:

\verbatim
\endverbatim

*/

#endif
